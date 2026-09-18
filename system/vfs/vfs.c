#include "qosa_log.h"
#include "qosa_def.h"
#include "qosa_sys.h"
#include "qosa_virtual_file.h"
#include "unirtos_app_init_registry.h"

#define QOS_LOG_TAG                   LOG_TAG

#define UNIR_VFS_DEMO_TASK_STACK_SIZE 4096

/**
 * @brief 閬嶅巻鎸囧畾鐩綍涓嬫墍鏈夋枃浠跺拰瀛愮洰褰曪紝杈撳嚭璇︾粏淇℃伅
 */
static void unir_vfs_demo_dir_list(char *file_path)
{
    QOSA_VFS_DIR             *dir = QOSA_NULL;
    struct qosa_vfs_dirent_t *entry = QOSA_NULL;
    struct qosa_vfs_stat_t    st = {0};
    qosa_int64_t              size = 0;
    qosa_int32_t              ret = 0;
    char                      child[QOSA_VFS_PATH_MAX] = {0};

    /* 鎵撳紑鐩綍 */
    dir = qosa_vfs_opendir(file_path);
    if (dir == QOSA_NULL)
    {
        QLOGE("dir open err=%d", qosa_get_errno());
        return;
    }

    /* 寰幆璇诲彇鐩綍鏉＄洰 */
    while ((entry = qosa_vfs_readdir(dir)) != QOSA_NULL)
    {
        QLOGV("%s\n", entry->d_name);

        if (entry->d_type == QOSA_VFS_DT_REG)
        {
            /* 鏅€氭枃浠讹細鑾峰彇鏂囦欢鐘舵€佷俊鎭?*/
            char path[1024] = {0};
            qosa_snprintf(path, sizeof(path), "./%s", entry->d_name);

            if (qosa_vfs_stat(path, &st) == -1)
            {
                QLOGV("stat");
            }
            else
            {
                QLOGV("File name: %s", entry->d_name);
                QLOGV("File size: %ld bytes", (long)st.st_size);
                QLOGV("File mode: %o", (int)st.st_mode);
            }
        }
        else if (entry->d_type == QOSA_VFS_DT_DIR)
        {
            /* 瀛愮洰褰曪細鑾峰彇鐩綍鎬诲ぇ灏?*/
            qosa_memset(child, 0, sizeof(child));
            qosa_snprintf(child, sizeof(child), "%s/%s", file_path, entry->d_name);
            size = qosa_vfs_dir_total_size(child);
            QLOGV("Dir name: [%s],[%s]", entry->d_name, child);
            qosa_int32_t high = ((size >> 32) & 0xFFFFFFFF);
            qosa_int32_t low = (size & 0xFFFFFFFF);
            QLOGV("Dir size: %d,%d bytes", high, low);
        }
    }
    ret = qosa_vfs_closedir(dir);
    QLOGD("close ret=%d", ret);
}

/**
 * @brief 鐩綍鎿嶄綔娴嬭瘯锛氬垱寤虹洰褰曞拰鏂囦欢銆侀亶鍘嗐€佸垹闄? */
