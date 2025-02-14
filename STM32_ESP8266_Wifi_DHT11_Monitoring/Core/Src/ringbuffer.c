/*
 * ringbuffer.c
 *
 */

#include "ringbuffer.h"

void RingBuffer_Init(RingBuffer* buf)
{
	buf->head = buf->tail = 0;
}

uint16_t RingBuffer_GetDataLength(const RingBuffer* buf)
{
	uint16_t length;

	if (buf->head >= buf->tail)
	{
		length = buf->head - buf->tail;
	}
	else
	{
		length = RING_BUF_SIZE - 1 - (buf->tail - buf->head); // -1 by reason there's an additional element to check whether an array is empty
	}

	return length;
}

uint16_t RingBuffer_Read(RingBuffer* buf, uint8_t* data, uint16_t size)
{
	uint16_t read_count = 0;

	while ((read_count < size) && (buf->head != buf->tail)) //buf->head == buf->tail in case of the ring buffer is empty
	{
		data[read_count++] = buf->buffer[buf->tail];
		buf->tail 		   = (buf->tail + 1) % RING_BUF_SIZE;
	}

	return read_count;
}

uint8_t RingBuffer_Write(RingBuffer* buf, const uint8_t* data, uint16_t size)
{
	uint16_t write_count = 0;
	uint8_t  ret;

	if (((buf->head + 1) % RING_BUF_SIZE) == buf->tail)
	{
		ret = RING_BUF_FULL;
	}
	else if ((RING_BUF_SIZE - 1 - RingBuffer_GetDataLength(buf)) < size) // -1 by reason there's an additional element to check whether an array is empty
	{
		ret = RING_BUF_NO_SPACE;
	}
	else
	{
		while (write_count < size)
		{
			buf->buffer[buf->head] = data[write_count++];
			buf->head 	   		   = (buf->head + 1) % RING_BUF_SIZE;
		}

		ret = RING_BUF_OK;
	}

	return ret;
}
