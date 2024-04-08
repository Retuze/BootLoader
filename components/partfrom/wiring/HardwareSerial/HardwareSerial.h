#ifndef _HARDWARESERIAL_H_
#define _HARDWARESERIAL_H_

#include "usart.h"
#include "partfrom.h"

#ifdef __cplusplus
extern "C" {
#endif

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size);

#ifdef __cplusplus
}

class HardwareSerial : public Print {
private:
    FIFO fifo;
    uint8_t _lastDMASize = 0;
    size_t write(const uint8_t* str, size_t size)
    {
        HAL_UART_Transmit(huart, str, size, 100);
        return size;
    }

public:
    UART_HandleTypeDef* huart;
    void (*fun)(); // 接收到数据时的回调函数
    void begin()
    {
        
        HAL_UARTEx_ReceiveToIdle_DMA(huart, (uint8_t*)fifo.ring_buf->buffer, fifo.ring_buf->size);
    }
    void get(UART_HandleTypeDef* huart, uint16_t Size)
    {
        if (huart == this->huart) {
            fifo.ring_buf->in += (Size - _lastDMASize);
            (huart->RxEventType == HAL_UART_RXEVENT_TC) ? _lastDMASize = 0 : _lastDMASize = Size;
        }
    }
    HardwareSerial(UART_HandleTypeDef* huart)
    {
        this->huart = huart;
    }
    // 当buff为NULL时，调用malloc动态创建
    char* read(char* buff = NULL)
    {
        if (buff == NULL) {
            buff = (char*)calloc(fifo.length() + 1, sizeof(char));
        }
        fifo.get(buff, fifo.length());
        return buff;
    }
    char* read(char* buff ,int len)
    {
        fifo.get(buff, len);
        return buff;
    }
    char* peek()
    {
        char* buff = (char*)calloc(fifo.length() + 1, sizeof(char));
        fifo.peek(buff, fifo.length());
        return buff;
    }
    int available()
    {
        return fifo.length();
    }

    // 在接收中断触发时，调用绑定的回调
    void onReceive(void (*fun)())
    {
        this->fun = fun;
    }
};

extern HardwareSerial Serial;

#endif

#endif // _HARDWARESERIAL_H_