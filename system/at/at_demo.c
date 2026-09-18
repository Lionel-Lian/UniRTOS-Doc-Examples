#include "qosa_sys.h"
#include "qosa_def.h"
#include "qosa_log.h"
#include "qosa_at_cmd.h"
#include "qosa_at_param.h"
#include "unirtos_app_init_registry.h"

/*===========================================================================
 * 瀹忓畾涔変笌澹版槑
 ===========================================================================*/
#define QOS_LOG_TAG "EXAMPLE_AT"

/* 澹版槑鎵ц鍑芥暟 */
void unir_exec_example_qexamplehello_cmd(qosa_at_cmd_t *cmd);

/*===========================================================================
 * AT 鍛戒护鎻忚堪琛? ===========================================================================*/
static const qosa_at_desc_t unir_examples_at_desc[] = {
    {"+QEXAMPLEHELLO", unir_exec_example_qexamplehello_cmd, 0},
};

/*===========================================================================
 * AT 鍛戒护澶勭悊鍑芥暟
 ===========================================================================*/
/**
 * @brief 澶勭悊 AT+QEXAMPLEHELLO 鍛戒护
 * 鏀寔绫诲瀷:
 * AT+QEXAMPLEHELLO=? (TEST)
 * AT+QEXAMPLEHELLO?  (READ)
 * AT+QEXAMPLEHELLO   (EXE)
 * AT+QEXAMPLEHELLO="string",num (SET)
 */
void unir_exec_example_qexamplehello_cmd(qosa_at_cmd_t *cmd)
{
    char          resp[128] = {0};
    char         *test_str = QOSA_NULL;
    qosa_bool_t   paramok = QOSA_TRUE;
    qosa_uint32_t num = 0;

    switch (cmd->type)
    {
        case QOSA_AT_CMD_SET: { // 澶勭悊璁剧疆鍛戒护
            // 鑾峰彇绗竴涓瓧绗︿覆鍙傛暟
            test_str = (char *)qosa_at_param_string(cmd->params[0], &paramok);
            if (!paramok) {
                qosa_at_resp_cme_error(cmd->dev_port, QOSA_ERR_AT_CME_PARAM_INVALID);
                return;
            }

            // 濡傛灉鏈夌浜屼釜鍙傛暟锛岃幏鍙栨暣鏁?            if (cmd->param_count == 2) {
                num = qosa_at_param_uint(cmd->params[1], &paramok);
                if (!paramok) {
                    qosa_at_resp_cme_error(cmd->dev_port, QOSA_ERR_AT_CME_PARAM_INVALID);
                    return;
                }
            }

            // 杩斿洖璁剧疆鐨勭粨鏋?
            qosa_snprintf(resp, sizeof(resp), "+QEXAMPLEHELLO: \"%s\",%d", test_str, num);
            qosa_at_resp_cmd(cmd->dev_port, QOSA_ATCI_RESULT_CODE_OK, QOSA_CMD_RC_OK, resp, 1);
        }
        break;

        case QOSA_AT_CMD_TEST: { // 澶勭悊娴嬭瘯鍛戒护 AT+QEXAMPLEHELLO=? 
            qosa_snprintf(resp, sizeof(resp), "%s", "+QEXAMPLEHELLO: \"test_str\",<num>");
            qosa_at_resp_cmd(cmd->dev_port, QOSA_ATCI_RESULT_CODE_OK, QOSA_CMD_RC_OK, resp, 1);
        }
        break;

        case QOSA_AT_CMD_READ: { // 澶勭悊鏌ヨ鍛戒护 AT+QEXAMPLEHELLO?
            qosa_snprintf(resp, sizeof(resp), "%s", "+QEXAMPLEHELLO: Hello World!");
            qosa_at_resp_cmd(cmd->dev_port, QOSA_ATCI_RESULT_CODE_OK, QOSA_CMD_RC_OK, resp, 1);
        }
        break;

        case QOSA_AT_CMD_EXE: { // 澶勭悊鎵ц鍛戒护 AT+QEXAMPLEHELLO 
            qosa_at_resp_cmd(cmd->dev_port, QOSA_ATCI_RESULT_CODE_OK, QOSA_CMD_RC_OK, QOSA_NULL, 1);
        }
        break;

        default:
            qosa_at_resp_cme_error(cmd->dev_port, QOSA_ERR_AT_CME_OPERATION_NOT_SUPPORTED);
        break;
    }
}

/*===========================================================================
 * 鍒濆鍖栧嚱鏁? ===========================================================================*/
void qexample_hello_init(void)
{
    // 鍚戠郴缁熸敞鍐岃嚜瀹氫箟 AT 鍛戒护琛?    qosa_at_parser_add_cust_at(unir_examples_at_desc, QOSA_ARRAY_SIZE(unir_examples_at_desc));
    QLOGV("QEXAMPLEHELLO demo initialized.");
}

UNIRTOS_APP_EXPORT(200, "at_demo", qexample_hello_init);
