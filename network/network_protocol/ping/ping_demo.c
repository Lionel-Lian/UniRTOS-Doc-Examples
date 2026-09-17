#include "qosa_sys.h"
#include "qosa_def.h"
#include "qosa_log.h"
#include "qcm_ping_app.h"
#include "unirtos_app_init_registry.h"
#define quec_ping_log(...)             QOSA_LOG_D(LOG_TAG, ##__VA_ARGS__)
#define QUEC_PING_DEMO_TASK_STACK_SIZE 4096

// PING 鏈嶅姟鍣ㄥ湴鍧€
#define QUEC_PING_DEMO_SERVER          "www.baidu.com"

// PING 闇€瑕侀厤缃殑鍙傛暟
#define QUEC_PING_DEMO_SIMID           0
#define QUEC_PING_DEMO_PDPID           1
// PING 娆℃暟
#define QUEC_PING_DEMO_CNT             4
// PING 鍗曟瓒呮椂鏃堕棿
#define QUEC_PING_DEMO_TIMEOUT         4
// PING TTL
#define QUEC_PING_DEMO_TTL             255

/**
 * @brief ping鍥炶皟鍝嶅簲鏁版嵁缁撴瀯浣? *
 * 璇ョ粨鏋勪綋鐢ㄤ簬瀛樺偍ping鎿嶄綔鐨勫搷搴斾俊鎭紝鍖呮嫭浜嬩欢绫诲瀷銆侀敊璇爜銆? * 鍝嶅簲鏁版嵁浠ュ強鐢ㄦ埛鑷畾涔夊弬鏁? */
typedef struct
{
    qcm_ping_event_type event_id;   /**< ping浜嬩欢绫诲瀷 */
    qcm_ping_error_e    evt_code;   /**< ping浜嬩欢閿欒鐮?*/
    qcm_ping_resp_t     resp_ptr;   /**< ping鍝嶅簲鏁版嵁鎸囬拡 */
    void               *user_param; /**< 鐢ㄦ埛鑷畾涔夊弬鏁版寚閽?*/
} ping_demo_resp_t;

static qosa_task_t g_ping_demo_task = QOSA_NULL;
static qosa_msgq_t g_ping_demo_msg = QOSA_NULL;
/**
 * @brief ping缁撴灉鍥炶皟鍑芥暟
 *
 * 璇ュ嚱鏁扮敤浜庡鐞唒ing鎿嶄綔鐨勭粨鏋滃洖璋冿紝灏唒ing鍝嶅簲鏁版嵁杩涜灏佽骞堕€氳繃娑堟伅闃熷垪鍙戦€? *
 * @param event_id ping浜嬩欢绫诲瀷
 * @param evt_code ping閿欒鐮? * @param resp_ptr ping鍝嶅簲鏁版嵁鎸囬拡
 * @param user_param 鐢ㄦ埛鑷畾涔夊弬鏁版寚閽? */
static void quec_ping_result_cb(qcm_ping_event_type event_id, qcm_ping_error_e evt_code, qcm_ping_resp_t *resp_ptr, void *user_param)
{
    ping_demo_resp_t ping_rsp = {0};

    ping_rsp.event_id = event_id;
    ping_rsp.evt_code = evt_code;
    ping_rsp.user_param = user_param;

    /* 鏍规嵁浜嬩欢绫诲瀷澶嶅埗鐩稿簲鐨勫搷搴旀暟鎹?*/
    if (ping_rsp.event_id == QCM_PING_STATS)
    {
        qosa_memcpy(&ping_rsp.resp_ptr.type.status, &resp_ptr->type.status, sizeof(qcm_ping_stats_type));
    }
    else
    {
        qosa_memcpy(&ping_rsp.resp_ptr.type.summary, &resp_ptr->type.summary, sizeof(qcm_ping_summary_type));
    }

    /* 鍙戦€佹秷鎭?*/
    qosa_msgq_release(g_ping_demo_msg, sizeof(ping_demo_resp_t), (qosa_uint8_t *)&ping_rsp, QOSA_NO_WAIT);
}

