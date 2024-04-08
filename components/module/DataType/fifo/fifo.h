#ifndef _FIFO_H_
#define _FIFO_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "kfifo.h"

#ifdef __cplusplus
}
class FIFO
{
private:
    
public:
    struct ring_buffer *ring_buf;
    FIFO(uint32_t size = 128, void (*lock)(bool) = nullptr)
    {
        ring_buf=ring_buffer_init(size,lock);
    }
    ~FIFO()
    {
        ring_buffer_free(ring_buf);
    }
    
    uint32_t peek(void *buffer,uint32_t size)
    {
        return ring_buffer_peek(ring_buf,buffer,size);
    }
    uint32_t put(void *buffer,uint32_t size)
    {
        return ring_buffer_put(ring_buf,buffer,size);
    }
    void putChar(uint8_t val)
    {
        uint8_t buffer[1] = {val};
        ring_buffer_put(ring_buf,buffer,sizeof(buffer));
    }
    void putInt(uint32_t val)
    {
        uint32_t buffer[1]={val};
        ring_buffer_put(ring_buf,buffer,sizeof(buffer));
    }
    void putFloat(float val)
    {
        float buffer[1]={val};
        ring_buffer_put(ring_buf,buffer,sizeof(buffer));
    }
    uint32_t get(void *buffer,uint32_t size)
    {
        return ring_buffer_get(ring_buf,buffer,size);
    }
    uint8_t getChar(void)
    {
        uint8_t buffer[1] = {0};
        ring_buffer_get(ring_buf,buffer,sizeof(buffer));
        return buffer[0];
    }
    uint32_t getInt(void)
    {
        uint32_t buffer[1] = {0};
        ring_buffer_get(ring_buf,buffer,sizeof(buffer));
        return buffer[0];
    }
    float getFloat(void)
    {
        float buffer[1] = {0};
        ring_buffer_get(ring_buf,buffer,sizeof(buffer));
        return buffer[0];
    }
    uint32_t length(void)
    {
        return ring_buffer_len(ring_buf);
    }
    uint32_t capacity(void)
    {
        return ring_buf->size;
    }
};

#endif

#endif // _FIFO_H_