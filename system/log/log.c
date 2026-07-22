#include "qosa_log.h"

#define QOS_LOG_TAG  LOG_TAG_COMPONENT

static void qlog_print_once(void)
{
    /* QLOGE: error级别日志，用于打印错误信息。 */
    QLOGE("this is error message");

    /* QLOGW: warning级别日志，用于打印警告信息。 */
    QLOGW("this is warning message");

    /* QLOGI: info级别日志，用于打印普通流程信息。 */
    QLOGI("this is info message");

    /* QLOGD: debug级别日志，用于打印调试信息。 */
    QLOGD("this is debug message");

    /* QLOGV: verbose级别日志，用于打印更详细的跟踪信息。 */
    QLOGV("this is verbose message");

    /* QLOGV_EX: verbose扩展日志，用于快速打印详细调试信息。 */
    QLOGV_EX("快速调试信息");
}

void qlog_level_demo(void)
{
    qosa_log_control_set(QOSA_LOG_BIT_MASTER_ENABLE | QOSA_LOG_BIT_USB);

    while (1) {
        qlog_print_once();
        qosa_task_sleep_sec(3);
    }
}

void main_demo(void)
{
    qlog_level_demo();
}