/**
 * @brief 澶勭悊PING鎿嶄綔鐨勭粨鏋滃洖璋冨嚱鏁帮紝鏍规嵁浜嬩欢绫诲瀷瑙ｆ瀽骞惰褰昉ING缁熻鎴栨眹鎬讳俊鎭€? *
 * 璇ュ嚱鏁板鐞嗘潵鑷狿ING妯″潡鐨勫搷搴斾簨浠讹紝鍖呮嫭PING鐘舵€佹洿鏂板拰鏈€缁堢殑姹囨€讳俊鎭€? * 鏍规嵁浜嬩欢ID鍖哄垎鏄崟娆ING缁撴灉(QCM_PING_STATS)杩樻槸鏈€缁堟眹鎬?QCM_PING_SUMMARY)锛? * 骞跺皢鐩稿叧淇℃伅鏍煎紡鍖栧悗閫氳繃鏃ュ織杈撳嚭銆傚綋鏀跺埌姹囨€讳簨浠舵椂锛屾爣璁板鐞嗗畬鎴愩€? *
 * @param[in] ping_ptr 鎸囧悜PING鍝嶅簲鏁版嵁缁撴瀯鐨勬寚閽堬紝鍖呭惈浜嬩欢ID銆佷簨浠剁爜鍙婂搷搴斿唴瀹? * @return 杩斿洖鏄惁澶勭悊瀹屾垚鐨勬爣蹇楋紝QOSA_TRUE琛ㄧず澶勭悊缁撴潫锛孮OSA_FALSE琛ㄧず鏈粨鏉? */
static qosa_bool_t quec_ping_result_handler(ping_demo_resp_t *ping_ptr)
{
    char         resp_buf[256] = {0};            // 鐢ㄤ簬瀛樺偍鏍煎紡鍖栧悗鐨勫搷搴斿瓧绗︿覆
    qosa_int32_t resp_len = 0;                   // 鍝嶅簲瀛楃涓查暱搴︼紙褰撳墠鏈疄闄呬娇鐢級
    qosa_int32_t evt_code = ping_ptr->evt_code;  // 鎻愬彇浜嬩欢鐮?    qosa_bool_t  finish = QOSA_FALSE;            // 鏄惁瀹屾垚鏍囧織锛岄粯璁や负鏈畬鎴?
    // 鎵撳嵃浜嬩欢ID鍜屼簨浠剁爜锛屼究浜庤皟璇曡拷韪?    quec_ping_log("event_id=%d,%x", ping_ptr->event_id, evt_code);

    // 鏍规嵁涓嶅悓鐨勪簨浠禝D杩涜澶勭悊
    switch (ping_ptr->event_id)
    {
        // 澶勭悊鍗曟PING鐨勭粺璁′俊鎭簨浠?        case QCM_PING_STATS: {
            // 鍙湁鍦ㄤ簨浠剁爜涓烘垚鍔熺殑鎯呭喌涓嬫墠澶勭悊
            if (evt_code == QCM_PING_OK)
            {
                // 鑾峰彇PING鐘舵€佷俊鎭粨鏋勪綋鎸囬拡
                qcm_ping_stats_type *stats = &ping_ptr->resp_ptr.type.status;
                if (stats != QOSA_NULL)
                {
                    // 鏍煎紡鍖朠ING鐘舵€佷俊鎭?                    resp_len += qosa_snprintf(resp_buf, 256, "\"%s\",%ld,%ld,%ld", stats->resolved_ip_addr, stats->ping_size, stats->ping_rtt, stats->ping_ttl);
                    // 鎵撳嵃PING鐘舵€佷俊鎭?                    quec_ping_log("PING: [%s]", resp_buf);
                }
            }
        }
        break;
        // 澶勭悊PING缁撴潫鏃剁殑姹囨€讳俊鎭簨浠?        case QCM_PING_SUMMARY: {
            // 鍙湁鍦ㄤ簨浠剁爜涓烘垚鍔熺殑鎯呭喌涓嬫墠澶勭悊
            if (evt_code == QCM_PING_OK)
            {
                // 鑾峰彇PING姹囨€讳俊鎭粨鏋勪綋鎸囬拡
                qcm_ping_summary_type *summary = &ping_ptr->resp_ptr.type.summary;
                if (summary != QOSA_NULL)
                {
                    // 鏍煎紡鍖朠ING姹囨€讳俊鎭?                    resp_len += qosa_snprintf(
                        resp_buf,
                        256,
                        "%ld,%ld,%ld,%ld,%ld,%ld",
                        summary->num_pkts_sent,
                        summary->num_pkts_recvd,
                        summary->num_pkts_lost,
                        summary->min_rtt,
                        summary->max_rtt,
                        summary->avg_rtt
                    );
                    // 鎵撳嵃PING缁撴潫鐨勬眹鎬讳俊鎭?                    quec_ping_log("PING_END: [%s]", resp_buf);
                }
            }
            // 鏍囪澶勭悊瀹屾垚
            finish = QOSA_TRUE;
        }
        break;
        default:
            break;
    }
    // 杩斿洖澶勭悊瀹屾垚鏍囧織
    return finish;
}


