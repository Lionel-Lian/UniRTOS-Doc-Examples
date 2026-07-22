#include "mbedtls/rsa.h"
#include "qosa_sys.h"

// 假设此时 ctx 内已经被导入了 RSA-2048 的 Public Key。
void demo_rsa_encrypt(mbedtls_rsa_context *ctx) {
    int ret;
    unsigned char plain_text[] = "SecretData!";
    size_t plain_len = sizeof(plain_text) - 1;
    unsigned char cipher_text[256]; // 2048 Bit = 256 bytes

    /* 执行公钥加密 */
    ret = mbedtls_rsa_pkcs1_encrypt(ctx, 
                                    NULL, NULL, // 不使用 RNG
                                    MBEDTLS_RSA_PUBLIC, 
                                    plain_len, 
                                    plain_text, 
                                    cipher_text);
    if(ret == 0) {
        // cipher_text 中即为输出的 256 字节的密文。
    }
}