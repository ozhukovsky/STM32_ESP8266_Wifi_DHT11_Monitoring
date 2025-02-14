/*
 * ringbuffer.h
 *
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include <stdbool.h>
#include <stdint.h>

#define RING_BUF_SIZE (511 + 1) //+1 additional element to indicate whether a buffer is full.

#define RING_BUF_OK	  	  0
#define RING_BUF_FULL 	  1
#define RING_BUF_NO_SPACE 2

typedef struct
{
	uint16_t head;
	uint16_t tail;
	uint8_t  buffer[RING_BUF_SIZE];
} RingBuffer;

void RingBuffer_Init(RingBuffer* buf);
uint16_t RingBuffer_GetDataLength(const RingBuffer* buf);
uint16_t RingBuffer_Read(RingBuffer* buf, uint8_t* data, uint16_t size);
uint8_t RingBuffer_Write(RingBuffer* buf, const uint8_t* data, uint16_t size);
#endif /* INC_RINGBUFFER_H_ */
