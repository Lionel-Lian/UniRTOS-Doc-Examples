#include "qosa_def.h"

#define QOS_LOG_TAG  LOG_TAG_COMPONENT

// 随机数接口声明
int qosa_rand(void);

// 生成 0-99 的随机数
int get_random_less_than_100(void) {
    int rand_val = qosa_rand();
    int result = rand_val % 100;  // 取模运算得到 0-99
    QLOGD("原始随机数: %d, 转换后: %d", rand_val, result);
    return result;
}

// 生成 1000-10000 的随机数
int get_random_1000_to_10000(void) {
    int rand_val = qosa_rand();
    // 先取模得到 0-9000，然后加上 1000
    int result = (rand_val % 9001) + 1000;  // 9001 = 10000 - 1000 + 1
    QLOGD("原始随机数: %d, 转换后: %d", rand_val, result);
    return result;
}

// 主演示函数
void demo_random(void) {
    // 初始化日志
    qosa_log_control_set(QOSA_LOG_BIT_MASTER_ENABLE | QOSA_LOG_BIT_USB);
    
    QLOGI("===== 随机数生成演示 =====");
    
    // 生成 10 个小于 100 的随机数
    QLOGI("生成 10 个小于 100 的随机数:");
    for (int i = 0; i < 10; i++) {
        int num = get_random_less_than_100();
        QLOGI("随机数 %d: %d", i + 1, num);
    }
    
    // 生成 10 个 1000-10000 的随机数
    QLOGI("\n生成 10 个 1000-10000 的随机数:");
    for (int i = 0; i < 10; i++) {
        int num = get_random_1000_to_10000();
        QLOGI("随机数 %d: %d", i + 1, num);
    }
    
    QLOGI("===== 随机数演示结束 =====");
}