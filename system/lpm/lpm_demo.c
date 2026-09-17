#include "qosa_def.h"
#include "qosa_sys.h"
#include "qosa_lpm.h"
#include "qosa_log.h"
#include "qosa_event_notify.h"
#include "lpm_demo.h"
#include "unirtos_app_init_registry.h"

#define QOS_LOG_TAG LOG_TAG

// 浜嬩欢瀹氫箟
#define LPM_DEMO_EVENT_SLEEP_STATUS (QOSA_EVENT_LPM_SLEEP_STATUS + 100)

/** lpm demo task handle */
static qosa_task_t g_lpm_demo_task = QOSA_NULL;

/** lpm demo message queue handle */
static qosa_msgq_t g_lpm_demo_queue = QOSA_NULL;

/** 搴旂敤鎶曠エ鍙ユ焺 */
static qosa_handle g_app_vote_handle = QOSA_NULL;

// ===========================================================================
// [Data Structure] Define messages for communication between Callback and MainTask
// ===========================================================================

/**
 * @struct demo_lpm_msg_t
 * @brief LPM demo message structure for communication between callback and main task
 */
typedef struct
{
    qosa_int32_t event_id; /*!< Event type */
    union
    {
        qosa_lpm_sleep_event_t sleep_event; /*!< Sleep event data */
        qosa_lpm_psm_event_t psm_event;    /*!< PSM event data */
        qosa_int32_t result_code;           /*!< General result code */
    } data;                                /*!< Union data for different events */
} demo_lpm_msg_t;

// ===========================================================================
// [Producer] Callback functions
// Features: Run in underlying threads/interrupts, only responsible for packaging messages and sending them to the queue
// ===========================================================================

/**
 * @brief LPM 浼戠湢浜嬩欢鍥炶皟鍑芥暟
 * 
 * @param [in] user_argv
 *           - 鐢ㄦ埛鑷畾涔夊弬鏁帮紝鏈嚱鏁颁腑鏈娇鐢? * @param [in] argv
 *           - 浜嬩欢鍙傛暟鎸囬拡锛屾寚鍚?qosa_lpm_sleep_event_t 缁撴瀯浣? * 
 * @return 杩斿洖 0 琛ㄧず鎴愬姛澶勭悊
 */
static int demo_lpm_sleep_callback(void *user_argv, void *argv)
{
    QOSA_UNUSED(user_argv);
    
    demo_lpm_msg_t         msg = {0};
    qosa_lpm_sleep_event_t *ev = (qosa_lpm_sleep_event_t *)argv;
    
    QLOGI("[LPM EVENT] Sleep Status: %d Reason: %d", ev->status, ev->reason);
    
    msg.event_id = LPM_DEMO_EVENT_SLEEP_STATUS;
    // 娣辨嫹璐濅簨浠舵暟鎹?    qosa_memcpy(&msg.data.sleep_event, ev, sizeof(qosa_lpm_sleep_event_t));
    
    if (g_lpm_demo_queue != QOSA_NULL)
    {
        qosa_msgq_release(g_lpm_demo_queue, sizeof(msg), (qosa_uint8_t *)&msg, QOSA_NO_WAIT);
    }
    
    return 0;
}

// ===========================================================================
// [Business Logic] Specific functional implementation functions
// ===========================================================================

/**
 * @brief 閰嶇疆 LPM 鍙傛暟
 * 
 * @param None
 * 
 * @return qosa_lpm_error_e
 *         - 鎴愬姛杩斿洖 QOSA_LPM_ERR_OK
 *         - 澶辫触杩斿洖鍏朵粬閿欒鐮? */
