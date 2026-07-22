/**
 * @file ssl_blocking_demo.c
 * @brief 阻塞式SSL连接示例
 * @author embedded_dev@quectel.com
 * @date 2026-01-11
 *
 * @copyright Copyright (c) 2026 Quectel Wireless Solution, Co., Ltd.
 */
#include "qosa_def.h"
#include "qosa_sys.h"
#include "qcm_vtls.h"
#include "qosa_log.h"
#include "qosa_sockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define QOS_LOG_TAG                    "SSL_DEMO"
#define SSL_DEMO_SERVER_PORT          443
#define SSL_DEMO_SERVER_NAME          "www.example.com"
#define BUFFER_SIZE                   1024

/* 自定义IO函数 */
static int blocking_io_read(void *ctx, unsigned char *buf, qosa_size_t len)
{
    int                     ret = 0;
    qcm_ssl_connect_data_t *ssl_ctx = (qcm_ssl_connect_data_t *)ctx;
    
    ret = read(ssl_ctx->ssl_config.socket_fd, buf, len);
    QLOGD("读取数据: %d字节", ret);
    
    if (ret <= 0) {
        ret = QCM_VTLS_IO_ERR_SSL_CONN_RESET;
    }
    
    return ret;
}

static int blocking_io_write(void *ctx, const unsigned char *buf, qosa_size_t len)
{
    int                     ret = 0;
    qcm_ssl_connect_data_t *ssl_ctx = (qcm_ssl_connect_data_t *)ctx;
    
    ret = write(ssl_ctx->ssl_config.socket_fd, buf, len);
    QLOGD("写入数据: %d字节", ret);
    
    if (ret <= 0) {
        ret = QCM_VTLS_IO_ERR_SSL_CONN_RESET;
    }
    
    return ret;
}

static int blocking_io_select(void *ctx, qosa_bool_t read_flag, qosa_bool_t write_flag, qosa_uint32_t wait_time)
{
    fd_set                  readset;
    fd_set                  writeset;
    int                     ret = 0;
    struct timeval          tm;
    qcm_ssl_connect_data_t *ssl_ctx = (qcm_ssl_connect_data_t *)ctx;
    
    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    
    if (read_flag == QOSA_TRUE) {
        FD_SET(ssl_ctx->ssl_config.socket_fd, &readset);
    }
    
    if (write_flag == QOSA_TRUE) {
        FD_SET(ssl_ctx->ssl_config.socket_fd, &writeset);
    }
    
    tm.tv_sec = wait_time;
    tm.tv_usec = 0;
    
    ret = select(ssl_ctx->ssl_config.socket_fd + 1, &readset, &writeset, QOSA_NULL, &tm);
    QLOGD("select返回: %d", ret);
    
    return ret;
}

/* 创建socket连接 */
static int create_socket_connection(const char *server_ip, uint16_t port)
{
    int                client_socket = -1;
    struct sockaddr_in server_addr = {0};
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(port);
    
    // 创建socket
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client_socket == -1) {
        QLOGE("创建socket失败");
        return -1;
    }
    
    // 连接服务器
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(client_socket);
        QLOGE("连接服务器失败");
        return -1;
    }
    
    QLOGD("成功连接到服务器: %s:%d", server_ip, port);
    return client_socket;
}

