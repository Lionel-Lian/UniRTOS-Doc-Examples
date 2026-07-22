#include "qcm_sha256.h"
#include "qosa_sys.h"

void app_sha_demo_short() {
    qosa_uint8_t input_msg[] = "Hello_QuecOS_2026!";
    qosa_uint8_t hash_res[32] = {0};

    // 一步到位，调用完成此时 hash_res 内就存储着 32字节 的结果
    qcm_core_sha256(input_msg, sizeof(input_msg) - 1, hash_res);
}