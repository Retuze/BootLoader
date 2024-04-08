#include "write.h"

// gcc 标准printf重定向，默认不支持浮点数打印
int _write(int fd, char *pBuffer, int size)
{
    HAL_UART_Transmit(&huart1, (const uint8_t *)pBuffer, size, 100);
    return size;
}