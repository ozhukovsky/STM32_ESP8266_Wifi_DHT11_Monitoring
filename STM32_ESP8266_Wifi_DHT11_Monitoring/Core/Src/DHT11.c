/*
 * dht11.c
 *
 */
#include "DHT11.h"

void DHT11_Data(uint8_t* data)
{
	uint8_t resp_status = 0;
	uint8_t byte_num = 0, bit = 0, byte = 0;

	while (resp_status == 0)
	{
		//START SIGNAL
		setPinToOutput(GPIOA, GPIO_PIN_1);

		delayUS_ASM(20000);
		setPinToInput(GPIOA, GPIO_PIN_1);

		//CHECKING RESPONSE
		// DHT11 - 30us high average + 40us low average
		delayUS_ASM(40);

	    if ((GPIOA->IDR & GPIO_PIN_1) == GPIO_PIN_RESET)
	    {
	    	delayUS_ASM(80);

			if ((GPIOA->IDR & GPIO_PIN_1) != GPIO_PIN_RESET)
			{
				resp_status = 1;

				while((GPIOA->IDR & GPIO_PIN_1) != GPIO_PIN_RESET);
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
	    			while((GPIOA->IDR & GPIO_PIN_1) == GPIO_PIN_RESET);

					delayUS_ASM(35);

					if((GPIOA->IDR & GPIO_PIN_1) != GPIO_PIN_RESET)
					{
						byte |= 0x80 >> bit;
					}

					while((GPIOA->IDR & GPIO_PIN_1) != GPIO_PIN_RESET);
	    		}

	    		data[byte_num] = byte;
	    	}
	    }
	    else
	    {
	    	data[4] = 0;
	    }
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
