#include "qosa_sys.h"
#include "qosa_def.h"
#include "qosa_log.h"
#include "qosa_rtc.h"
#include "qcm_ntp_app.h"
#include "unirtos_app_init_registry.h"

#define quec_ntp_log(...)             QOSA_LOG_D(LOG_TAG, ##__VA_ARGS__)

#define QUEC_NTP_DEMO_TASK_STACK_SIZE 4096

// NTP 鏈嶅姟鍣ㄥ湴鍧€鍙婄鍙?#define QUEC_NTP_DEMO_SERVER          "ntp.aliyun.com"
#define QUEC_NTP_DEMO_PORT            123

// NTP 闇€瑕侀厤缃殑鍙傛暟
#define QUEC_NTP_DEMO_SIMID           0
#define QUEC_NTP_DEMO_PDPID           1
// NTP 閲嶈瘯娆℃暟鍙婇噸璇曢棿闅旀椂闂?#define QUEC_NTP_DEMO_RETRY_CNT       3
#define QUEC_NTP_DEMO_RETRY_TIMEOUT   15

/**
 * @struct qapp_ntp_msg_t
 * @brief ntp cb鍑芥暟杩斿洖搴旂瓟浜嬩欢閫氱煡
 */
typedef struct
{
    qcm_ntp_client_id   client_id;  /*!< 瀵瑰簲褰撳墠鍦ㄦ墽琛岀殑NTP id */
    qcm_ntp_result_code result;     /*!< NTP APP鎵ц缁撴灉杩斿洖 */
    qosa_rtc_time_t     sync_time;  /*!< 濡傛灉鎴愬姛鍒欒繑鍥濶TP鏌ヨ鍒扮殑鏃堕棿 */
    void               *user_param; /*!< 瀵瑰簲鐢ㄦ埛鍚姩鏃舵惡甯︾殑鐢ㄦ埛鍙傛暟 */
} qapp_ntp_msg_t;


/*===========================================================================
 *  Variate
 ===========================================================================*/

static qosa_task_t g_ntp_demo_task = QOSA_NULL;
static qosa_msgq_t g_ntp_demo_msg = QOSA_NULL;

/*===========================================================================
 *  Static API Functions
 ===========================================================================*/

/**
 * NTP 鏃堕棿鏌ヨ鍚屾缁撴灉鍥炶皟鍑芥暟
 *
 * 璇ュ嚱鏁扮敤浜庡鐞哊TP瀹㈡埛绔椂闂存煡璇㈠畬鎴愬悗鐨勭粨鏋滃洖璋? * 骞跺彂閫佸埌娑堟伅闃熷垪涓緵鍏朵粬妯″潡澶勭悊
 *
 * @param client_id NTP瀹㈡埛绔爣璇嗙
 * @param result NTP 鏌ヨ缁撴灉鐮侊紝琛ㄧず鏌ヨ鎴愬姛鎴栧け璐? * @param sync_time 鏌ヨ鍚庣殑鏃堕棿淇℃伅缁撴瀯浣撴寚閽? * @param arg 鐢ㄦ埛鑷畾涔夊弬鏁版寚閽? */
static void quec_ntp_sync_result_cb(qcm_ntp_client_id client_id, qcm_ntp_result_code result, qosa_rtc_time_t *sync_time, void *arg)
{
    qapp_ntp_msg_t msg = {0};

    /* 灏佽NTP鏌ヨ缁撴灉娑堟伅 */
    msg.client_id = client_id;
    msg.result = result;
    msg.user_param = arg;
    qosa_memcpy(&msg.sync_time, sync_time, sizeof(qosa_rtc_time_t));

    /* 鍙戦€佹秷鎭?*/
    qosa_msgq_release(g_ntp_demo_msg, sizeof(qapp_ntp_msg_t), (qosa_uint8_t *)&msg, QOSA_NO_WAIT);
}

/**
 * @brief NTP 缁撴灉澶勭悊鍑芥暟
 * @param msg NTP娑堟伅缁撴瀯浣撴寚閽堬紝鍖呭惈鏌ヨ缁撴灉鍜屾椂闂翠俊鎭? */
