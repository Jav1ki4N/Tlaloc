//
// Created by i4N on 2025/10/15.
//

#include "uart_app.h"

#include <array>
#include <cstring>

uint8_t recv_data = 0;

/* UART Settings */
void UART_App_Init()
{
    HAL_UART_Receive_IT(&huart1, &recv_data, 1);
    //HAL_UART_Transmit(&huart1,(uint8_t*)"- UART Ready\r\n",16,1000);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        HAL_UART_Receive_IT(&huart1, &recv_data, 1);
    }
}
