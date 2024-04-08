#include "prefs.h"
#include <stdint.h>

uint8_t pf_init(prefs_t* prefs)
{
    for (int i = 0; i < prefs->page_cnt; i++) {
        prefs->eraseOnePage(prefs->start_addr + i * prefs->page_size);
    }
    return 0;
}

// BKDR Hash Function
static uint32_t pf_BKDRHash(const char* str)
{
    uint32_t seed = 131; // 31 131 1313 13131 131313 etc..
    uint32_t hash = 0;
    while (*str) {
        hash = hash * seed + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}
static uint8_t pf_initData(prefs_data* data, uint32_t start_addr)
{
    data->hash = *((uint32_t*)start_addr);
    data->current_addr = start_addr;
    data->next_addr = *((uint32_t*)start_addr + 1);
    data->next_self_addr = *((uint32_t*)start_addr + 2);
    return 0;
}
static uint8_t pf_nextData(prefs_data* data)
{
    return pf_initData(data, data->next_addr);
}
static uint8_t pf_nextSelfData(prefs_data* data)
{
    return pf_initData(data, data->next_self_addr);
}
static uint8_t pf_printData(prefs_data* data)
{
    printk("hash :%X\r\n", data->hash);
    printk("current addr :%X\r\n", data->current_addr);
    printk("next addr :%X\r\n", data->next_addr);
    printk("next self addr :%X\r\n", data->next_self_addr);
    return 0;
}
uint8_t pf_begin(prefs_t* prefs)
{
    prefs_data current;
    pf_initData(&current, prefs->start_addr);
    pf_printData(&current);
    while (current.hash != 0xffffffff) {
        pf_nextData(&current);
    }
    prefs->write_addr = current.current_addr;
    return 0;
}
uint32_t pf_capacity(prefs_t* prefs)
{
    return (prefs->page_size * prefs->page_cnt);
}
uint32_t pf_available(prefs_t* prefs)
{
    return (prefs->page_size * prefs->page_cnt) - (prefs->write_addr - prefs->start_addr);
}
static uint8_t pf_findNext(prefs_t* prefs, prefs_data* current, const char* key)
{
    uint32_t hash = pf_BKDRHash(key);
    pf_initData(current, prefs->start_addr);
    pf_printData(current);
    while (current->hash != 0xffffffff) {
        // hash匹配，且数据过期,直接跳到下一个数据
        if (current->hash == hash && current->next_self_addr != 0xffffffff) {
            pf_nextSelfData(current);
        }
        // hash匹配，且数据有效
        else if (current->hash == hash && current->next_self_addr == 0xffffffff) {
            return pf_OK;
        }
        // hash不匹配,执行常规更新
        else {
            pf_nextData(current);
        }
    }
    if (current->current_addr > prefs->start_addr + prefs->page_size * prefs->page_cnt) {
        return pf_NOT_ENOUGH_SPACE;
    }
    return pf_NOT_FOUND;
}
static uint8_t pf_putItem(prefs_t* prefs, const char* key, void* data, uint32_t size)
{
    if (size % 2 != 0) {
        printk("写入数据必须为2字节倍数\n");
        return pf_NOT_DIV_TWO;
    }
    prefs_data current;
    uint8_t res = pf_findNext(prefs, &current, key);
    if (res == pf_OK) {
        // 把当前数据改为无效
        // 这里的处理是错误的，应该是找到空的地址
        prefs_data temp;
        pf_initData(&temp, current.next_addr);
        while (temp.hash != 0xffffffff) {
            pf_nextData(&temp);
        }
        prefs->write(current.current_addr + 8, &temp.current_addr, 4);
        pf_nextData(&current);
    } else {
        // 找到空位了
    }

    uint32_t key_hash = pf_BKDRHash(key);
    prefs->write(current.current_addr, &key_hash, 4);
    uint32_t next_addr = current.current_addr + 12 + size;
    prefs->write(current.current_addr + 4, &next_addr, 4);
    for (size_t i = 0; i < size / 2; i++) {
        prefs->write(current.current_addr + 12 + i * 2, data + i * 2, 2);
    }
    prefs->write_addr = next_addr;
    return pf_OK;
}
// put开头的方法 返回0表示操作成功 返回非0值表示操作失败
uint8_t pf_putChar(prefs_t* prefs, const char* key, uint8_t value)
{
    uint16_t temp = value;
    uint16_t* val = &temp;
    return pf_putItem(prefs, key, val, 2);
}
// put开头的方法 返回0表示操作成功 返回非0值表示操作失败
uint8_t pf_putShort(prefs_t* prefs, const char* key, uint16_t value)
{
    uint16_t temp = value;
    uint16_t* val = &temp;
    return pf_putItem(prefs, key, val, 2);
}
uint8_t pf_isKey(prefs_t* prefs, const char* key)
{
    prefs_data current;
    uint8_t res = pf_findNext(prefs, &current, key);
    if (res == pf_OK) {
        prefs->read_addr = current.current_addr;
        return pf_OK;
    } else {
        return pf_NOT_FOUND;
    }
}
uint8_t pf_getChar(prefs_t* prefs, const char* key)
{
    return *(uint16_t*)(prefs->read_addr + 4 + 4 + 4);
}
uint16_t pf_getShort(prefs_t* prefs, const char* key)
{
    return *(uint16_t*)(prefs->read_addr + 4 + 4 + 4);
}
void prefs(prefs_t *p,
    void (*eraseOnePage)(uint32_t flash_addr),
    void (*write)(uint32_t flash_addr, void* data, int len),
    void (*read)(uint32_t flash_addr, void* data, int len),
    uint32_t start_addr, uint32_t page_size, uint32_t page_cnt)
{
    p->start_addr = start_addr;
    p->eraseOnePage = eraseOnePage;
    p->write = write;
    p->read = read;
    p->page_size = page_size;
    p->page_cnt = page_cnt;
}