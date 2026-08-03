#include "qosa_def.h"

#define PROFILE_BUILTIN_DEMO_SIM_ID 0
#define PROFILE_BUILTIN_DEMO_PDP_ID 1

static void profile_builtin_init(void)
{
    qosa_uint8_t sim_id = PROFILE_BUILTIN_DEMO_SIM_ID;
    qosa_uint8_t pdp_id = PROFILE_BUILTIN_DEMO_PDP_ID;

    // 将【内置模板中国移动】的拨号参数，下发写入到指定SIM卡、指定PDP链路中，完成拨号参数配置生效
    qapp_easy_nw_datacall_prof_tpl_write(sim_id, pdp_id, QAPP_EASY_NW_DATACALL_PROF_TPL_INTERNAL_CMNET_IPV4V6);
}