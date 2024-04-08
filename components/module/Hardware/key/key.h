#ifndef _KEY_H_
#define _KEY_H_

#include "fifo.h"
#include "sys/_stdint.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

class KEY {
private:
    enum {
        DOWN,
        UP,
    };
    /// @brief 按下返回值为0，抬起返回值为1
    uint8_t (*get_key)();
    void (*short_event)();
    bool (*long_event)();
    void (*long_event_repeat)();
    void (*press_event)();
    void (*release_event)();
    // 默认运行周期20ms
    uint8_t time_tick = 10;
    uint8_t state = UP;
    uint16_t long_tick = 0;
    // 该变量用来指示长按完触发短按的标志位
    uint8_t long_click;

public:
    FIFO event;
    enum {
        PRESS,
        RELEASE,
        CLICK,
        LONG_CLICK,
        LONG_CLICK_REPT,
    };
    KEY()
    {
    }
    KEY(uint8_t (*get_key)())
    {
        this->get_key = get_key;
    }
    void setTimeTick(uint8_t val)
    {
        time_tick = val;
    }
    void setPressEvent(void (*fun)())
    {
        this->press_event = fun;
    }
    void setReleaseEvent(void (*fun)())
    {
        this->release_event = fun;
    }
    void setOnClickEvent(void (*short_event)())
    {
        this->short_event = short_event;
    }
    // 如果长按事件返回值为true则触发短按事件
    void setOnLongClickEvent(bool (*long_event)())
    {
        this->long_event = long_event;
    }
    void setOnLongClickEventRepeat(void (*long_event_repeat)())
    {
        this->long_event_repeat = long_event_repeat;
    }
    void run()
    {
        // 首次检测到按下
        if (get_key() == DOWN && state != DOWN) {
            // 按键被按下
            state = DOWN;
            long_tick++;
            event.putChar(PRESS);
            if (press_event) {
                press_event();
            }
        }
        // 检测到一直按下
        else if (get_key() == DOWN && long_tick != 0) {
            long_tick++;
            if ((long_tick >= 500 / time_tick)) {
                if ((long_tick - 1) < 500 / time_tick) {
                    if (long_event != nullptr) {
                        event.putChar(LONG_CLICK);
                        if (long_event()) {
                            long_click = 1;
                        }
                    }
                } else if (long_event_repeat != nullptr) {
                    if ((long_tick - 500 / time_tick) % 10 == 0) {
                        event.putChar(LONG_CLICK_REPT);
                        long_event_repeat();
                    }
                }
            }
        }
        // 首次检测到从按下变为抬起
        else if (get_key() == UP && state != UP) {
            event.putChar(RELEASE);
            if (release_event) {
                release_event();
            }
            // 按下时间不超过500ms，正常触发短按事件
            if ((long_tick <= 500 / time_tick) || long_click == 1) {
                long_click = 0;
                event.putChar(CLICK);
                if (short_event != nullptr) {
                    short_event();
                }
            }
            long_tick = 0;
            state = UP;
        }
    }
};

#endif

#endif // _KEY_H_