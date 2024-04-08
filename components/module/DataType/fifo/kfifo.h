/*******************************************
# Author        : Retuze 
# Date          : 2024-01-14 04:28:08
# LastEditors   : Retuze 
# LastEditTime  : 2024-01-14 16:12:59
# Description   : 
*********************************************/
#ifndef _KFIFO_H_
#define _KFIFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// 取a和b中最小值
#define k_min(a, b) (((a) < (b)) ? (a) : (b))

static inline int generic_fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }
    return r;
}

static inline unsigned long roundup_pow_of_two(unsigned long x)
{
    return (1UL << generic_fls(x - 1));
}

struct ring_buffer
{
    void *buffer;  // 缓冲区
    uint32_t size; // 大小
    uint32_t in;   // 入口位置
    uint32_t out;  // 出口位置
    void (*lock)(bool) ;   // 互斥锁
};

struct ring_buffer *ring_buffer_init(uint32_t size, void (*lock)(bool));
void ring_buffer_free(struct ring_buffer *ring_buf);
uint32_t ring_buffer_len(const struct ring_buffer *ring_buf);
uint32_t ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size);
uint32_t ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size);
uint32_t ring_buffer_peek(struct ring_buffer *ring_buf, void *buffer, uint32_t size);
#ifdef __cplusplus
}
#endif

#endif // _KFIFO_H_