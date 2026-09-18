#include "qosa_sys.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

// 鍛戒护绫诲瀷瀹氫箟
typedef enum {
    CMD_NONE = 0,
    CMD_SYSTEM_RESET,
    CMD_DATA_UPDATE,
    CMD_STATUS_REPORT,
    CMD_NETWORK_CONNECT,
    CMD_NETWORK_DISCONNECT,
    CMD_MAX
} command_type_t;

// 鍛戒护娑堟伅缁撴瀯
typedef struct {
    command_type_t cmd_type;
    qosa_uint32_t  cmd_id;
    qosa_uint32_t  timestamp;
    qosa_int32_t   param1;
    qosa_int32_t   param2;
    void*          extra_data;  // 鍙€夐澶栨暟鎹?} command_message_t;

// 瀛愮郴缁熷畾涔?typedef enum {
    SUBSYS_NETWORK = 0,
    SUBSYS_DATA,
    SUBSYS_SYSTEM,
    SUBSYS_MAX
} subsystem_t;

static qosa_msgq_t g_cmd_queues[SUBSYS_MAX] = {NULL};

// 鍛戒护鍒嗗彂鍣紙涓绘帶浠诲姟锛?void command_dispatcher(void *arg)
{
    command_message_t master_cmd;
    int ret;
    
    // 鍒涘缓涓诲懡浠ら槦鍒?    ret = qosa_msgq_create(&g_cmd_queues[0], sizeof(command_message_t), 20);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("Dispatcher", "Create master queue failed");
        return;
    }
    
    while (1) {
        // 绛夊緟鍛戒护锛堟棤闄愮瓑寰咃級
        ret = qosa_msgq_wait(g_cmd_queues[0], (qosa_uint8_t*)&master_cmd, 
                            sizeof(command_message_t), QOSA_WAIT_FOREVER);
        
        if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Dispatcher", "Receive command error: %d", ret);
            continue;
        }
        
        QOSA_LOG_I("Dispatcher", "Received CMD%d (ID:%u)", 
                  master_cmd.cmd_type, master_cmd.cmd_id);
        
        // 鏍规嵁鍛戒护绫诲瀷鍒嗗彂鍒板搴斿瓙绯荤粺
        subsystem_t target_subsys;
        switch (master_cmd.cmd_type) {
            case CMD_NETWORK_CONNECT:
            case CMD_NETWORK_DISCONNECT:
                target_subsys = SUBSYS_NETWORK;
                break;
            case CMD_DATA_UPDATE:
                target_subsys = SUBSYS_DATA;
                break;
            case CMD_SYSTEM_RESET:
            case CMD_STATUS_REPORT:
                target_subsys = SUBSYS_SYSTEM;
                break;
            default:
                QOSA_LOG_W("Dispatcher", "Unknown command type: %d", master_cmd.cmd_type);
                continue;
        }
        
        // 杞彂鍛戒护鍒板瓙绯荤粺闃熷垪
        if (g_cmd_queues[target_subsys] != NULL) {
            ret = qosa_msgq_release(g_cmd_queues[target_subsys], 
                                   sizeof(command_message_t),
                                   (qosa_uint8_t*)&master_cmd, 0);
            
            if (ret != QOSA_ERROR_OK) {
                QOSA_LOG_E("Dispatcher", "Forward to subsys%d failed: %d", 
                          target_subsys, ret);
            }
        }
    }
}

