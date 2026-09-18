#include "qosa_sys.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

#define MAX_CONNECTIONS 5

// 杩炴帴姹犲拰淇″彿閲?static qosa_sem_t connection_sem = NULL;
static int active_connections = 0;

// 妯℃嫙鏁版嵁搴撹繛鎺ョ粨鏋?typedef struct {
    int id;
    char status[20];
} db_connection_t;

static db_connection_t connections[MAX_CONNECTIONS];

// 鑾峰彇鏁版嵁搴撹繛鎺?int acquire_connection(int timeout_ms)
{
    int ret;
    
    // 绛夊緟鍙敤杩炴帴
    ret = qosa_sem_wait(connection_sem, timeout_ms);
    if (ret == QOSA_ERROR_SEMA_TIMEOUT_ERR) {
        QOSA_LOG_W("DB", "No available connection, timeout");
        return -1;
    } else if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("DB", "Wait connection sem failed: %d", ret);
        return -1;
    }
    
    // 鏌ユ壘绌洪棽杩炴帴
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (strcmp(connections[i].status, "idle") == 0) {
            connections[i].id = i;
            strcpy(connections[i].status, "busy");
            active_connections++;
            
            qosa_uint32_t available;
            qosa_sem_get_cnt(connection_sem, &available);
            QOSA_LOG_I("DB", "Connection %d acquired, active: %d, available: %u", 
                      i, active_connections, available);
            
            return i;
        }
    }
    
    // 涓嶅簲璇ユ墽琛屽埌杩欓噷
    qosa_sem_release(connection_sem);
    return -1;
}

// 閲婃斁鏁版嵁搴撹繛鎺?void release_connection(int conn_id)
{
    if (conn_id < 0 || conn_id >= MAX_CONNECTIONS) {
        QOSA_LOG_E("DB", "Invalid connection ID: %d", conn_id);
        return;
    }
    
    strcpy(connections[conn_id].status, "idle");
    active_connections--;
    
    // 閲婃斁淇″彿閲?    int ret = qosa_sem_release(connection_sem);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("DB", "Release connection sem failed: %d", ret);
    }
    
    qosa_uint32_t available;
    qosa_sem_get_cnt(connection_sem, &available);
    QOSA_LOG_I("DB", "Connection %d released, active: %d, available: %u", 
              conn_id, active_connections, available);
}

// 鏁版嵁搴撴搷浣滀换鍔?void db_task(void *arg)
{
    int task_id = *(int*)arg;
    int conn_id;
    
    for (int i = 0; i < 3; i++) {  // 姣忎釜浠诲姟鎵ц3娆℃搷浣?        // 鑾峰彇杩炴帴锛堢瓑寰?绉掞級
        conn_id = acquire_connection(2000);
        if (conn_id < 0) {
            QOSA_LOG_W("Task%d", "Failed to get connection", task_id);
            continue;
        }
        
        // 妯℃嫙鏁版嵁搴撴搷浣?        QOSA_LOG_I("Task%d", "Using connection %d for DB operation", task_id, conn_id);
        qosa_task_sleep(300 + task_id * 50);  // 涓嶅悓浠诲姟涓嶅悓鎿嶄綔鏃堕棿
        
        // 閲婃斁杩炴帴
        release_connection(conn_id);
        
        qosa_task_sleep(200);  // 浠诲姟闂撮棿闅?    }
}

// 鍒濆鍖栬繛鎺ユ睜
int connection_pool_init(void)
{
    int ret;
    
    // 1. 鍒濆鍖栬繛鎺ユ睜
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i].id = i;
        strcpy(connections[i].status, "idle");
    }
    
    // 2. 鍒涘缓杩炴帴淇″彿閲忥紙鍒濆鍏ㄩ儴鍙敤锛?    ret = qosa_sem_create_ex(&connection_sem, MAX_CONNECTIONS, MAX_CONNECTIONS);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("Pool", "Create connection sem failed: %d", ret);
        return ret;
    }
    
    // 3. 鍒涘缓澶氫釜鏁版嵁搴撲换鍔?    int task_ids[] = {1, 2, 3, 4, 5, 6, 7, 8};  // 8涓换鍔＄珵浜?涓繛鎺?    
    for (int i = 0; i < 8; i++) {
        char task_name[20];
        snprintf(task_name, sizeof(task_name), "DBTask%d", task_ids[i]);
        
        ret = qosa_task_create(task_name, db_task, &task_ids[i],
                              4096, QOSA_TASK_PRIORITY_NORMAL);
        if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Pool", "Create task %s failed", task_name);
        }
    }
    
    QOSA_LOG_I("Pool", "Connection pool started with %d connections", MAX_CONNECTIONS);
    return QOSA_ERROR_OK;
}

static void __unirtos_export_semaphore_task_sync(void)
{
    (void)connection_pool_init();
}

UNIRTOS_APP_EXPORT(200, "semaphore_task_sync", __unirtos_export_semaphore_task_sync);
