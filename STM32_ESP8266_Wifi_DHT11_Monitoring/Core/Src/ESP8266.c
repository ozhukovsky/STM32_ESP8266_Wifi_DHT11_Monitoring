#include <stdio.h>
#include <string.h>
#include "ESP8266.h"
#include "ringbuffer.h"
#include "DHT11.h"

#define COMMAND_OK  0
#define COMMAND_ERR 1

#define ESP_RESPONSE_TIMEOUT 3000 //Milliseconds
#define ESP_IPD_TIMEOUT		 10000

UART_HandleTypeDef* huart;


char* check_dht_template = "<!DOCTYPE html> <html>\n<head><meta name=\"viewport\"\
							content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n\
							<title>DHT CONTROL</title>\n<style>html { font-family: Helvetica; \
							display: inline-block; margin: 0px auto; text-align: center;}\n\
							body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\
							h3 {color: #444444;margin-bottom: 50px;}\n.button {display: block;\
							width: 80px;background-color: #1abc9c;border: none;color: white;\
							padding: 13px 30px;text-decoration: none;font-size: 25px;\
							margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n\
							.button-on {background-color: #1abc9c;}\n.button-on:active \
							{background-color: #16a085;}\n.button-off {background-color: #34495e;}\n\
							.button-off:active {background-color: #2c3e50;}\np {font-size: 14px;color: #888;margin-bottom: 10px;}\n\
							</style>\n</head>\n<body>\n<h1>ESP8266 DHT Server</h1>\n\
							<p>Temperature: %s.%s</p><p>Humidity: %s.%s</p></body></html>";

char* unknown_html		 = "<!DOCTYPE html>\
							<html>\
							<head>\
							<title>ESP8266</title>\
							</head>\
							<body>\
							<h1>ESP8266 Server</h1>\
							<p>Request error</p>\
							</body>\
							</html>";

bool data_received = true;
RingBuffer ring_buf;

bool waitForInput(const char* input, uint32_t timeout);
uint32_t receiveDataUntil(char* buf, uint32_t buf_size, char end_ch, uint32_t timeout);
void sendCommand(const char* msg);
void processIPD(void);
void serviceDHT(char* prefix_url, uint8_t link_id);
uint8_t serverSend(char *html, uint32_t size, uint8_t link_id);

/*****************************************************************************************************************************************/

uint8_t ESP_Init(UART_HandleTypeDef* esp_uart, char* ssid, char* pass)
{
	uint8_t init_stat;

	huart = esp_uart;

	__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

	sendCommand("AT+RST\r\n");
//
	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		init_stat = ESP_STAT_NOINIT;
		goto END_INIT;
	}

	HAL_Delay(2000);

	sendCommand("AT\r\n");

	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		init_stat = ESP_STAT_NOINIT;
		goto END_INIT;
	}

	sendCommand("AT+CWMODE_CUR=1\r\n");

	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		init_stat = ESP_STAT_NOINIT;
		goto END_INIT;
	}

	char* cmd = (char*)malloc(strlen("AT+CWJAP_CUR=\"\",\"\"\r\n") + strlen(ssid) + strlen(pass) + 1);

	sprintf(cmd, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, pass);

	sendCommand(cmd);

	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		init_stat = ESP_STAT_NOINIT;
		goto END_INIT;
	}

	sendCommand("AT+CIPMUX=1\r\n");
	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		init_stat = ESP_STAT_NOINIT;
		goto END_INIT;
	}

	sendCommand("AT+CIPSERVER=1,80\r\n");
	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		init_stat = ESP_STAT_NOINIT;
		goto END_INIT;
	}

	init_stat = ESP_STAT_OK;

END_INIT:
	return init_stat;
}