static void unir_vfs_demo_dir_test(void)
{
    QOSA_VFS_DIR *dir = QOSA_NULL;
    qosa_int32_t  ret = 0;

    QLOGV("test dir");

    /* 灏濊瘯鎵撳紑鐩綍锛屼笉瀛樺湪鍒欏垱寤?*/
    dir = qosa_vfs_opendir("./testdir");
    if (dir == QOSA_NULL)
    {
        QLOGE("dir not exist");
        ret = qosa_vfs_mkdir("./testdir", 0);
        if (ret != 0)
        {
            QLOGE("mkdir err=%d", qosa_get_errno());
            return;
        }
    }

    /* 鍒涘缓娴嬭瘯鏂囦欢鍜屽瓙鐩綍 */
    ret = qosa_vfs_creat("./testdir/vfs_test1.txt", 0);
    if (ret < 0)
    {
        QLOGE("creat file err=%d", qosa_get_errno());
    }
    ret = qosa_vfs_creat("./testdir/vfs_test2.txt", 0);
    if (ret < 0)
    {
        QLOGE("creat file2 err=%d", qosa_get_errno());
    }
    ret = qosa_vfs_mkdir("./testdir/subdir", 0);
    if (ret != 0)
    {
        QLOGE("subdir mkdir err=%d", qosa_get_errno());
    }

    /* 閬嶅巻鐩綍鍐呭 */
    unir_vfs_demo_dir_list("./testdir");

    /* 鍏抽棴鐩綍 */
    dir = qosa_vfs_opendir("./testdir");
    if (dir == QOSA_NULL)
    {
        QLOGE("dir open err=%d", qosa_get_errno());
        return;
    }
    ret = qosa_vfs_closedir(dir);
    QLOGD("close dir ret=%d", ret);

    /* 灏濊瘯鍒犻櫎闈炵┖鐩綍锛堥鏈熷け璐ワ級 */
    ret = qosa_vfs_rmdir("./testdir");
    QLOGD("remove dir ret=%d", ret);

    /* 閫掑綊鍒犻櫎鐩綍鍙婃墍鏈夊唴瀹?*/
    ret = qosa_vfs_rmdir_recursive("./testdir");
    QLOGD("remove dir ret=%d", ret);
}

/**
 * @brief 鏂囦欢鎿嶄綔娴嬭瘯锛氬垱寤恒€佸啓鍏ャ€佽鍙栥€佺姸鎬佹煡璇€佸垹闄? */
static void unir_vfs_demo_file_test(void)
{
    int                    fd = 0;
    int                    ret = 0;
    char                   data[10 + 1] = {0};
    struct qosa_vfs_stat_t stat = {0};

    QLOGV("test file");

    /* 鎵撳紑鎴栧垱寤烘枃浠讹紝璇诲啓妯″紡 */
    fd = qosa_vfs_open("./vfs_test.txt", QOSA_VFS_O_CREAT | QOSA_VFS_O_RDWR);
    if (fd < 0)
    {
        QLOGE("open dir error!!");
        return;
    }

    /* 鍐欏叆娴嬭瘯鏁版嵁 */
    qosa_snprintf(data, 10, "%s", "1234567890");
    ret = qosa_vfs_write(fd, data, 10);
    QLOGD("write ret=%d", ret);

    /* 鑾峰彇鏂囦欢鐘舵€佷俊鎭?*/
    ret = qosa_vfs_fstat(fd, &stat);
    if (ret == 0)
    {
        QLOGD("size=%d", stat.st_size);
    }

    /* 灏嗘枃浠舵寚閽堢Щ鍥炶捣濮嬩綅缃?*/
    qosa_vfs_lseek(fd, 0, QOSA_VFS_SEEK_SET);

    /* 璇诲彇鏂囦欢鏁版嵁 */
    qosa_memset(data, 0, sizeof(data));
    ret = qosa_vfs_read(fd, data, 10);
    QLOGD("read ret=%d,data=[%s]", ret, data);

    /* 鍏抽棴骞跺垹闄ゆ祴璇曟枃浠?*/
    qosa_vfs_close(fd);
    qosa_vfs_unlink("./vfs_test.txt");
}

static void unir_vfs_task_handler(void *argv)
{
    QOSA_UNUSED(argv);
    qosa_task_sleep_sec(10);
    unir_vfs_demo_file_test();
    unir_vfs_demo_dir_test();
}

void unir_vfs_demo_init(void)
{
    int         err = 0;
    qosa_task_t vfs_task = QOSA_NULL;

    err = qosa_task_create(&vfs_task, UNIR_VFS_DEMO_TASK_STACK_SIZE, QOSA_PRIORITY_NORMAL,
                           "vfs_demo", unir_vfs_task_handler, QOSA_NULL);
    if (err != QOSA_OK)
    {
        QLOGE("task create error");
        return;
    }
}

UNIRTOS_APP_EXPORT(200, "vfs", unir_vfs_demo_init);
