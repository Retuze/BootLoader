#ifndef _THREAD_H_
#define _THREAD_H_

#include "partfrom.h"
#include "rtdebug.h"
#include "rtdef.h"
#include "rthw.h"
#include "rtthread.h"

#ifdef __cplusplus
extern "C" {
#endif

void rt_hw_board_init(void);

#ifdef __cplusplus
}

class Thread {
private:
    // rt_thread_t tid;

public:
    static rt_err_t sleep(rt_tick_t tick)
    {
        return rt_thread_delay(tick);
    }
    // begin方法动态创建线程需要在初始化堆内存之后，静态方法可以提前创建线程
    static int setup(const char* name, void (*entry)(void*), void* parameter = RT_NULL, rt_uint32_t stack_size = 1024, rt_uint8_t priority = RT_THREAD_PRIORITY_MAX / 3, rt_uint32_t tick = 10)
    {
        rt_hw_interrupt_disable();

        /* board level initialization
         * NOTE: please initialize heap inside board initialization.
         */
        rt_hw_board_init();

        /* show RT-Thread version */
        rt_show_version();

        /* timer system initialization */
        rt_system_timer_init();

        /* scheduler system initialization */
        rt_system_scheduler_init();

        /* create init_thread */
        auto t = rt_thread_create(name, entry, parameter, stack_size, priority, tick);
        rt_thread_startup(t);

        /* timer thread initialization */
        rt_system_timer_thread_init();

        /* idle thread initialization */
        rt_thread_idle_init();

        /* start scheduler */
        rt_system_scheduler_start();

        /* never reach here */
        return 0;
    }
    static rt_err_t begin(const char* name, void (*entry)(void*), void* parameter = RT_NULL, rt_uint32_t stack_size = 1024, rt_uint8_t priority = RT_THREAD_PRIORITY_MAX / 3, rt_uint32_t tick = 10)
    {
        auto tid = rt_thread_create(name, entry, parameter, stack_size, priority, tick);
        RT_ASSERT(tid);
        return rt_thread_startup(tid);
    }
    static void end()
    {
        rt_thread_delete(rt_thread_self());
    }
};
class Timer {
private:
    rt_timer_t timer_t;

public:
    void begin(const char* name, rt_tick_t time, void (*timeout)(void*), void* parameter = RT_NULL, rt_uint8_t flag = RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER)
    {
        /* 创建一个软件定时器 */
        timer_t = rt_timer_create(name, timeout, parameter, time, flag);
        if (timer_t != RT_NULL) {
            rt_timer_start(timer_t);
        }
    }
};
#endif

#endif // _THREAD_H_