#ifndef _PREFS_H_
#define _PREFS_H_

#include "main.h"
#include "partfrom.h"
#include "printk.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct prefs_t {
    int page_cnt;
    int page_size;
    uint32_t start_addr;
    void (*eraseOnePage)(uint32_t flash_addr);
    void (*write)(uint32_t flash_addr, void* data, int len);
    void (*read)(uint32_t flash_addr, void* data, int len);
    uint32_t read_addr;
    uint32_t write_addr;
} prefs_t;

// 数据保存在flash中的样子
typedef struct prefs_data {
    uint32_t hash; // 保存key对应的hash值
    uint32_t current_addr;
    uint32_t next_self_addr; // 在数据过时保存下一个数据的地址,数据有效的时候则为）0xFFFF
    uint32_t next_addr; // 保存的下一个数据的起始地址
} prefs_data;

// 状态码
enum {
    pf_OK,
    pf_NOT_ENOUGH_SPACE,
    pf_NOT_DIV_TWO,
    pf_NOT_FOUND,
    pf_UNKOW_ERROR,
};

uint8_t pf_putChar(prefs_t* prefs, const char* key, uint8_t value);
uint8_t pf_putShort(prefs_t* prefs, const char* key, uint16_t value);
uint8_t pf_putInt(prefs_t* prefs, const char* key, uint32_t value);
uint8_t pf_putFloat(prefs_t* prefs, const char* key, float value);
uint8_t pf_isKey(prefs_t* prefs, const char* key);
uint8_t pf_getChar(prefs_t* prefs, const char* key);
uint16_t pf_getShort(prefs_t* prefs, const char* key);
uint32_t pf_getInt(prefs_t* prefs, const char* key);
float pf_getFloat(prefs_t* prefs, const char* key);
uint8_t pf_removeItem(prefs_t* prefs, const char* key);
size_t pf_getFreeByte(prefs_t* prefs);
size_t pf_getAllByte(prefs_t* prefs);
size_t pf_getUsedByte(prefs_t* prefs);
size_t pf_getItemCount(prefs_t* prefs);
size_t pf_getWriteCount(prefs_t* prefs);
void pf_eraseAll(prefs_t* prefs);
void prefs(prefs_t* p,
    void (*eraseOnePage)(uint32_t flash_addr),
    void (*write)(uint32_t flash_addr, void* data, int len),
    void (*read)(uint32_t flash_addr, void* data, int len),
    uint32_t start_addr, uint32_t page_size, uint32_t page_cnt);
uint8_t pf_begin(prefs_t* prefs);
uint32_t pf_available(prefs_t* prefs);
uint32_t pf_capacity(prefs_t* prefs);
uint8_t pf_init(prefs_t* prefs);

#ifdef __cplusplus
}
#endif

#endif // _PREFS_H_