/* 主函数 */
int main(void)
{
    qcm_ssl_config_t        ssl_config = {0};
    qcm_ssl_connect_data_t *ssl_conn = NULL;
    int                     client_socket = -1;
    char                    server_ip[INET_ADDRSTRLEN] = "93.184.216.34"; // example.com的IP
    char                    buffer[BUFFER_SIZE];
    qcm_vtls_result_status_e result;
    
    QLOGI("开始SSL阻塞式连接示例");
    
    // 1. 初始化SSL模块
    if (qcm_ssl_init() == 0) {
        QLOGE("SSL初始化失败");
        return -1;
    }
    QLOGD("SSL初始化成功");
    
    // 2. 配置SSL参数
    ssl_config.ssl_version = QCM_SSL_VERSION_3;  // TLS 1.2
    ssl_config.transport = QCM_SSL_TLS_PROTOCOL;
    ssl_config.auth_mode = QCM_SSL_VERIFY_SERVER;
    ssl_config.sni_enable = QOSA_TRUE;
    ssl_config.ssl_negotiate_timeout = 30;
    ssl_config.io_read = blocking_io_read;
    ssl_config.io_write = blocking_io_write;
    ssl_config.io_select = blocking_io_select;
    
    // 3. 创建SSL连接对象
    ssl_conn = qcm_ssl_new(&ssl_config);
    if (ssl_conn == NULL) {
        QLOGE("创建SSL连接对象失败");
        qcm_ssl_clean_all_sessionid();
        return -1;
    }
    QLOGD("SSL连接对象创建成功");
    
    // 4. 创建socket连接
    client_socket = create_socket_connection(server_ip, SSL_DEMO_SERVER_PORT);
    if (client_socket < 0) {
        QLOGE("创建socket连接失败");
        qcm_ssl_free(ssl_conn);
        return -1;
    }
    
    // 设置socket句柄
    ssl_conn->ssl_config.socket_fd = (qosa_ptr)client_socket;
    
    // 5. 设置主机信息（SNI）
    result = qcm_ssl_set_hostinfo(ssl_conn, SSL_DEMO_SERVER_NAME, SSL_DEMO_SERVER_PORT);
    if (result != QCM_VTLS_RESULT_OK) {
        QLOGE("设置主机信息失败: %d", result);
        close(client_socket);
        qcm_ssl_free(ssl_conn);
        return -1;
    }
    QLOGD("主机信息设置成功");
    
    // 6. 建立SSL连接
    QLOGD("开始SSL握手...");
    result = qcm_ssl_connect(ssl_conn);
    if (result != QCM_VTLS_RESULT_OK) {
        QLOGE("SSL连接失败: %d", result);
        close(client_socket);
        qcm_ssl_free(ssl_conn);
        return -1;
    }
    QLOGD("SSL连接成功建立");
    
    // 7. 发送HTTP请求
    const char *http_request = "GET / HTTP/1.1\r\n"
                               "Host: www.example.com\r\n"
                               "Connection: close\r\n"
                               "\r\n";
    
    qosa_size_t bytes_written = qcm_ssl_write(ssl_conn, (char *)http_request, 
                                              strlen(http_request), &result);
    if (result != QCM_VTLS_RESULT_OK) {
        QLOGE("发送HTTP请求失败: %d", result);
    } else {
        QLOGD("成功发送%zu字节HTTP请求", bytes_written);
    }
    
    // 8. 接收HTTP响应
    QLOGD("开始接收HTTP响应...");
    int total_received = 0;
    
    while (1) {
        qosa_size_t bytes_read = qcm_ssl_read(ssl_conn, buffer, BUFFER_SIZE - 1, &result);
        
        if (result == QCM_VTLS_SSL_PEER_CLOSE_NOTIFY_ERR) {
            QLOGD("对端已关闭连接");
            break;
        }
        
        if (result != QCM_VTLS_RESULT_OK && result != QCM_VTLS_SSL_READ_WRITE_EAGAIN) {
            QLOGE("读取数据失败: %d", result);
            break;
        }
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            total_received += bytes_read;
            QLOGD("收到%zu字节数据，总接收: %d字节", bytes_read, total_received);
            
            // 这里可以处理接收到的数据
            // printf("%.*s", (int)bytes_read, buffer);
        }
        
        if (bytes_read == 0) {
            // 没有更多数据
            break;
        }
    }
    
    QLOGD("HTTP响应接收完成，总共接收%d字节", total_received);
    
    // 9. 清理资源
    QLOGD("开始清理资源...");
    qcm_ssl_close(ssl_conn);
    close(client_socket);
    qcm_ssl_free(ssl_conn);
    qcm_ssl_clean_all_sessionid();
    
    QLOGI("SSL示例执行完成");
    return 0;
}