static qosa_lpm_error_e business_config_lpm(void)
{
    qosa_lpm_config_t config = {0};
    
    // 鑾峰彇褰撳墠閰嶇疆
    qosa_lpm_config_get(&config);
    QLOGI("Current LPM Mode: %d", config.lpm_mode);
    
    // 閰嶇疆浼戠湢鍙傛暟
    config.lpm_mode = QOSA_LPM_MODE_ENABLE;      // 鍚敤浼戠湢
    config.delay_time = 5000;                    // 寤惰繜 5 绉掕繘鍏ヤ紤鐪?    config.dtr_en = QOSA_LPM_DTR_ENABLE;          // 鍚敤 DTR 鍞ら啋
    config.uart_en = QOSA_LPM_UART_ENABLE;        // 鍚敤 UART 鍞ら啋
    config.rfr_enable = QOSA_TRUE;               // 鍚敤 RRC 蹇€熼噴鏀?    config.no_data_time = 10;                     // 10 绉掓棤鏁版嵁閲婃斁 RRC
    config.retry_time = 600;                      // 寮傚父鍚?10 鍒嗛挓閲嶈瘯
    
    // 閰嶇疆 PSM 鍙傛暟锛堝彲閫夛級
    qosa_strncpy((char *)config.tau, "01000010", QOSA_LPM_PTAUS_MAX_LEN);
    qosa_strncpy((char *)config.active, "00100010", QOSA_LPM_PTAUS_MAX_LEN);
    
    qosa_lpm_error_e ret = qosa_lpm_config_set(&config);
    if (ret == QOSA_LPM_ERR_OK)
    {
        QLOGI("LPM Config Set Success");
    }
    else
    {
        QLOGE("LPM Config Set Failed: %d", ret);
    }
    
    return ret;
}

/**
 * @brief 鍒涘缓搴旂敤鎶曠エ鍙ユ焺
 * 
 * @param None
 * 
 * @return qosa_lpm_error_e
 *         - 鎴愬姛杩斿洖 QOSA_LPM_ERR_OK
 *         - 澶辫触杩斿洖鍏朵粬閿欒鐮? */
static qosa_lpm_error_e business_create_vote_handle(void)
{
    if (g_app_vote_handle == QOSA_NULL)
    {
        qosa_lpm_error_e ret = qosa_lpm_app_vote_new_handle("lpm_demo", &g_app_vote_handle);
        if (ret == QOSA_LPM_ERR_OK)
        {
            QLOGI("Vote Handle Created Successfully");
        }
        else
        {
            QLOGE("Vote Handle Create Failed: %d", ret);
        }
        return ret;
    }
    return QOSA_LPM_ERR_OK;
}

/**
 * @brief 鎵ц闇€瑕佷繚鎸佸敜閱掔殑浠诲姟
 * 
 * @param None
 * 
 * @return None
 */
static void business_do_wakeup_task(void)
{
    QLOGI("Starting Wakeup Task...");
    
    // 1. 绂佹浼戠湢
    qosa_lpm_error_e ret = qosa_lpm_app_vote_disable(g_app_vote_handle);
    if (ret != QOSA_LPM_ERR_OK)
    {
        QLOGE("Vote Disable Failed: %d", ret);
        return;
    }
    
    QLOGI("Device will stay awake now");
    
    // 2. 鎵ц浠诲姟锛堜緥濡傦細鏁版嵁閲囬泦銆佺綉缁滀紶杈撶瓑锛?    qosa_task_sleep_ms(5000);  // 妯℃嫙浠诲姟鎵ц 5 绉?    
    // 3. 浠诲姟瀹屾垚锛屽厑璁镐紤鐪?    ret = qosa_lpm_app_vote_enable(g_app_vote_handle);
    if (ret == QOSA_LPM_ERR_OK)
    {
        QLOGI("Vote Enabled, device can enter sleep now");
    }
    else
    {
        QLOGE("Vote Enable Failed: %d", ret);
    }
}

// ===========================================================================
// [Consumer] Demo main task
// Features: Has independent stack space, can safely handle complex logic, block waiting for messages
// ===========================================================================

/**
 * @brief LPM demo 浠诲姟涓诲嚱鏁? * 
 * @param [in] arg
 *           - 浠诲姟鍙傛暟鎸囬拡锛屾湰鍑芥暟涓湭浣跨敤
 * 
 * @return 鏃犺繑鍥炲€? */
