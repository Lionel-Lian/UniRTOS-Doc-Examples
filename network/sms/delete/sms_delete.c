#include "qosa_def.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

static void sms_delete_demo_init(void)
{
    QLOGI("sms delete demo snippet extracted from network/sms/demo/sms_demo.c");
}

UNIRTOS_APP_EXPORT(200, "sms_delete_demo", sms_delete_demo_init);