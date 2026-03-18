# STM32 ESP8266 Wi‑Fi DHT11 Monitoring Server

A lightweight embedded web server running on an STM32 microcontroller with an ESP8266 Wi‑Fi module.  
The system reads temperature and humidity from a DHT11 sensor and serves the data through a simple HTTP webpage.

This project demonstrates:
- UART communication with ESP8266 using AT commands  
- Custom ring buffer for asynchronous UART reception  
- Bare‑metal DHT11 driver with precise timing  
- HTTP server logic (`+IPD` parsing, dynamic HTML generation)  
- Low‑power operation using STM32 sleep mode  

[DHT11 driver implementation](STM32_ESP8266_Wifi_DHT11_Monitoring/Core/Src/DHT11.c)

[ESP8266 functionality](STM32_ESP8266_Wifi_DHT11_Monitoring/Core/Src/ESP8266.c)

[Ring buffer](STM32_ESP8266_Wifi_DHT11_Monitoring/Core/Src/ringbuffer.c)

[Main functionality](STM32_ESP8266_Wifi_DHT11_Monitoring/Core/Src/main.c)

---

## Features

- Wi‑Fi connection via ESP8266 (AT firmware)
- HTTP server on port 80
- Dynamic HTML page with:
  - Temperature (integer + decimal)
  - Humidity (integer + decimal)
- Custom DHT11 driver (no HAL delays)
- Ring buffer for non‑blocking UART RX
- Sleep mode between requests to reduce power consumption
- Clean separation of modules:
  - `ESP8266.c` — AT command driver + HTTP server
  - `DHT11.c` — sensor driver
  - `ringbuffer.c` — UART RX buffering
  - `main.c` — system initialization and main loop
---

## How It Works

### 1. ESP8266 Initialization
The MCU sends a sequence of AT commands:

- `AT+RST`
- `AT`
- `AT+CWMODE_CUR=1`
- `AT+CWJAP_CUR="SSID","PASS"`
- `AT+CIPMUX=1`
- `AT+CIPSERVER=1,80`

If all responses return `OK`, the server starts.

### 2. Handling Incoming HTTP Requests
ESP8266 sends data in the form:

+IPD,<link_id>,<len>:GET /check_dht HTTP/1.1
Код


The firmware:

- extracts `link_id`
- extracts URL
- calls `serviceDHT()` for `/check_dht`
- sends dynamically generated HTML back to the client

### 3. DHT11 Reading
The driver:

- toggles GPIO between input/output
- sends start signal
- waits for sensor response
- reads 40 bits (5 bytes)
- extracts humidity + temperature

### 4. Low‑Power Mode
Between requests, MCU enters sleep:

```c
HAL_PWR_EnterSLEEPMode(0, PWR_SLEEPENTRY_WFI);
```

## Example Web Output

ESP8266 DHT Server
Temperature: 23.4
Humidity:    45.0

## Requirements

- STM32F103 (or similar)

- ESP8266 with AT firmware

- DHT11 sensor

- UART @ 115200 baud

- STM32CubeIDE or compatible toolchain

## Build & Flash

- Open project in STM32CubeIDE

- Configure your Wi‑Fi credentials in main.c:

```c
ESP_Init(&huart1, "SSID", "PASSWORD");
```
- Build and flash

- Connect to the same Wi‑Fi network

- Open browser and enter ESP8266 IP:
    
http://<esp_ip>/check_dht

## Notes

- HTML template is embedded directly in firmware

- Ring buffer ensures stable UART reception
