#include "qosa_sys.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

// 娑堟伅瀹氫箟
typedef struct {
    qosa_uint32_t msg_id;
    qosa_uint32_t timestamp;
    qosa_int32_t  data;
    char          description[32];
} app_message_t;

static qosa_msgq_t g_msg_queue = NULL;
static const qosa_uint32_t MSG_QUEUE_SIZE = 10;  // 闃熷垪瀹归噺
static const qosa_uint32_t MSG_SIZE = sizeof(app_message_t);

// 鐢熶骇鑰呬换鍔★紙澶氫釜瀹炰緥锛?void producer_task(void *arg)
{
    int task_id = *(int*)arg;
    app_message_t msg;
    int ret;
    qosa_uint32_t sequence = 0;
    
    while (1) {
        // 鏋勯€犳秷鎭?        msg.msg_id = sequence++;
        msg.timestamp = qosa_get_system_time();
        msg.data = task_id * 1000 + sequence;
        snprintf(msg.description, sizeof(msg.description), 
                "Msg from Task%d-#%u", task_id, sequence);
        
        // 鍙戦€佹秷鎭埌闃熷垪锛堥潪闃诲锛岄槦鍒楁弧鍒欎涪寮冿級
        ret = qosa_msgq_release(g_msg_queue, MSG_SIZE, (qosa_uint8_t*)&msg, 0);
        
        if (ret == QOSA_ERROR_OK) {
            qosa_uint32_t queue_cnt;
            qosa_msgq_get_cnt(g_msg_queue, &queue_cnt);
            
            QOSA_LOG_I("Producer%d", "Sent msg#%u, queue: %u/%u", 
                      task_id, msg.msg_id, queue_cnt, MSG_QUEUE_SIZE);
        } else if (ret == QOSA_ERROR_MSGQ_FULL_ERR) {
            QOSA_LOG_W("Producer%d", "Queue full, discard msg#%u", task_id, msg.msg_id);
        } else {
            QOSA_LOG_E("Producer%d", "Send failed: %d", task_id, ret);
        }
        
        qosa_task_sleep(200 + task_id * 50);  // 涓嶅悓鐢熶骇鑰呭彂閫侀棿闅斾笉鍚?    }
}

// 娑堣垂鑰呬换鍔★紙澶勭悊鎵€鏈夋秷鎭級
void consumer_task(void *arg)
{
    app_message_t msg;
    int ret;
    qosa_uint32_t processed_count = 0;
    
    while (1) {
        // 绛夊緟娑堟伅锛堝甫1绉掕秴鏃讹級
        ret = qosa_msgq_wait(g_msg_queue, (qosa_uint8_t*)&msg, MSG_SIZE, 1000);
        
        if (ret == QOSA_ERROR_OK) {
            // 澶勭悊娑堟伅
            processed_count++;
            
            qosa_uint32_t queue_cnt;
            qosa_msgq_get_cnt(g_msg_queue, &queue_cnt);
            
            QOSA_LOG_I("Consumer", "Processing[%u]: ID=%u, From=%s, Data=%d, Queue=%u",
                      processed_count, msg.msg_id, msg.description, 
                      msg.data, queue_cnt);
            
            // 妯℃嫙娑堟伅澶勭悊鏃堕棿
            qosa_task_sleep(300);
            
        } else if (ret == QOSA_ERROR_SEMA_TIMEOUT_ERR) {
            QOSA_LOG_D("Consumer", "No message for 1 second, waiting...");
        } else {
            QOSA_LOG_E("Consumer", "Receive error: %d", ret);
            qosa_task_sleep(1000);
        }
    }
}

// 鐩戞帶浠诲姟锛堝畾鏈熸樉绀洪槦鍒楃姸鎬侊級
void monitor_task(void *arg)
{
    qosa_uint32_t last_cnt = 0;
    
    while (1) {
        qosa_uint32_t current_cnt;
        int ret = qosa_msgq_get_cnt(g_msg_queue, &current_cnt);
        
        if (ret == QOSA_ERROR_OK) {
            if (current_cnt != last_cnt) {
                float usage = (float)current_cnt / MSG_QUEUE_SIZE * 100;
                QOSA_LOG_I("Monitor", "Queue status: %u/%u (%.1f%%)", 
                          current_cnt, MSG_QUEUE_SIZE, usage);
                last_cnt = current_cnt;
            }
        }
        
        qosa_task_sleep(2000);  // 姣?绉掓鏌ヤ竴娆?    }
}

// 鍒濆鍖栨秷鎭槦鍒楃郴缁?int message_queue_system_init(void)
{
    int ret;
    
    // 1. 鍒涘缓娑堟伅闃熷垪
    ret = qosa_msgq_create(&g_msg_queue, MSG_SIZE, MSG_QUEUE_SIZE);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("System", "Create message queue failed: %d", ret);
        return ret;
    }
    
    QOSA_LOG_I("System", "Message queue created: size=%u bytes, capacity=%u",
              MSG_SIZE, MSG_QUEUE_SIZE);
    
    // 2. 鍒涘缓3涓敓浜ц€呬换鍔?    int producer_ids[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        char task_name[20];
        snprintf(task_name, sizeof(task_name), "Producer%d", producer_ids[i]);
        
        ret = qosa_task_create(task_name, producer_task, &producer_ids[i],
                              4096, QOSA_TASK_PRIORITY_NORMAL);
        if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("System", "Create %s failed", task_name);
        } else {
            QOSA_LOG_I("System", "Producer task %d started", producer_ids[i]);
        }
    }
    
    // 3. 鍒涘缓娑堣垂鑰呬换鍔?    ret = qosa_task_create("Consumer", consumer_task, NULL,
                          4096, QOSA_TASK_PRIORITY_NORMAL);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("System", "Create consumer task failed");
        goto cleanup;
    }
    
    // 4. 鍒涘缓鐩戞帶浠诲姟
    ret = qosa_task_create("Monitor", monitor_task, NULL,
                          2048, QOSA_TASK_PRIORITY_LOW);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_W("System", "Create monitor task failed (optional)");
    }
    
    QOSA_LOG_I("System", "Message queue system started successfully");
    return QOSA_ERROR_OK;
    
cleanup:
    if (g_msg_queue != NULL) {
        qosa_msgq_delete(g_msg_queue);
        g_msg_queue = NULL;
    }
    return ret;
}

static void __unirtos_export_queue_processing(void)
{
    (void)message_queue_system_init();
}

UNIRTOS_APP_EXPORT(200, "queue_processing", __unirtos_export_queue_processing);