void sendCommand(const char* msg)
{
	HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

bool waitForInput(const char* input, uint32_t timeout)
{
	char 	input_ch = 0;
	uint8_t i 		 = 0;
	bool 	ret 	 = false;

	uint32_t start_tick = HAL_GetTick();

	while ((HAL_GetTick() - start_tick) < timeout)
	{
		if (RingBuffer_Read(&ring_buf, (uint8_t*)&input_ch, 1))
		{
			if (input[i++] != input_ch)
			{
				i = 0;
			}

			if (i == strlen(input))
			{
				ret = true;
				break;
			}

			start_tick = HAL_GetTick();
		}
	}

	return ret;
}

uint8_t serverSend(char *html, uint32_t size, uint8_t link_id)
{
	uint8_t ret;
	char 	cmd[25];

	sprintf(cmd, "AT+CIPSEND=%d,%d\r\n", link_id, size);
	sendCommand(cmd);

	if (!waitForInput(">", ESP_RESPONSE_TIMEOUT))
	{
		ret = COMMAND_ERR;
		goto END;
	}

	sendCommand(html);

	if (!waitForInput("SEND OK", ESP_RESPONSE_TIMEOUT))
	{
		ret = COMMAND_ERR;
		goto END;
	}

	sendCommand("AT+CIPCLOSE=5\r\n");

	if (!waitForInput("OK\r\n", ESP_RESPONSE_TIMEOUT))
	{
		ret = COMMAND_ERR;
		goto END;
	}

	ret = COMMAND_OK;
END:
	return ret;
}

bool ESP_CheckPendingData(void)
{
	return RingBuffer_GetDataLength(&ring_buf) > 0;
}

void ESP_ProcessInput(void)
{
	if (waitForInput("+IPD,", ESP_IPD_TIMEOUT))
	{
		processIPD();
	}
}

void processIPD(void)
{
	char input_buf[11];

	receiveDataUntil(input_buf, sizeof(input_buf), ',', ESP_IPD_TIMEOUT); //Fetching link ID

	uint8_t link_id = atoi(input_buf);

	receiveDataUntil(input_buf, sizeof(input_buf), ':', ESP_IPD_TIMEOUT); //Fetching data length

	uint32_t data_len = atoi(input_buf);

	//Skipping Request method
	receiveDataUntil(input_buf, sizeof(input_buf), ' ', ESP_IPD_TIMEOUT);

	//Getting prefix URL
	receiveDataUntil(input_buf, sizeof(input_buf), ' ', ESP_IPD_TIMEOUT);

	//Free the ring buffer
	RingBuffer_Init(&ring_buf);

	if (!strcmp(input_buf, "/check_dht"))
	{
		serviceDHT(input_buf, link_id);
	}
	else
	{
		serverSend(unknown_html, strlen(unknown_html), link_id);
	}
}

void serviceDHT(char* prefix_url, uint8_t link_id)
{
	char*    html;
	uint32_t html_size;
	uint8_t  dht_data[5];
	char     temp_int[4], temp_dec[4], rh_int[4], rh_dec[4];

	DHT11_Data(dht_data);

	html_size = (strlen(check_dht_template) - 8) + sprintf(temp_int, "%d" ,dht_data[2]) + sprintf(temp_dec, "%d", dht_data[3])
				+ sprintf(rh_int, "%d" ,dht_data[0]) + sprintf(rh_dec, "%d", dht_data[1]) + 1;

	html	  = (char*)malloc(html_size);

	sprintf(html, check_dht_template, temp_int, temp_dec, rh_int, rh_dec);

	serverSend(html, html_size - 1, link_id);
}

uint32_t receiveDataUntil(char* input_buf, uint32_t buf_size, char end_ch, uint32_t timeout)
{
	char      input_ch;
	uint32_t  i = 0;

	uint32_t start_tick = HAL_GetTick();

	while ((HAL_GetTick() - start_tick) < timeout
		 && i < buf_size)
	{
		if (RingBuffer_Read(&ring_buf, (uint8_t*)&input_ch, 1))
		{
			if (input_ch != end_ch)
			{
				input_buf[i++] = input_ch;
			}
			else
			{
				input_buf[i] = '\0';
				break;
			}

			start_tick = HAL_GetTick();
		}
	}

	return i;
}


void ESP_UART_IRQHandler(void)
{
	uint32_t usart_sr_reg = huart->Instance->SR;
	uint8_t  data;

	if ((usart_sr_reg & USART_SR_RXNE) != RESET)
	{
		data = (uint8_t)(huart->Instance->DR);
		RingBuffer_Write(&ring_buf, &data, 1);
	}
}
