// DNS_DEMO_TASK_STACK_SIZE 4096
#include "qosa_asyn_dns.h"
#include "qosa_log.h"
#include "qosa_def.h"
#include "qosa_sys.h"
#include "qcm_socket_adp.h"
#include "qosa_asyn_dns.h"
#include "qosa_datacall.h"
#include "qosa_sockets.h"
#include "unirtos_app_init_registry.h"

#define QOS_LOG_TAG                   LOG_TAG
// DNS 闇€瑕侀厤缃殑鍙傛暟
#define QUEC_DNS_DEMO_SIMID           0
#define QUEC_DNS_DEMO_PDPID           1
#define QUEC_DNS_DEMO_HOSTNAME        "www.baidu.com"
#define QUEC_DNS_DEMO_TASK_STACK_SIZE 4096
/**
 * @brief dns鍥炶皟鍝嶅簲鏁版嵁缁撴瀯浣? *
 * 璇ョ粨鏋勪綋鐢ㄤ簬瀛樺偍dns鎿嶄綔鐨勫搷搴斾俊鎭紝鍖呮嫭閿欒鐮併€? * 鍝嶅簲鏁版嵁浠ュ強鐢ㄦ埛鑷畾涔夊弬鏁? */
typedef struct
{
    qosa_dns_error_e        evt_code;   /**< dns浜嬩欢閿欒鐮?*/
    struct qosa_addrinfo_s *info;       /**< dns鍝嶅簲鏁版嵁鎸囬拡 */
    void                   *user_param; /**< 鐢ㄦ埛鑷畾涔夊弬鏁版寚閽?*/
} dns_demo_resp_t;

static qosa_task_t g_dns_demo_task = QOSA_NULL;
static qosa_msgq_t g_dns_demo_msg = QOSA_NULL;

/**
 * @brief dns缁撴灉鍥炶皟鍑芥暟
 *
 * 璇ュ嚱鏁扮敤浜庡鐞哾ns鎿嶄綔鐨勭粨鏋滃洖璋冿紝灏哾ns鍝嶅簲鏁版嵁杩涜灏佽骞堕€氳繃娑堟伅闃熷垪鍙戦€? *
 * @param argv 鐢ㄦ埛鑷畾涔夊弬鏁? * @param info dns瑙ｆ瀽缁撴灉鎸囬拡
 * @param type dns閿欒鐮? */
static void quec_dns_result_cb(void *argv, struct qosa_addrinfo_s **info, qosa_dns_error_e type)
{
    dns_demo_resp_t dns_rsp = {0};

    dns_rsp.evt_code = type;
    dns_rsp.info = *info;  // 娉ㄦ剰锛氳繖閲屽鍒舵寚閽堬紝瀹為檯浣跨敤鏃堕渶灏忓績鍐呭瓨绠＄悊
    dns_rsp.user_param = argv;

    /* 鍙戦€佹秷鎭?*/
    qosa_msgq_release(g_dns_demo_msg, sizeof(dns_demo_resp_t), (qosa_uint8_t *)&dns_rsp, QOSA_NO_WAIT);
}

/**
 * @brief 澶勭悊DNS鎿嶄綔鐨勭粨鏋滃洖璋冨嚱鏁帮紝鏍规嵁绫诲瀷瑙ｆ瀽骞惰褰旸NS淇℃伅銆? *
 * 璇ュ嚱鏁板鐞嗘潵鑷狣NS妯″潡鐨勫搷搴斾簨浠讹紝鏍规嵁閿欒鐮佸鐞嗚В鏋愮粨鏋滐紝
 * 骞跺皢IP鍦板潃淇℃伅閫氳繃鏃ュ織杈撳嚭銆傚鐞嗗畬鎴愬悗閲婃斁鍐呭瓨骞舵爣璁板畬鎴愩€? *
 * @param[in] dns_ptr 鎸囧悜DNS鍝嶅簲鏁版嵁缁撴瀯鐨勬寚閽堬紝鍖呭惈閿欒鐮佸強鍝嶅簲鍐呭
 * @return 杩斿洖鏄惁澶勭悊瀹屾垚鐨勬爣蹇楋紝QOSA_TRUE琛ㄧず澶勭悊缁撴潫锛孮OSA_FALSE琛ㄧず鏈粨鏉? */
