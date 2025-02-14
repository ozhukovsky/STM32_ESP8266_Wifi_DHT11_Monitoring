/*
 * dht11.h
 *
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include <stdint.h>
#include "main.h"

void DHT11_Data(uint8_t* data);
void setPinToInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void setPinToOutput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif /* INC_DHT11_H_ */