static void quec_ntp_result_handler(qapp_ntp_msg_t *msg)
{
    QOSA_UNUSED(msg);
    char             rsp_ptr[QOSA_ARRAY_BYTE_128] = {0};  // 鐢ㄤ簬瀛樺偍鏃堕棿淇℃伅瀛楃涓?    qosa_int32_t     rsp_len = 0;                         // 鐢ㄤ簬璁板綍鏃堕棿淇℃伅瀛楃涓查暱搴?    qosa_rtc_time_t *time = QOSA_NULL;                    // 鐢ㄤ簬瀛樺偍鏌ヨ鍚庣殑鏃堕棿淇℃伅
    qosa_int8_t      timezone = qosa_rtc_get_timezone();  // 鐢ㄤ簬鑾峰彇褰撳墠鏃跺尯

    /* 妫€鏌ユ秷鎭寚閽堟槸鍚︿负绌?*/
    if (msg == QOSA_NULL)
    {
        return;
    }

    /* 鏍规嵁NTP鏌ヨ缁撴灉杩涜鐩稿簲澶勭悊 */
    if (msg->result == QCM_NTP_SUCCESS)
    {
        /* 鏌ヨ鎴愬姛锛氭牸寮忓寲鏃堕棿淇℃伅骞惰褰曟棩蹇?*/
        time = &msg->sync_time;
        rsp_len += qosa_snprintf(
            rsp_ptr,
            QOSA_ARRAY_BYTE_128,
            "\"%04d/%02d/%02d,%02d:%02d:%02d%c%02d\"",
            time->tm_year + 1900,
            time->tm_mon + 1,
            time->tm_mday,
            time->tm_hour,
            time->tm_min,
            time->tm_sec,
            timezone >= 0 ? '+' : '-',
            QOSA_ABS(timezone)
        );
        quec_ntp_log("NTP_TIME: [%s]", rsp_ptr);
    }
    else
    {
        /* 鏌ヨ澶辫触锛氳褰曢敊璇爜 */
        quec_ntp_log("ntp errcode=%x", msg->result);
    }
}

/**
 * @brief NTP 鏌ヨ婕旂ず澶勭悊鍑芥暟
 * @param ctx 浠诲姟涓婁笅鏂囨寚閽堬紝鏈娇鐢? *
 * 璇ュ嚱鏁版紨绀轰簡NTP鏃堕棿鏌ヨ鍚屾鐨勫畬鏁存祦绋嬶紝鍖呮嫭鍒涘缓NTP瀹㈡埛绔€侀厤缃弬鏁般€? * 鍚姩寮傛鍚屾浠ュ強澶勭悊鍚屾缁撴灉绛夋搷浣溿€? */
static void quec_ntp_demo_process(void *ctx)
{
    qcm_ntp_client_id   client_id = 0;
    qcm_ntp_config_t    ntp_options = {0};
    qapp_ntp_msg_t      msg = {0};
    qcm_ntp_result_code ret = 0;

    // 绛夊緟10绉掞紝鏂逛究鎶撳彇log鍜屽紑鏈烘敞缃?    qosa_task_sleep_sec(10);

    // 鐢宠NTP client id
    client_id = qcm_ntp_client_new();
    if (client_id <= 0)
    {
        quec_ntp_log("ntp err");
        return;
    }

    // 閰嶇疆NTP鍙傛暟骞跺惎鍔ㄥ悓姝ヨ繃绋?    ntp_options.sim_id = QUEC_NTP_DEMO_SIMID;
    ntp_options.pdp_id = QUEC_NTP_DEMO_PDPID;

    ntp_options.num_data_bytes = 48;
    ntp_options.retry_cnt = QUEC_NTP_DEMO_RETRY_CNT;
    // 璁剧疆鍚屾鏈湴鏃堕棿鏍囧織锛屾澶勪笉鏇存柊鏈湴鏃堕棿, 濡傛灉璁剧疆涓?QOSA_TRUE 鍒欎細鍚屾鍒版湰鍦版椂闂?    ntp_options.sync_local_time = QOSA_FALSE;
    ntp_options.retry_interval_tm = QUEC_NTP_DEMO_RETRY_TIMEOUT;

    // 鍚姩 NTP 寮傛鎵ц
    ret = qcm_ntp_sync_start(client_id, QUEC_NTP_DEMO_SERVER, QUEC_NTP_DEMO_PORT, &ntp_options, quec_ntp_sync_result_cb, QOSA_NULL);
    if (ret != QCM_NTP_SUCCESS)
    {
        quec_ntp_log("ntp start err =%x", ret);
        qosa_msgq_delete(g_ntp_demo_msg);
        return;
    }

    // 绛夊緟鍚屾缁撴灉娑堟伅骞惰繘琛屽鐞?    qosa_msgq_wait(g_ntp_demo_msg, (qosa_uint8_t *)&msg, sizeof(qapp_ntp_msg_t), QOSA_WAIT_FOREVER);
    quec_ntp_result_handler(&msg);
    qosa_msgq_delete(g_ntp_demo_msg);
}

void quec_demo_ntp_init(void)
{
    quec_ntp_log("enter Quectel NTP DEMO !!!");
    if (g_ntp_demo_msg == QOSA_NULL)
    {
        qosa_msgq_create(&g_ntp_demo_msg, sizeof(qapp_ntp_msg_t), 5);
    }
    if (g_ntp_demo_task == QOSA_NULL)
    {
        qosa_task_create(&g_ntp_demo_task, QUEC_NTP_DEMO_TASK_STACK_SIZE, QOSA_PRIORITY_NORMAL, "ntp_demo", quec_ntp_demo_process, QOSA_NULL);
    }
}

UNIRTOS_APP_EXPORT(200, "ntp_demo", quec_demo_ntp_init);
