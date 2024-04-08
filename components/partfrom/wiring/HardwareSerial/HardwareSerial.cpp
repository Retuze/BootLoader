#include "HardwareSerial.h"

HardwareSerial Serial(&huart1);

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart==Serial.huart)
    {
        Serial.get(huart, Size);
        Serial.fun();
    }
}