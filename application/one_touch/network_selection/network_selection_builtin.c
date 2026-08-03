#include "qosa_def.h"

#define NETWORK_SELECTION_BUILTIN_DEMO_SIM_ID 0

static void network_selection_builtin_init(void)
{
    qosa_uint8_t sim_id = NETWORK_SELECTION_BUILTIN_DEMO_SIM_ID;

    qapp_easy_nw_init_acc_tpl_write(sim_id, QAPP_EASY_NW_INIT_ACC_TPL_INTERNAL_DEFAULT);
}