// 缃戠粶瀛愮郴缁熶换鍔?void network_subsystem(void *arg)
{
    command_message_t cmd;
    int ret;
    
    // 鍒涘缓缃戠粶瀛愮郴缁熼槦鍒?    ret = qosa_msgq_create(&g_cmd_queues[SUBSYS_NETWORK], 
                          sizeof(command_message_t), 10);
    if (ret != QOSA_ERROR_OK) {
        QOSA_LOG_E("Network", "Create queue failed");
        return;
    }
    
    QOSA_LOG_I("Network", "Subsystem started");
    
    while (1) {
        // 绛夊緟缃戠粶鐩稿叧鍛戒护锛堝甫瓒呮椂锛?        ret = qosa_msgq_wait(g_cmd_queues[SUBSYS_NETWORK], 
                            (qosa_uint8_t*)&cmd,
                            sizeof(command_message_t), 5000);
        
        if (ret == QOSA_ERROR_SEMA_TIMEOUT_ERR) {
            // 瓒呮椂锛屾墽琛屽畾鏈熺淮鎶?            QOSA_LOG_D("Network", "No command, performing maintenance");
            continue;
        } else if (ret != QOSA_ERROR_OK) {
            QOSA_LOG_E("Network", "Receive error: %d", ret);
            continue;
        }
        
        // 澶勭悊鍛戒护
        switch (cmd.cmd_type) {
            case CMD_NETWORK_CONNECT:
                QOSA_LOG_I("Network", "Executing CONNECT command (ID:%u)", cmd.cmd_id);
                // 妯℃嫙缃戠粶杩炴帴鎿嶄綔
                qosa_task_sleep(1000);
                QOSA_LOG_I("Network", "Network connected");
                break;
                
            case CMD_NETWORK_DISCONNECT:
                QOSA_LOG_I("Network", "Executing DISCONNECT command (ID:%u)", cmd.cmd_id);
                // 妯℃嫙缃戠粶鏂紑鎿嶄綔
                qosa_task_sleep(500);
                QOSA_LOG_I("Network", "Network disconnected");
                break;
                
            default:
                QOSA_LOG_W("Network", "Unexpected command: %d", cmd.cmd_type);
        }
    }
}

// 鍛戒护鍙戦€佹帴鍙ｏ紙渚涘閮ㄦā鍧楄皟鐢級
int send_command(command_type_t cmd_type, qosa_int32_t param1, qosa_int32_t param2)
{
    static qosa_uint32_t cmd_counter = 0;
    command_message_t cmd;
    int ret;
    
    if (g_cmd_queues[0] == NULL) {
        return QOSA_ERROR_MSGQ_INVALID_ERR;
    }
    
    // 鏋勯€犲懡浠?    cmd.cmd_type = cmd_type;
    cmd.cmd_id = cmd_counter++;
    cmd.timestamp = qosa_get_system_time();
    cmd.param1 = param1;
    cmd.param2 = param2;
    cmd.extra_data = NULL;
    
    // 鍙戦€佸埌涓诲懡浠ら槦鍒?    ret = qosa_msgq_release(g_cmd_queues[0], sizeof(command_message_t),
                           (qosa_uint8_t*)&cmd, 0);
    
    if (ret == QOSA_ERROR_OK) {
        QOSA_LOG_D("CmdSender", "Command %d sent (ID:%u)", cmd_type, cmd.cmd_id);
    }
    
    return ret;
}

// 鍒濆鍖栧懡浠ょ郴缁?int command_system_init(void)
{
    int ret;
    
    // 鍚姩鍒嗗彂鍣ㄤ换鍔?    ret = qosa_task_create("Dispatcher", command_dispatcher, NULL,
                          4096, QOSA_TASK_PRIORITY_HIGH);
    if (ret != QOSA_ERROR_OK) {
        return ret;
    }
    
    // 鍚姩缃戠粶瀛愮郴缁熶换鍔?    ret = qosa_task_create("Network", network_subsystem, NULL,
                          4096, QOSA_TASK_PRIORITY_NORMAL);
    if (ret != QOSA_ERROR_OK) {
        return ret;
    }
    
    // 鍚姩鍏朵粬瀛愮郴缁熶换鍔★紙鐣ワ級
    
    QOSA_LOG_I("CmdSystem", "Command system initialized");
    return QOSA_ERROR_OK;
}

// 娴嬭瘯鍛戒护鍙戦€?void test_command_sender(void *arg)
{
    QOSA_LOG_I("Test", "Starting command test...");
    
    qosa_task_sleep(2000);
    
    // 鍙戦€佺綉缁滆繛鎺ュ懡浠?    send_command(CMD_NETWORK_CONNECT, 0, 0);
    
    qosa_task_sleep(3000);
    
    // 鍙戦€佹暟鎹洿鏂板懡浠?    send_command(CMD_DATA_UPDATE, 100, 200);
    
    qosa_task_sleep(2000);
    
    // 鍙戦€佺綉缁滄柇寮€鍛戒护
    send_command(CMD_NETWORK_DISCONNECT, 0, 0);
    
    QOSA_LOG_I("Test", "Command test completed");
}

static void __unirtos_export_queue_dispatching_system(void)
{
    (void)command_system_init();
}

UNIRTOS_APP_EXPORT(200, "queue_dispatching_system", __unirtos_export_queue_dispatching_system);
