#include "qcm_sha256.h"
#include "qosa_sys.h"

void app_sha_demo_long_file_sim() {
    qcm_core_sha256_context_t ctx;
    qosa_uint8_t final_res[32] = {0};
    qosa_uint8_t file_chunk_buf[1024]; // 假定你每次只舍得给文件分配1k字节用来承接

    // 1. 准备并初始化引擎
    qcm_sha256_init(&ctx); 
    qcm_sha256_starts(&ctx); 
    
    // 2. 模拟从某个大型输入流里进行分片循环处理
    while(/* 文件还没读完，并每次假设读取到片断存入 file_chunk_buf */) {
        qcm_sha256_update(&ctx, file_chunk_buf, 1024 /*真实读取长度*/);
    }

    // 3. 收尾并获得最终拼接产生的宏观结果
    qcm_sha256_finish(&ctx, final_res);

    // 4. 清理内存工作
    qcm_sha256_free(&ctx);
}