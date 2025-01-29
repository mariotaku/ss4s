#include "ringbuf.h"

#include <stdlib.h>
#include <string.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

struct ss4s_ringbuf {
    size_t cap, size;
    size_t head, tail;
};

ss4s_ringbuf_t *ringbuf_new(size_t capacity) {
    ss4s_ringbuf_t *buf = malloc(sizeof(ss4s_ringbuf_t) + capacity);
    buf->cap = capacity;
    buf->size = 0;
    buf->head = 0;
    buf->tail = 0;
    return buf;
}

size_t ringbuf_write(ss4s_ringbuf_t *buf, const unsigned char *src, size_t size) {
    if (buf->size + size > buf->cap) {
        // Buffer overflow
        return 0;
    }
    unsigned char *dst = (unsigned char *) buf + sizeof(ss4s_ringbuf_t);
    size_t tmp_tail = buf->tail + size;
    if (tmp_tail < buf->cap) {
        // One time read
        memcpy(&dst[buf->tail], src, size);
    } else {
        size_t tmp_write = buf->cap - buf->tail;
        // Append parts at tail
        memcpy(&dst[buf->tail], src, tmp_write);
        tmp_tail = size - tmp_write;
        // Append from start
        memcpy(dst, &src[tmp_write], tmp_tail);
    }
    buf->size += size;
    buf->tail = tmp_tail;

    return size;
}

size_t ringbuf_read(ss4s_ringbuf_t *buf, unsigned char *dst, size_t size) {
    size_t read_size = min(buf->size, size);
    if (read_size == 0) {
        return 0;
    }
    const unsigned char *src = (unsigned char *) buf + sizeof(ss4s_ringbuf_t);
    size_t tmp_head = buf->head + read_size;
    if (tmp_head < buf->cap) {
        // One time read
        memcpy(dst, &src[buf->head], read_size);
    } else {
        size_t tmp_read = buf->cap - buf->head;
        // Read parts before cap
        memcpy(dst, &src[buf->head], tmp_read);
        tmp_head = read_size - tmp_read;
        // Read parts from start
        memcpy(&dst[tmp_read], src, tmp_head);
    }
    buf->size -= read_size;
    buf->head = tmp_head;
    return read_size;
}

size_t ringbuf_rewind(ss4s_ringbuf_t *buf, size_t size) {
    size_t rewind_size = min(buf->size, size);
    if (rewind_size == 0) {
        return 0;
    }
    buf->size -= rewind_size;
    if (buf->tail < rewind_size) {
        buf->tail = buf->cap - (rewind_size - buf->tail);
    } else {
        buf->tail = buf->tail - rewind_size;
    }
    return rewind_size;
}

void ringbuf_clear(ss4s_ringbuf_t *buf) {
    buf->size = 0;
    buf->head = 0;
    buf->tail = 0;
}

size_t ringbuf_size(const ss4s_ringbuf_t *buf) {
    return buf->size;
}

size_t ringbuf_capacity(const ss4s_ringbuf_t *buf) {
    return buf->cap;
}

void ringbuf_delete(ss4s_ringbuf_t *buf) {
    free(buf);
}
