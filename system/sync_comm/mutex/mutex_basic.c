#include "qosa_sys.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

// 鍏变韩璧勬簮
static int shared_counter = 0;
static qosa_mutex_t counter_mutex = NULL;

// 浠诲姟1锛氬鍔犺鏁板櫒
void task1_counter_increment(void *arg)
{
    int ret;
    
    while (1) {
        // 鑾峰彇閿侊紙绛夊緟10ms锛?        ret = qosa_mutex_lock(counter_mutex, 10);
        if (ret == QOSA_ERROR_OK) {
            // 涓寸晫鍖哄紑濮?            shared_counter++;
            QOSA_LOG_I("Task1", "Counter: %d", shared_counter);
            
            // 閲婃斁閿?            qosa_mutex_unlock(counter_mutex);
            // 涓寸晫鍖虹粨鏉?        }
        
        qosa_task_sleep(100); // 浼戠湢100ms
    }
}

// 浠诲姟2锛氬噺灏戣鏁板櫒
void task2_counter_decrement(void *arg)
{
    int ret;
    
    while (1) {
        // 灏濊瘯鑾峰彇閿侊紙闈為樆濉烇級
        ret = qosa_mutex_try_lock(counter_mutex);
        if (ret == QOSA_ERROR_OK) {
            // 涓寸晫鍖哄紑濮?            shared_counter--;
            QOSA_LOG_I("Task2", "Counter: %d", shared_counter);
            
            // 閲婃斁閿?            qosa_mutex_unlock(counter_mutex);
            // 涓寸晫鍖虹粨鏉?        } else if (ret == QOSA_ERROR_MUTEX_EBUSY_ERR) {
            QOSA_LOG_D("Task2", "Mutex busy, skip this time");
        }
        
        qosa_task_sleep(150); // 浼戠湢150ms
    }
}

// 鍒濆鍖栧嚱鏁?int mutex_example_init(void)
{
    int ret;
    
    // 1. 鍒涘缓浜掓枼閿?    ret = qosa_mutex_create(&counter_mutex);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("MutexExample", "Create mutex failed: %d", ret);
        return ret;
    }
    
    // 2. 鍒涘缓浠诲姟1
    ret = qosa_task_create("Task1", task1_counter_increment, NULL, 
                          4096, QOSA_TASK_PRIORITY_NORMAL);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("MutexExample", "Create task1 failed");
        qosa_mutex_delete(counter_mutex);
        return ret;
    }
    
    // 3. 鍒涘缓浠诲姟2
    ret = qosa_task_create("Task2", task2_counter_decrement, NULL,
                          4096, QOSA_TASK_PRIORITY_NORMAL);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("MutexExample", "Create task2 failed");
        qosa_mutex_delete(counter_mutex);
        return ret;
    }
    
    QOSA_LOG_I("MutexExample", "Mutex example started");
    return QOSA_ERROR_OK;
}

// 娓呯悊鍑芥暟
void mutex_example_cleanup(void)
{
    if (counter_mutex != NULL) {
        qosa_mutex_delete(counter_mutex);
        counter_mutex = NULL;
    }
    QOSA_LOG_I("MutexExample", "Mutex example cleaned up");
}

static void __unirtos_export_mutex_basic(void)
{
    (void)mutex_example_init();
}

UNIRTOS_APP_EXPORT(200, "mutex_basic", __unirtos_export_mutex_basic);