/**
 * @brief ping鍔熻兘婕旂ず澶勭悊鍑芥暟
 *
 * 璇ュ嚱鏁板疄鐜颁簡涓€涓猵ing鎿嶄綔鐨勫畬鏁存祦绋嬶紝鍖呮嫭鍒濆鍖杙ing鍙傛暟銆佸惎鍔╬ing鎿嶄綔銆? * 绛夊緟骞跺鐞唒ing缁撴灉绛夋楠ゃ€傚嚱鏁颁細鍦ㄥ惎鍔╬ing鍚庤繘鍏ュ惊鐜瓑寰呯姸鎬侊紝鐩村埌
 * ping鎿嶄綔瀹屾垚鎴栧嚭鐜伴敊璇€? */
static void quec_ping_demo_process(void *ctx)
{
    qcm_ping_config_type ping_options = {0};
    ping_demo_resp_t     rsp = {0};
    qcm_ping_error_e     ret = 0;
    qosa_bool_t          is_finish = QOSA_FALSE;

    // 绛夊緟10绉掞紝鏂逛究鎶撳彇log鍜屽紑鏈烘敞缃?    qosa_task_sleep_sec(10);

    // 瀵硅繘琛宲ing鎿嶄綔鐨勬暟鎹ぇ灏忥紝娆℃暟锛岃秴鏃舵椂闂磋祴鍊硷紝濡傛灉娌℃湁鍙傛暟锛岄粯璁ゅ€?    ping_options.num_data_bytes = 64;
    ping_options.num_pings = QUEC_PING_DEMO_CNT;
    ping_options.ping_response_time_out = QUEC_PING_DEMO_TIMEOUT;
    ping_options.ttl = QUEC_PING_DEMO_TTL;

    // 鍚姩ping鎿嶄綔锛屾寚瀹歅DPID銆丼IM鍗D銆佹湇鍔″櫒鍦板潃绛夊弬鏁?    ret = qcm_ping_start(QUEC_PING_DEMO_PDPID, QUEC_PING_DEMO_SIMID, QUEC_PING_DEMO_SERVER, &ping_options, quec_ping_result_cb, QOSA_NULL);
    if (ret != QCM_PING_OK)
    {
        quec_ping_log("ping start err =%x", ret);
        qosa_msgq_delete(g_ping_demo_msg);
        return;
    }

    // 寰幆绛夊緟骞跺鐞唒ing缁撴灉娑堟伅
    while (1)
    {
        // 绛夊緟鍚屾缁撴灉娑堟伅骞惰繘琛屽鐞?        qosa_msgq_wait(g_ping_demo_msg, (qosa_uint8_t *)&rsp, sizeof(ping_demo_resp_t), QOSA_WAIT_FOREVER);
        is_finish = quec_ping_result_handler(&rsp);
        if (is_finish)
        {
            // ping 澶勭悊缁撴潫
            break;
        }
    }
    qosa_msgq_delete(g_ping_demo_msg);
    quec_ping_log("PING END");
}

void quec_demo_ping_init(void)
{
    quec_ping_log("enter Quectel PING DEMO !!!");
    if (g_ping_demo_msg == QOSA_NULL)
    {
        qosa_msgq_create(&g_ping_demo_msg, sizeof(ping_demo_resp_t), 10);
    }
    if (g_ping_demo_task == QOSA_NULL)
    {
        qosa_task_create(&g_ping_demo_task, QUEC_PING_DEMO_TASK_STACK_SIZE, QOSA_PRIORITY_NORMAL, "ping_demo", quec_ping_demo_process, QOSA_NULL);
    }
}

UNIRTOS_APP_EXPORT(200, "ping_demo", quec_demo_ping_init);
