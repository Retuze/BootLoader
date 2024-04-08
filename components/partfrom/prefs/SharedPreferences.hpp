#ifndef _SHAREDPREFERENCES_H_
#define _SHAREDPREFERENCES_H_

#include "prefs.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
class SharedPreferences {
protected:
    prefs_t prefs;

public:
    SharedPreferences(void (*eraseOnePage)(uint32_t num), void (*write)(uint32_t num, void* data, int len), void (*read)(uint32_t num, void* data, int len), uint32_t start_addr, uint32_t page_size, uint32_t page_cnt)
    {
        prefs.start_addr = start_addr;
        prefs.eraseOnePage = eraseOnePage;
        prefs.write = write;
        prefs.read = read;
        prefs.page_size = page_size;
        prefs.page_cnt = page_cnt;
        pf_begin(&prefs);
    }
    uint8_t putChar(const char* key, uint8_t value) { return pf_putChar(&this->prefs, key, value); }
    uint8_t getChar(const char* key) { return pf_getChar(&this->prefs, key); }
    uint16_t putShort(const char* key, uint16_t value) { return pf_putChar(&this->prefs, key, value); }
    uint16_t getShort(const char* key) { return pf_getChar(&this->prefs, key); }
    uint8_t isKey(const char* key) { return pf_isKey(&this->prefs, key); }
    uint32_t available() { return pf_available(&this->prefs); }
    uint32_t capacity() { return pf_capacity(&this->prefs); }
    uint8_t init() { return pf_init(&this->prefs); }
};
class SP_STM32F103C8T6 : public SharedPreferences {
    public:
    SP_STM32F103C8T6(
        void (*eraseOnePage)(uint32_t num) = [](uint32_t num) {
            HAL_FLASH_Unlock(); // 解锁
            SET_BIT(FLASH->CR, FLASH_CR_PER); // 置位扇区擦除控制位
            WRITE_REG(FLASH->AR, num); // 对应扇区起始地址
            SET_BIT(FLASH->CR, FLASH_CR_STRT); // 开始扇区清除
            while ((FLASH->SR & (FLASH_SR_BSY | FLASH_SR_EOP)) != 0x20) // 等待擦除完毕
                ;
            FLASH->SR |= FLASH_SR_EOP; // 清除标志位
            HAL_FLASH_Lock(); // 重新上锁，擦除完成
        }, // erase
        void (*write)(uint32_t num, void* data, int len) = [](uint32_t num, void* data, int len) {
            HAL_FLASH_Unlock();
            for (int i = 0; i < len / 2; i++) {
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, num + i * 2, *((uint16_t*)(data) + i));
            }
            HAL_FLASH_Lock(); }, // write
        void (*read)(uint32_t num, void* data, int len) = [](uint32_t num, void* data, int len) {
            for (int i = 0; i < len / 2; i++) {
                *((uint16_t*)data + i) = *((uint16_t*)num + i);
            } }, uint32_t start_addr = 0x800C000 - 1 * 1024, uint32_t page_size = 1024, uint32_t page_cnt = 1)
        : SharedPreferences(eraseOnePage, write, read, start_addr, page_size, page_cnt) // 初始化基类
    {
        // 这里可以添加SP_STM32F103C8T6特有的初始化代码，如果有的话
    }
};
#endif

#endif // _SHAREDPREFERENCES_H_