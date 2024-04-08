#include "app.h"
#include "HardwareSerial.h"
#include "SharedPreferences.hpp"
#include "Thread.hpp"
#include "digital_io.h"
#include "handler.h"
#include "key.h"
#include "led.h"
#include "linenano.h"
#include "log.hpp"
#include "prefs.h"
#include "printk.h"
#include "rtthread.h"
#include "shell.h"
#include "stm32f1xx_hal_gpio.h"
#include "sys/_intsup.h"
#include "usart.h"
#include <stdint.h>
#include "U8g2lib.h"

LOG_LOG lg;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

LEDC led([](uint8_t val) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (GPIO_PinState)(val));
});

KEY key([]() -> uint8_t {
    return (uint8_t)HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15);
});

void app_entry(void* arg)
{
    set_output([](const char* buf, unsigned short len) {
        HAL_UART_Transmit(&huart1, (const uint8_t*)buf, (uint16_t)len, 100);
    });
    Serial.begin();
    Thread::begin("ui", [](void* arg) {
        u8g2.begin();
        u8g2.enableUTF8Print();
        u8g2.setFont(u8g2_font_unifont_t_chinese2);
        while (1) {
            u8g2.clearBuffer();
            u8g2.drawUTF8(32, 32, "你好世界");
            u8g2.sendBuffer();
            Thread::sleep(10);
        }
    });
    // 创建一个软件定时器
    Timer timer;
    timer.begin("key", 10, [](void* arg) {
        key.run();
    });
    key.setOnClickEvent([]() {
        printk("clicked!!!\n");
    });
    key.setOnLongClickEvent([]() -> bool {
        printk("long clicked!!!\n");
        return true;
    });
    key.setOnLongClickEventRepeat([]() {
        printk("long rept!!!\n");
    });
    key.setPressEvent([]() {
        printk("Press!!!\n");
    });
    key.setReleaseEvent([]() {
        printk("Release!!!\n");
    });
    while (1) {
        led.toggle();
        Thread::sleep(100);
    }
}

void app_lunch()
{
    Thread::setup("app_entry", app_entry, RT_NULL, 4096);
}