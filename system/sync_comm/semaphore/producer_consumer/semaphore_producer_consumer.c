#include "qosa_sys.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

#define BUFFER_SIZE 100

// 鍏变韩缂撳啿鍖哄拰淇″彿閲?static char buffer[BUFFER_SIZE];
static qosa_sem_t empty_sem = NULL;    // 绌烘Ы浣嶄俊鍙烽噺
static qosa_sem_t full_sem = NULL;     // 鏁版嵁淇″彿閲?static int producer_index = 0;
static int consumer_index = 0;
static int item_count = 0;

// 鐢熶骇鑰呬换鍔?void producer_task(void *arg)
{
    char data = 'A';
    int ret;
    
    while (1) {
        // 绛夊緟绌烘Ы浣?        ret = qosa_sem_wait(empty_sem, QOSA_WAIT_FOREVER);
        if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Producer", "Wait empty sem failed: %d", ret);
            continue;
        }
        
        // 鐢熶骇鏁版嵁
        buffer[producer_index] = data;
        producer_index = (producer_index + 1) % BUFFER_SIZE;
        item_count++;
        
        // 璁＄畻淇″彿閲忚鏁板€硷紙鐢ㄤ簬鐩戞帶锛?        qosa_uint32_t empty_cnt, full_cnt;
        qosa_sem_get_cnt(empty_sem, &empty_cnt);
        qosa_sem_get_cnt(full_sem, &full_cnt);
        
        QOSA_LOG_I("Producer", "Produced '%c', items: %d, empty: %u, full: %u", 
                   data, item_count, empty_cnt, full_cnt);
        
        // 閫氱煡娑堣垂鑰?        ret = qosa_sem_release(full_sem);
        if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Producer", "Release full sem failed: %d", ret);
        }
        
        data = (data == 'Z') ? 'A' : data + 1;
        qosa_task_sleep(100); // 妯℃嫙鐢熶骇鏃堕棿
    }
}

// 娑堣垂鑰呬换鍔?void consumer_task(void *arg)
{
    char data;
    int ret;
    
    while (1) {
        // 绛夊緟鏁版嵁
        ret = qosa_sem_wait(full_sem, 500); // 500ms瓒呮椂
        if (ret == QOSA_ERROR_SEMA_TIMEOUT_ERR) {
            QOSA_LOG_W("Consumer", "Wait data timeout");
            continue;
        } else if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Consumer", "Wait full sem failed: %d", ret);
            continue;
        }
        
        // 娑堣垂鏁版嵁
        data = buffer[consumer_index];
        consumer_index = (consumer_index + 1) % BUFFER_SIZE;
        item_count--;
        
        // 閫氱煡鐢熶骇鑰呮湁绌烘Ы浣?        ret = qosa_sem_release(empty_sem);
        if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Consumer", "Release empty sem failed: %d", ret);
        }
        
        QOSA_LOG_I("Consumer", "Consumed '%c', remaining items: %d", data, item_count);
        
        qosa_task_sleep(150); // 妯℃嫙娑堣垂鏃堕棿
    }
}

// 鍒濆鍖栫敓浜ц€?娑堣垂鑰呮ā鍨?int producer_consumer_init(void)
{
    int ret;
    
    // 1. 鍒涘缓淇″彿閲?    // 绌烘Ы浣嶄俊鍙烽噺锛氬垵濮嬩负BUFFER_SIZE锛屾渶澶т负BUFFER_SIZE
    ret = qosa_sem_create_ex(&empty_sem, BUFFER_SIZE, BUFFER_SIZE);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("PC", "Create empty sem failed: %d", ret);
        return ret;
    }
    
    // 鏁版嵁淇″彿閲忥細鍒濆涓?锛屾渶澶т负BUFFER_SIZE
    ret = qosa_sem_create_ex(&full_sem, 0, BUFFER_SIZE);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("PC", "Create full sem failed: %d", ret);
        qosa_sem_delete(empty_sem);
        return ret;
    }
    
    // 2. 鍒涘缓鐢熶骇鑰呬换鍔?    ret = qosa_task_create("Producer", producer_task, NULL, 
                          4096, QOSA_TASK_PRIORITY_NORMAL);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("PC", "Create producer task failed");
        goto cleanup;
    }
    
    // 3. 鍒涘缓娑堣垂鑰呬换鍔?    ret = qosa_task_create("Consumer", consumer_task, NULL,
                          4096, QOSA_TASK_PRIORITY_NORMAL);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("PC", "Create consumer task failed");
        goto cleanup;
    }
    
    QOSA_LOG_I("PC", "Producer-Consumer model started");
    return QOSA_ERROR_OK;
    
cleanup:
    qosa_sem_delete(empty_sem);
    qosa_sem_delete(full_sem);
    return ret;
}

static void __unirtos_export_semaphore_producer_consumer(void)
{
    (void)producer_consumer_init();
}

UNIRTOS_APP_EXPORT(200, "semaphore_producer_consumer", __unirtos_export_semaphore_producer_consumer);
