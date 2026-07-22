#include "qosa_log.h"

// 定义模块标签
#define QOS_LOG_TAG  LOG_TAG_COMPONENT

// 示例1：基本使用
void demo_basic(void) {
    // 启用日志输出
    qosa_log_control_set(QOSA_LOG_BIT_MASTER_ENABLE | QOSA_LOG_BIT_USB);
    
    QLOGE("系统错误: 内存分配失败");
    QLOGW("内存使用率: 85%%");
    QLOGI("系统初始化完成");
    QLOGD("数据包大小: %d 字节", 1024);
    QLOGV("详细调试: x=%d, y=%d", 10, 20);
    QLOGV_EX("快速调试信息");
}

// 示例2：函数中使用
int demo_calculate(int a, int b) {
    QLOGI("执行计算: a=%d, b=%d", a, b);
    
    if (b == 0) {
        QLOGE("除数不能为0");
        return -1;
    }
    
    int result = a / b;
    QLOGD("计算结果: %d", result);
    return result;
}

// 示例3：循环中的进度日志
void demo_loop(int count) {
    QLOGI("开始处理 %d 个任务", count);
    
    for (int i = 0; i < count; i++) {
        if (i % 10 == 0) {
            QLOGV("进度: %d/%d", i, count);
        }
        // 模拟处理
    }
    
    QLOGI("处理完成");
}

// 主函数
void main_demo(void) {
    // 初始化日志
    qosa_log_control_set(QOSA_LOG_BIT_MASTER_ENABLE | QOSA_LOG_BIT_USB);
    
    QLOGI("===== 日志演示开始 =====");
    
    // 测试各种功能
    demo_basic();
    demo_calculate(100, 5);
    demo_calculate(100, 0);  // 触发错误
    demo_loop(25);
    
    QLOGI("===== 日志演示结束 =====");
}