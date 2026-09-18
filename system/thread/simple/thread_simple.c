#include "qosa_sys.h"
#include <stdio.h>
#include "unirtos_app_init_registry.h"

void task_A(void *argv)
{
    int count = 0;
    while (1) {
        printf("[A] counter: %d\n", count++);
        qosa_task_sleep_ms(1000);
    }
}

void task_B(void *argv)
{
    while (1) {
        printf("[B] system tick: %u\n", qosa_get_system_tick());
        qosa_task_sleep_ms(2000);
    }
}

static int thread_simple_main(void)
{
    qosa_task_t handleA, handleB;

    qosa_task_create(&handleA, 1024, 10, "taskA", task_A, NULL);
    qosa_task_create(&handleB, 1024, 10, "taskB", task_B, NULL);

    while (1) qosa_task_sleep_ms(1000); // 主任务让出CPU
}

static void __unirtos_export_thread_simple(void)
{
    (void)thread_simple_main();
}

UNIRTOS_APP_EXPORT(200, "thread_simple", __unirtos_export_thread_simple);