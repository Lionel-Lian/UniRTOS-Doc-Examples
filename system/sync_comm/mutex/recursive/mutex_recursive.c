#include "qosa_sys.h"
#include "qosa_log.h"
#include "unirtos_app_init_registry.h"

static qosa_mutex_t recursive_mutex = NULL;

// 鍐呴儴鍑芥暟锛堥渶瑕侀攣淇濇姢锛?static void internal_function(void)
{
    int ret;
    
    // 宓屽鑾峰彇閿侊紙閫掑綊閿佸厑璁革級
    ret = qosa_mutex_lock(recursive_mutex, QOSA_WAIT_FOREVER);
    if (ret == QOSA_ERROR_OK) {
        QOSA_LOG_D("Recursive", "Lock acquired in internal_function");
        // 鎵ц鎿嶄綔...
        qosa_mutex_unlock(recursive_mutex);
    }
}

// 澶栭儴鍑芥暟锛堜篃闇€瑕侀攣淇濇姢锛?void external_function(void)
{
    int ret;
    
    // 绗竴娆¤幏鍙栭攣
    ret = qosa_mutex_lock(recursive_mutex, QOSA_WAIT_FOREVER);
    if (ret == QOSA_ERROR_OK) {
        QOSA_LOG_D("Recursive", "Lock acquired in external_function");
        
        // 璋冪敤鍐呴儴鍑芥暟锛堝祵濂楄幏鍙栭攣锛?        internal_function();
        
        // 閲婃斁閿侊紙闇€瑕佷笌鑾峰彇娆℃暟鍖归厤锛?        qosa_mutex_unlock(recursive_mutex);
    }
}

// 閫掑綊閿佺ず渚嬪垵濮嬪寲
int recursive_mutex_example(void)
{
    int ret;
    
    ret = qosa_mutex_create(&recursive_mutex);
    if (ret == QOSA_ERROR_OK) {
        // 鍒涘缓浠诲姟鎵ц閫掑綊閿佹祴璇?        qosa_task_create("RecursiveTest", external_function, NULL,
                        4096, QOSA_TASK_PRIORITY_NORMAL);
    }
    
    return ret;
}

static void __unirtos_export_mutex_recursive(void)
{
    (void)recursive_mutex_example();
}

UNIRTOS_APP_EXPORT(200, "mutex_recursive", __unirtos_export_mutex_recursive);
