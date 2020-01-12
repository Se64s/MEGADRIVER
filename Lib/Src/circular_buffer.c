/**
  ******************************************************************************
  * @file           : circular_buffer.c
  * @brief          : circular buffer implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "circular_buffer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static void advance_pointer(circular_buf_t * cbuf)
{
    assert(cbuf);

    if (cbuf->full)
    {
        cbuf->tail = (cbuf->tail + 1) % cbuf->max;
    }

    cbuf->head = (cbuf->head + 1) % cbuf->max;

    // We mark full because we will advance tail on the next time around
    cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(circular_buf_t * cbuf)
{
    assert(cbuf);

    cbuf->full = false;
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

/* Public function prototypes -----------------------------------------------*/

void circular_buf_init(circular_buf_t * cbuf, uint8_t* buffer, size_t size)
{
    assert(buffer && size && cbuf);

    cbuf->buffer = buffer;
    cbuf->max = size;
    circular_buf_reset(cbuf);

    assert(circular_buf_empty(cbuf));
}

void circular_buf_free(circular_buf_t * cbuf)
{
    assert(cbuf);

    cbuf->buffer = NULL;

    circular_buf_reset(cbuf);
}

void circular_buf_reset(circular_buf_t * cbuf)
{
    assert(cbuf);

    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

size_t circular_buf_size(circular_buf_t * cbuf)
{
    assert(cbuf);

    size_t size = cbuf->max;

    if (!cbuf->full)
    {
        if (cbuf->head >= cbuf->tail)
        {
            size = (cbuf->head - cbuf->tail);
        }
        else
        {
            size = (cbuf->max + cbuf->head - cbuf->tail);
        }
    }

    return size;
}

size_t circular_buf_capacity(circular_buf_t * cbuf)
{
    assert(cbuf);

    return cbuf->max;
}

void circular_buf_put(circular_buf_t * cbuf, uint8_t data)
{
    assert(cbuf && cbuf->buffer);

    cbuf->buffer[cbuf->head] = data;

    advance_pointer(cbuf);
}

int circular_buf_put2(circular_buf_t * cbuf, uint8_t data)
{
    int r = -1;

    assert(cbuf && cbuf->buffer);

    if (!circular_buf_full(cbuf))
    {
        cbuf->buffer[cbuf->head] = data;
        advance_pointer(cbuf);
        r = 0;
    }

    return r;
}

int circular_buf_get(circular_buf_t * cbuf, uint8_t *data)
{
    assert(cbuf && data && cbuf->buffer);

    int r = -1;

    if (!circular_buf_empty(cbuf))
    {
        *data = cbuf->buffer[cbuf->tail];
        retreat_pointer(cbuf);

        r = 0;
    }

    return r;
}

bool circular_buf_empty(circular_buf_t * cbuf)
{
    assert(cbuf);

    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

bool circular_buf_full(circular_buf_t * cbuf)
{
    assert(cbuf);

    return cbuf->full;
}

/*****END OF FILE****/
