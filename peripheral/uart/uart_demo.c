/*****************************************************************/ /**
* @file uart_demo.c
* @brief
* @author bronson.zhan@quectel.com
* @date 2025-04-23
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2025-04-23 <td>1.0 <td>Bronson.Zhan <td> Init
* </table>
**********************************************************************/
#include "qosa_sys.h"
#include "qosa_uart.h"
#include "qosa_def.h"
#include "qosa_log.h"

#include "uart_demo.h"
#include "unirtos_app_init_registry.h"

/*===========================================================================
 *  Macro Definition
 ===========================================================================*/

#define QOS_LOG_TAG                         LOG_TAG_UART_API

/*===========================================================================
 *  Variate
 ===========================================================================*/

static qosa_task_t  g_quec_uart_demo_task = QOSA_NULL;
static qosa_uint8_t g_uart_data[1024] = {0};

static qosa_uint16_t g_uart_test_case = QOSA_UART_DEMO_OUTPUT;

/*===========================================================================
 *  Static API Functions
 ===========================================================================*/
 /**
 * @brief UART浜嬩欢鍥炶皟澶勭悊鍑芥暟
 * 
 * 璇ュ嚱鏁扮敤浜庡鐞哢ART绔彛鐨勫悇绉嶄簨浠舵寚绀猴紝鍖呮嫭鎺ユ敹鏁版嵁銆佸彂閫佸畬鎴愬拰鍙戦€佺紦鍐插尯浣庢按浣嶇瓑浜嬩欢銆? * 褰撲簨浠跺彂鐢熸椂锛屼細灏嗕簨浠朵俊鎭€氳繃UART鍙戦€佸嚭鍘汇€? * 
 * @param cb_param UART鍥炶皟鍙傛暟缁撴瀯浣撴寚閽堬紝鍖呭惈绔彛鍙枫€佷簨浠禝D鍜岀敤鎴锋暟鎹瓑淇℃伅
 */
static void quec_uart_ind(qosa_uart_cb_param_t *cb_param)
{
    qosa_uart_port_number_e port = cb_param->port;
    qosa_uint32_t           event_id = cb_param->event_id;
    char                    data[128] = {0};
    qosa_snprintf(data, sizeof(data), "port=%d, event_id=%d, user_data=%s", port, event_id, (unsigned char *)cb_param->user_data);
    // 鏍规嵁涓嶅悓鐨刄ART浜嬩欢绫诲瀷杩涜鐩稿簲澶勭悊
    if (cb_param->event_id & QOSA_UART_EVENT_RX_INDICATE)
    {
        qosa_uart_write(port, (unsigned char *)data, sizeof(data));
    }
    else if (cb_param->event_id & QOSA_UART_EVENT_TX_COMPLETE)
    {
        qosa_uart_write(port, (unsigned char *)data, sizeof(data));
    }
    else if (cb_param->event_id & QOSA_UART_EVENT_TX_LOW)
    {
        qosa_uart_write(port, (unsigned char *)data, sizeof(data));
    }
}