void lpm_demo_task_entry(void *arg)
{
    QOSA_UNUSED(arg);
    
    int              ret = 0;
    demo_lpm_msg_t    msg;
    
    QLOGI("Initializing LPM Demo...");
    
    // 1. 鍒涘缓娑堟伅闃熷垪锛堟繁搴?10锛屾瘡鏉℃秷鎭ぇ灏忎负 demo_lpm_msg_t锛?    ret = qosa_msgq_create(&g_lpm_demo_queue, sizeof(demo_lpm_msg_t), 10);
    if (ret != QOSA_OK)
    {
        QLOGE("Create lpm demo queue failed: %d", ret);
        return;
    }
    
    // 2. 娉ㄥ唽鍥炶皟
    qosa_event_notify_register(QOSA_EVENT_LPM_SLEEP_STATUS, demo_lpm_sleep_callback, QOSA_NULL);
    
    // 3. 閰嶇疆 LPM 鍙傛暟
    business_config_lpm();
    
    // 4. 鍒涘缓鎶曠エ鍙ユ焺
    business_create_vote_handle();
    
    // 5. 浜嬩欢寰幆
    while (1)
    {
        // 闃诲绛夊緟娑堟伅
        if (qosa_msgq_wait(g_lpm_demo_queue, (qosa_uint8_t *)&msg, sizeof(msg), QOSA_WAIT_FOREVER) == 0)  // 0 == OK
        {
            switch (msg.event_id)
            {
                case LPM_DEMO_EVENT_SLEEP_STATUS: {
                    qosa_lpm_sleep_event_t *ev = &msg.data.sleep_event;
                    
                    if (ev->status == QOSA_LPM_SLEEP_STATUS_SLEEP)
                    {
                        QLOGI(">>> Device Entering Sleep, Reason: %d <<<", ev->reason);
                        // 璁惧杩涘叆浼戠湢锛屽彲浠ヤ繚瀛樼姸鎬併€佸叧闂璁剧瓑
                    }
                    else if (ev->status == QOSA_LPM_SLEEP_STATUS_WAKEUP)
                    {
                        QLOGI("<<< Device Waking Up, Reason: %d >>>", ev->reason);
                        // 璁惧鍞ら啋锛屽彲浠ユ仮澶嶇姸鎬併€侀噸鏂板垵濮嬪寲澶栬绛?                        
                        // 鏍规嵁鍞ら啋鍘熷洜鎵ц涓嶅悓鎿嶄綔
                        switch (ev->reason)
                        {
                            case QOSA_LPM_WAKEUP_REASON_NET_DATA:
                                QLOGI("Wakeup by Network Data");
                                break;
                            case QOSA_LPM_WAKEUP_REASON_UART:
                                QLOGI("Wakeup by UART");
                                break;
                            case QOSA_LPM_WAKEUP_REASON_DTR:
                                QLOGI("Wakeup by DTR");
                                break;
                            case QOSA_LPM_WAKEUP_REASON_USB:
                                QLOGI("Wakeup by USB");
                                break;
                            default:
                                QLOGI("Wakeup by Other Reason");
                                break;
                        }
                    }
                    break;
                }
                default:
                    QLOGI("Event: Unhandled event ID %d", msg.event_id);
                    break;
            }
        }
        
        // 瀹氭湡鎵ц闇€瑕佷繚鎸佸敜閱掔殑浠诲姟锛堢ず渚嬶細姣?60 绉掞級
        static qosa_uint32_t last_task_time = 0;
        if (qosa_sys_get_tick() - last_task_time > 60000)
        {
            business_do_wakeup_task();
            last_task_time = qosa_sys_get_tick();
        }
    }
}

/**
 * @brief 鍒濆鍖?LPM demo
 * 
 * @param None
 * 
 * @return 鏃犺繑鍥炲€? * 
 * @note 浠诲姟鏍堝ぇ灏忎负 4KB锛屼紭鍏堢骇涓轰綆浼樺厛绾? */
void unir_lpm_demo_init(void)
{
    if (g_lpm_demo_task == QOSA_NULL)
    {
        int err = qosa_task_create(&g_lpm_demo_task, 4 * 1024, QOSA_PRIORITY_LOW, "lpm_demo", lpm_demo_task_entry, QOSA_NULL);
        if (err != QOSA_OK)
        {
            QLOGE("lpm_demo_init task create error");
            return;
        }
    }
}

UNIRTOS_APP_EXPORT(200, "lpm_demo", unir_lpm_demo_init);
