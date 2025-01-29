#pragma once

#include <stddef.h>

typedef struct ss4s_ringbuf ss4s_ringbuf_t;

ss4s_ringbuf_t *ringbuf_new(size_t capacity);

size_t ringbuf_write(ss4s_ringbuf_t *buf, const unsigned char *src, size_t size);

size_t ringbuf_read(ss4s_ringbuf_t *buf, unsigned char *dst, size_t size);

size_t ringbuf_rewind(ss4s_ringbuf_t *buf, size_t size);

void ringbuf_clear(ss4s_ringbuf_t *buf);

size_t ringbuf_size(const ss4s_ringbuf_t *buf);

size_t ringbuf_capacity(const ss4s_ringbuf_t *buf);

void ringbuf_delete(ss4s_ringbuf_t *buf);