/**
 * @brief UART鍔熻兘澶勭悊鍑芥暟锛岀敤浜庨厤缃拰娴嬭瘯UART鎺ュ彛鐨勫悇绉嶅姛鑳姐€? *
 * 璇ュ嚱鏁板垵濮嬪寲UART绔彛锛屾敞鍐屽洖璋冨嚱鏁帮紝骞舵牴鎹叏灞€鍙橀噺 g_uart_test_case 鐨勫€兼墽琛屼笉鍚岀殑娴嬭瘯鐢ㄤ緥锛? * 鍖呮嫭杈撳嚭鏁版嵁銆佽鍙栨暟鎹€佹尝鐗圭巼鍒囨崲浠ュ強妯″紡鍒囨崲绛夋搷浣溿€? *
 */
 static void quec_uart_demo_process(void *ctx)
{
    int ret = 0;

    qosa_uart_status_monitor_t monitor = {0};
    monitor.callback = quec_uart_ind; /* 娉ㄥ唽鍥炶皟鍑芥暟 */
    monitor.event_mask = QOSA_UART_EVENT_RX_INDICATE | QOSA_UART_EVENT_TX_COMPLETE;
    monitor.user_data = "Hello, Uart!";
    
    /* 娉ㄥ唽UART浜嬩欢鍥炶皟 */
    qosa_uart_register_cb(QUEC_TEST_UART_PORT, &monitor);

    /* 閰嶇疆UART閫氫俊鍙傛暟锛氭尝鐗圭巼銆佹暟鎹綅銆佸仠姝綅銆佹牎楠屼綅銆佹祦鎺?*/
    qosa_uart_config_t dcb_config = {0};
    dcb_config.baudrate = QOSA_UART_BAUD_115200;
    dcb_config.data_bit = QOSA_UART_DATABIT_8;
    dcb_config.flow_ctrl = QOSA_FC_NONE;
    dcb_config.parity_bit = QOSA_UART_PARITY_NONE;
    dcb_config.stop_bit = QOSA_UART_STOP_1;

    qosa_uart_ioctl(QUEC_TEST_UART_PORT, QOSA_UART_IOCTL_SET_DCB_CFG, (void *)&dcb_config);
    
        /* 鎵撳紑UART绔彛 */
    qosa_uart_open(QUEC_TEST_UART_PORT);

    while (1)
    {
        switch (g_uart_test_case)
        {
            /* 娴嬭瘯UART鍙戦€佸姛鑳?*/
            case QOSA_UART_DEMO_OUTPUT: {
                qosa_task_sleep_sec(1);
                qosa_uart_write(QUEC_TEST_UART_PORT, (unsigned char *)"hello Quectel\r\n", 15);
            }
            break;
            /* 娴嬭瘯UART鎺ユ敹鍔熻兘锛堥€氳繃鍥炶皟澶勭悊锛?*/
            case QOSA_UART_DEMO_READ_1: {
                qosa_task_sleep_sec(1);
                /* Received data in uart callback */
            }
            break;
            /* 娴嬭瘯UART鎺ユ敹鍔熻兘锛堜富鍔ㄨ鍙栵級 */
            case QOSA_UART_DEMO_READ_2: {
                qosa_task_sleep_sec(5);
                qosa_uart_read(QUEC_TEST_UART_PORT, (unsigned char *)&g_uart_data, 1024);

                QLOGI("recv uart data %s", g_uart_data);
                ret = qosa_uart_write(QUEC_TEST_UART_PORT, (unsigned char *)&g_uart_data, 1024);
                QLOGI("qosa_uart_write ret = %d", ret);
             }
            break;
            /* 娴嬭瘯涓嶅悓娉㈢壒鐜囦笅鐨刄ART閫氫俊 */
            case QOSA_UART_DEMO_BAUDRATE: {
                const qosa_uint32_t baudRateList[] = {0, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};

                int i;
                for (i = 0; i < sizeof(baudRateList) / sizeof(baudRateList[0]); i++)
                {
                    qosa_uart_ioctl(QUEC_TEST_UART_PORT, QOSA_UART_IOCTL_CHANGE_BAUDRATE, (void *)&baudRateList[i]);
                    qosa_task_sleep_sec(1);
                    qosa_uart_write(QUEC_TEST_UART_PORT, (unsigned char *)"Baudrate TEST\r\n", 15);
                    qosa_task_sleep_sec(1);
                }
            }
            break;
            /* 娴嬭瘯UART妯″紡鍒囨崲鍔熻兘锛圲art妯″紡涓嶢T鍛戒护妯″紡锛?*/
            case QOSA_UART_DEMO_CHANGE_CCIO_MODE: {
                qosa_uart_mode_e ccio_mode;
                int              i;
                ccio_mode = QOSA_UART_MODE_NORMAL;
                qosa_uart_ioctl(QUEC_TEST_UART_PORT, QOSA_UART_IOCTL_SET_CCIO_MODE, (void *)&ccio_mode);
                qosa_uart_write(QUEC_TEST_UART_PORT, (unsigned char *)"Enter Uart Mode\r\n", 17);
                for (i = 0; i < 20; i++)
                {
                    qosa_uart_write(QUEC_TEST_UART_PORT, (unsigned char *)"Waiting...\r\n", 12);
                    qosa_task_sleep_sec(1);
                }
                
                ccio_mode = QOSA_UART_MODE_AT;
                qosa_uart_write(QUEC_TEST_UART_PORT, (unsigned char *)"Enter AT Mode\r\n", 15);
                qosa_task_sleep_sec(1); /* 绛夊緟鍙戦€佺粨鏉?*/
                qosa_uart_ioctl(QUEC_TEST_UART_PORT, QOSA_UART_IOCTL_SET_CCIO_MODE, (void *)&ccio_mode);
                qosa_task_sleep_sec(20);
            }
            break;
            default:
                break;
        }
    }
}

/*===========================================================================
 *  Public API Functions
 ===========================================================================*/

void quec_demo_uart_case_switch(qosa_uart_demo_case_e caseNo)
{
    g_uart_test_case = caseNo;
}

void quec_uart_demo_init(void)
{
    QLOGI("enter Quectel UART DEMO !!!");
    if (g_quec_uart_demo_task == QOSA_NULL)
    {
        qosa_task_create(
            &g_quec_uart_demo_task,
            CONFIG_QUECOS_UART_DEMO_TASK_STACK_SIZE,
            QUEC_UART_DEMO_TASK_PRIO,
            "uart_demo",
            quec_uart_demo_process,
            QOSA_NULL,
            1
        );
    }
}

UNIRTOS_APP_EXPORT(200, "uart_demo", quec_uart_demo_init);
