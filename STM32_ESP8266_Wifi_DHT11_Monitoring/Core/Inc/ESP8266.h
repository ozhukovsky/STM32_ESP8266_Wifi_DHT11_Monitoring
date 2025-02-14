/*
 * ESP8266_HAL.h
 *
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

#define ESP_STAT_OK 	0
#define ESP_STAT_NOINIT 1

uint8_t ESP_Init(UART_HandleTypeDef* esp_uart, char* ssid, char* pass);
void ESP_ProcessInput(void);
void ESP_UART_IRQHandler(void);
bool ESP_CheckPendingData(void);

#endif /* INC_ESP8266_H_ */
