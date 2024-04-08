#ifndef _LED_H_
#define _LED_H_

#include "partfrom.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

class LEDC {
private:
    enum LED_STATE {
        OFF,
        ON,
        PWM,
    } state;
    uint8_t duty;
    uint32_t h_time = 0;
    uint32_t l_time = 1000;
    uint32_t h_tick = 0;
    uint32_t l_tick = 0;
    void (*setLedState)(uint8_t);

public:
    LEDC(void (*setLedState)(uint8_t))
    {
        this->setLedState = setLedState;
        this->setLedState(0);
        state = OFF;
    }
    void run()
    {
        if (h_time == 0 || l_time == 0) {
            return;
        }
        if (h_time >= h_tick * 20) {
            setLedState(100);
            h_tick++;
        } else if (l_time >= l_tick * 20) {
            setLedState(0);
            l_tick++;
        } else if (h_time < h_tick * 20 && l_time < l_tick * 20) {
            h_tick = 0;
            l_tick = 0;
        }
    }
    void pwm(uint8_t val)
    {
        state = PWM;
        duty = val;
        setLedState(val);
        h_time = 1000;
        l_time = 0;
    }
    uint8_t pwm()
    {
        return duty;
    }
    void on()
    {
        state = ON;
        setLedState(100);
        h_time = 1000;
        l_time = 0;
    }
    void off()
    {
        state = OFF;
        setLedState(0);
        h_time = 0;
        l_time = 1000;
    }
    void toggle()
    {
        state == OFF ? on() : off();
    }
    void blink(uint32_t period)
    {
        h_time = period / 2;
        l_time = period / 2;
    }
    void blink(uint32_t h_time, uint32_t l_time)
    {
        this->h_time = h_time;
        this->l_time = l_time;
    }
};

#endif

#endif // _LED_H_