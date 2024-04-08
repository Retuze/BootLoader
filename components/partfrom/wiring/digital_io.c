/*******************************************
# Author        : Retuze
# Date          : 2024-01-18 08:55:21
# LastEditors   : Retuze
# LastEditTime  : 2024-01-18 09:10:02
# Description   :
*********************************************/
#include "digital_io.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdint.h>

GPIO_TypeDef* STM_GPIO_PORT(uint32_t PinNum)
{
    switch (PinNum / 16) {
    case 0:
        return GPIOA;
        break;
    case 1:
        return GPIOB;
        break;
    case 2:
        return GPIOC;
        break;
    case 3:
        return GPIOD;
        break;
    default:
        return NULL;
        break;
    }
}
uint16_t STM_GPIO_PIN(uint32_t PinNum)
{
    switch (PinNum%16) {
    case 0:
        return GPIO_PIN_0;
        break;
    case 1:
        return GPIO_PIN_1;
        break;
    case 2:
        return GPIO_PIN_2;
        break;
    case 3:
        return GPIO_PIN_3;
        break;
    case 4:
        return GPIO_PIN_4;
        break;
    case 5:
        return GPIO_PIN_5;
        break;
    case 6:
        return GPIO_PIN_6;
        break;
    case 7:
        return GPIO_PIN_7;
        break;
    case 8:
        return GPIO_PIN_8;
        break;
    case 9:
        return GPIO_PIN_9;
        break;
    case 10:
        return GPIO_PIN_10;
        break;
    case 11:
        return GPIO_PIN_11;
        break;
    case 12:
        return GPIO_PIN_12;
        break;
    case 13:
        return GPIO_PIN_13;
        break;
    case 14:
        return GPIO_PIN_14;
        break;
    case 15:
        return GPIO_PIN_15;
        break;
    default:
        return 0;
        break;
    }
 }

void STM_GPIO_CLK_ENABLE(uint32_t PinNum)
{
    switch (PinNum / 16) {
    case 0:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    case 1:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    case 2:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    case 3:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    }
}

void digitalWrite(uint32_t ulPin, uint32_t ulVal)
{
    HAL_GPIO_WritePin(STM_GPIO_PORT(ulPin), STM_GPIO_PIN(ulPin), (GPIO_PinState)ulVal);
}

int digitalRead(uint32_t ulPin)
{
    return (int)HAL_GPIO_ReadPin(STM_GPIO_PORT(ulPin), STM_GPIO_PIN(ulPin));
}

void digitalToggle(uint32_t ulPin)
{
    HAL_GPIO_TogglePin(STM_GPIO_PORT(ulPin), STM_GPIO_PIN(ulPin));
}

void pinMode(uint32_t ulPin, uint32_t mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    STM_GPIO_CLK_ENABLE(ulPin);
    GPIO_InitStruct.Pin = STM_GPIO_PIN(ulPin);
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    switch (mode) {
    case OUTPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;
    case PULLDOWN:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
    case PULLUP:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;
    case OPEN_DRAIN:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;
    case INPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;
    case INPUT_PULLUP:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;
    case INPUT_PULLDOWN:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
    }
    HAL_GPIO_Init(STM_GPIO_PORT(ulPin), &GPIO_InitStruct);
}