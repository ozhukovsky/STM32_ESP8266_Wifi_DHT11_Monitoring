/*
 * dht11.c
 *
 */
#include "DHT11.h"

void DHT11_Data(uint8_t* data)
{
	uint8_t resp_status 		= 0;
	uint8_t byte_num, bit, byte = 0;

	//START SIGNAL
	setPinToOutput(GPIOA, GPIO_PIN_1);
	delayUS_ASM(20000);
	setPinToInput(GPIOA, GPIO_PIN_1);

	//CHECKING RESPONSE
	// DHT11 - 30us high average + 40us low average
	delayUS_ASM(40);

    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
    {
    	delayUS_ASM(80);

		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET)
		{
			resp_status = 1;

			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET);
		}
		else
		{
			resp_status = 0;
		}
    }
    else
    {
    	resp_status = 0;
    }

    if (resp_status)
    {
    	for (; byte_num < 5; byte_num++, byte = 0)
    	{
    		for (bit = 0; bit < 8; bit++)
    		{
    			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET);

				delayUS_ASM(35);

				if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET)
				{
					byte |= 0x80 >> bit;
				}

				while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET);
    		}

    		data[byte_num] = byte;
    	}
    }
    else
    {
    	data[4] = 0;
    }
}

void setPinToInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    uint32_t 		  position = 0;
    uint32_t 		  register_offset;
    volatile uint32_t *config_register;
    uint32_t 		  temp;

    while (GPIO_Pin)
    {
        if (GPIO_Pin & 0x01)
        {
            register_offset = (position < 8) ? 0 : 1;
            config_register = &GPIOx->CRL + register_offset;

            temp =  *config_register;
            temp &= ~(0x0F << ((position & 0x07) * 4));
            temp |= (0x04 << ((position & 0x07) * 4));  // Input floating

            *config_register = temp;
        }

        position++;
        GPIO_Pin >>= 1;
    }
}

void setPinToOutput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    uint32_t 		  position = 0;
    uint32_t 		  register_offset;
    volatile uint32_t *config_register;
    uint32_t 		  temp;

    while (GPIO_Pin)
    {
        if (GPIO_Pin & 0x01)
        {
            register_offset = (position < 8) ? 0 : 1;
            config_register = &GPIOx->CRL + register_offset;

            temp =  *config_register;
            temp &= ~(0x0F << ((position & 0x07) * 4));
            temp |= (0x01 << ((position & 0x07) * 4));  // Output push-pull

            *config_register = temp;
        }

        position++;
        GPIO_Pin >>= 1;
    }
}
