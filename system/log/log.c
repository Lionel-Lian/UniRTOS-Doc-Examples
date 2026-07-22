#include "qosa_log.h"

#define QOS_LOG_TAG  LOG_TAG_COMPONENT

static void demo_qlog_error(void)
{
    int ret = -1;

    QLOGI("1. QLOGE demo start");

    if (ret != 0) {
        /* QLOGE: 用于输出错误日志，通常表示当前操作失败或出现不可忽略的问题。 */
        QLOGE("QLOGE: operation failed, ret=%d", ret);
    }
}

static void demo_qlog_warning(void)
{
    int battery_level = 18;
    int signal_level = 1;

    QLOGI("2. QLOGW demo start");

    if (battery_level < 20) {
        /* QLOGW: 用于输出警告日志，表示系统还能继续运行，但需要关注当前状态。 */
        QLOGW("QLOGW: low battery, level=%d%%", battery_level);
    }

    if (signal_level <= 1) {
        /* QLOGW: 适合记录弱信号、资源偏低等可能影响后续业务的异常趋势。 */
        QLOGW("QLOGW: weak signal, level=%d", signal_level);
    }
}

static void demo_qlog_info(void)
{
    const char *module_name = "log_demo";

    QLOGI("3. QLOGI demo start");
    /* QLOGI: 用于输出普通流程信息，适合记录模块初始化、状态切换等关键节点。 */
    QLOGI("QLOGI: module %s init success", module_name);
    /* QLOGI: 适合记录正常业务流程中的重要状态，便于确认程序运行路径。 */
    QLOGI("QLOGI: service state changed to ready");
}

static void demo_qlog_debug(void)
{
    int socket_id = 3;
    int send_len = 128;
    int recv_len = 256;

    QLOGI("4. QLOGD demo start");
    /* QLOGD: 用于输出调试日志，适合记录开发调试时关注的变量值。 */
    QLOGD("QLOGD: socket_id=%d", socket_id);
    /* QLOGD: 适合记录函数参数、长度、状态码等排查问题时需要的上下文。 */
    QLOGD("QLOGD: send_len=%d, recv_len=%d", send_len, recv_len);
}

static void demo_qlog_verbose(void)
{
    int index = 0;

    QLOGI("5. QLOGV demo start");

    for (index = 0; index < 3; index++) {
        /* QLOGV: 用于输出更详细的跟踪日志，适合循环、状态机等高频细节。 */
        QLOGV("QLOGV: loop detail, index=%d", index);
    }

    /* QLOGV: 适合记录进入某个细粒度代码路径，便于分析执行顺序。 */
    QLOGV("QLOGV: enter detail trace point");
}

void qlog_level_demo(void)
{
    qosa_log_control_set(QOSA_LOG_BIT_MASTER_ENABLE | QOSA_LOG_BIT_USB);

    QLOGI("===== qlog level demo start =====");

    demo_qlog_error();
    demo_qlog_warning();
    demo_qlog_info();
    demo_qlog_debug();
    demo_qlog_verbose();

    QLOGI("===== qlog level demo end =====");
}

void main_demo(void)
{
    qlog_level_demo();
}