#include "qosa_rtc.h"
#include "qosa_def.h"

int main(void)
{
    qosa_time_t utc_time;
    qosa_rtc_time_t rtc_tm;
    
       // 1. Initialize RTC time service
    qosa_time_init();
    
       // 2. Get and print the current UTC time
    qosa_rtc_get_time(&utc_time);
    qosa_rtc_gmtime_r(&utc_time, &rtc_tm);
       printf("current UTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_tm.tm_year + 1900,
           rtc_tm.tm_mon + 1,
           rtc_tm.tm_mday,
           rtc_tm.tm_hour,
           rtc_tm.tm_min,
           rtc_tm.tm_sec);
    
       // 3. Get and print the local time with timezone
    qosa_time_t local_time;
    qosa_rtc_get_localtime(&local_time);
    qosa_rtc_gmtime_r(&local_time, &rtc_tm);
       printf("current local time: %04d-%02d-%02d %02d:%02d:%02d, timezone: %d\n",
           rtc_tm.tm_year + 1900,
           rtc_tm.tm_mon + 1,
           rtc_tm.tm_mday,
           rtc_tm.tm_hour,
           rtc_tm.tm_min,
           rtc_tm.tm_sec,
           qosa_rtc_get_timezone());
    
    return 0;
}