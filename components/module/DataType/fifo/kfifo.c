#include "kfifo.h"
// 初始化缓冲区
struct ring_buffer *ring_buffer_init(uint32_t size, void (*lock)(bool))
{
    // assert(buffer);
    struct ring_buffer *ring_buf = NULL;
    if (size & (size - 1))
    { /*如果不是2的幂次方，则向上取到2的幂次方*/
        size = roundup_pow_of_two(size);
    }
    ring_buf = (struct ring_buffer *)malloc(sizeof(struct ring_buffer));
    void *buffer = malloc(size); 
    if (!ring_buf)
    {
        return ring_buf;
    }
    memset(ring_buf, 0, sizeof(struct ring_buffer));
    ring_buf->buffer = buffer;
    ring_buf->size = size;
    ring_buf->in = 0;
    ring_buf->out = 0;
    ring_buf->lock = lock;
    return ring_buf;
}

// 释放缓冲区
void ring_buffer_free(struct ring_buffer *ring_buf)
{
    if (ring_buf)
    {
        if (ring_buf->buffer)
        {
            free(ring_buf->buffer);
            ring_buf->buffer = NULL;
        }
        free(ring_buf);
        ring_buf = NULL;
    }
}

// 缓冲区的长度
uint32_t __ring_buffer_len(const struct ring_buffer *ring_buf)
{
    return (ring_buf->in - ring_buf->out);
}

// 从缓冲区中观测数据
uint32_t __ring_buffer_peek(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    // assert(ring_buf || buffer);
    uint32_t len = 0;
    size = k_min(size, ring_buf->in - ring_buf->out);
    /* first get the data from fifo->out until the end of the buffer */
    len = k_min(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
    memcpy(buffer, ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + len, ring_buf->buffer, size - len);
    // ring_buf->out += size;
    return size;
}

// 从缓冲区中读取数据
uint32_t __ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    // assert(ring_buf || buffer);
    uint32_t len = 0;
    size = k_min(size, ring_buf->in - ring_buf->out);
    /* first get the data from fifo->out until the end of the buffer */
    len = k_min(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
    memcpy(buffer, ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + len, ring_buf->buffer, size - len);
    ring_buf->out += size;
    return size;
}

// 向缓冲区中写入数据
uint32_t __ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    // assert(ring_buf || buffer);
    uint32_t len = 0;
    size = k_min(size, ring_buf->size - ring_buf->in + ring_buf->out);
    /* first put the data starting from fifo->in to buffer end */
    len = k_min(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));
    memcpy(ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), buffer, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(ring_buf->buffer, buffer + len, size - len);
    ring_buf->in += size;
    return size;
}

uint32_t ring_buffer_len(const struct ring_buffer *ring_buf)
{
    uint32_t len = 0;
    if (ring_buf->lock)
    {
        ring_buf->lock(true);
    }
    len = __ring_buffer_len(ring_buf);
    if (ring_buf->lock)
    {
        ring_buf->lock(false);
    }
    return len;
}

uint32_t ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    if (ring_buf->lock)
    {
        ring_buf->lock(true);
    }
    ret = __ring_buffer_get(ring_buf, buffer, size);
    if (ring_buf->lock)
    {
        ring_buf->lock(false);
    }
    return ret;
}

uint32_t ring_buffer_peek(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    if (ring_buf->lock)
    {
        ring_buf->lock(true);
    }
    ret = __ring_buffer_peek(ring_buf, buffer, size);
    // if (ring_buf->lock)
    // {
    //     ring_buf->lock(false);
    // }
    return ret;
}

uint32_t ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    if (ring_buf->lock)
    {
        ring_buf->lock(true);
    }
    ret = __ring_buffer_put(ring_buf, buffer, size);
    if (ring_buf->lock)
    {
        ring_buf->lock(false);
    }
    return ret;
}