static qosa_bool_t quec_dns_result_handler(dns_demo_resp_t *dns_ptr)
{
    qosa_int32_t evt_code = dns_ptr->evt_code;  // 鎻愬彇浜嬩欢鐮?    qosa_bool_t  finish = QOSA_FALSE;           // 鏄惁瀹屾垚鏍囧織锛岄粯璁や负鏈畬鎴?
    // 鎵撳嵃浜嬩欢鐮侊紝渚夸簬璋冭瘯杩借釜
    QLOGD("evt_code=%x", evt_code);

    // 澶勭悊DNS瑙ｆ瀽缁撴灉
    if (evt_code == QOSA_DNS_RESULT_OK)
    {
        struct qosa_addrinfo_s *info = dns_ptr->info;
        if (info != QOSA_NULL)
        {
            while (info != QOSA_NULL)
            {
                // 鎵撳嵃IP鍦板潃鍜屽鏃忕被鍨?                QLOGD("IP: %s, Family: %d", info->ip_addr, info->ai_family);
                info = info->ai_next;
            }
            // 閲婃斁鍐呭瓨
            qosa_dns_result_free(dns_ptr->info);
        }
    }
    else
    {
        QLOGD("DNS failed: %x", evt_code);
    }
    // 鏍囪澶勭悊瀹屾垚
    finish = QOSA_TRUE;

    // 杩斿洖澶勭悊瀹屾垚鏍囧織
    return finish;
}

/**
 * @brief dns鍔熻兘婕旂ず澶勭悊鍑芥暟
 *
 * 璇ュ嚱鏁板疄鐜颁簡涓€涓猟ns鎿嶄綔鐨勫畬鏁存祦绋嬶紝鍖呮嫭鍒濆鍖杁ns鍙傛暟銆佸惎鍔╠ns鎿嶄綔銆? * 绛夊緟骞跺鐞哾ns缁撴灉绛夋楠ゃ€傚嚱鏁颁細鍦ㄥ惎鍔╠ns鍚庤繘鍏ュ惊鐜瓑寰呯姸鎬侊紝鐩村埌
 * dns鎿嶄綔瀹屾垚鎴栧嚭鐜伴敊璇€? */
static void quec_dns_demo_process(void *ctx)
{
    struct qosa_addrinfo_s hints = {0};
    dns_demo_resp_t        rsp = {0};
    qosa_dns_error_e       ret = 0;
    qosa_bool_t            is_finish = QOSA_FALSE;

    // 绛夊緟10绉掞紝鏂逛究鎶撳彇log鍜岀綉缁滃氨缁?    qosa_task_sleep_sec(10);

    // 閰嶇疆hints锛屾寚瀹欼Pv4锛圓F_INET锛夋垨IPv6锛圓F_INET6锛?    hints.ai_family = AF_INET;  // 绀轰緥浣跨敤IPv4

    // 鍚姩dns寮傛鎿嶄綔锛屾寚瀹歋IMID銆丳DPID銆佸煙鍚嶇瓑鍙傛暟
    ret = qosa_dns_asyn_getaddrinfo(QUEC_DNS_DEMO_SIMID, QUEC_DNS_DEMO_PDPID, QUEC_DNS_DEMO_HOSTNAME, &hints, quec_dns_result_cb, QOSA_NULL);
    if (ret != QOSA_DNS_RESULT_OK)
    {
        QLOGD("dns start err =%x", ret);
        qosa_msgq_delete(g_dns_demo_msg);
        return;
    }

    // 寰幆绛夊緟骞跺鐞哾ns缁撴灉娑堟伅
    while (1)
    {
        // 绛夊緟鍚屾缁撴灉娑堟伅骞惰繘琛屽鐞?        qosa_msgq_wait(g_dns_demo_msg, (qosa_uint8_t *)&rsp, sizeof(dns_demo_resp_t), QOSA_WAIT_FOREVER);
        is_finish = quec_dns_result_handler(&rsp);
        if (is_finish)
        {
            // dns 澶勭悊缁撴潫
            break;
        }
    }
    qosa_msgq_delete(g_dns_demo_msg);
    QLOGD("DNS END");
}

void quec_demo_dns_init(void)
{
    QLOGD("enter Quectel DNS DEMO !!!");
    if (g_dns_demo_msg == QOSA_NULL)
    {
        qosa_msgq_create(&g_dns_demo_msg, sizeof(dns_demo_resp_t), 10);
    }
    if (g_dns_demo_task == QOSA_NULL)
    {
        qosa_task_create(&g_dns_demo_task, QUEC_DNS_DEMO_TASK_STACK_SIZE, QOSA_PRIORITY_NORMAL, "dns_demo", quec_dns_demo_process, QOSA_NULL);
    }
}

UNIRTOS_APP_EXPORT(200, "dns_demo", quec_demo_dns_init);
