#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

typedef int(*nalu_cb)(void *ctx, const unsigned char *nalu, size_t size);

static int nalu_read(FILE *f, nalu_cb cb, void *ctx) {
    assert(f != NULL);
    assert(cb != NULL);
    int c, headIdx = 0, naluCount = 0;
    unsigned char *buf = malloc(1024 * 1024);
    size_t bufSize = 0;
    int ret = 0;
    while (ret == 0 && (c = fgetc(f)) >= 0) {
        buf[bufSize++] = c;
        switch (c) {
            case 0: {
                headIdx++;
                break;
            }
            case 1: {
                if (headIdx == 3) {
                    naluCount++;
                    if (bufSize > 4) {
                        ret = cb(ctx, buf, bufSize - 4);
                        bufSize = 4;
                    }
                }
                headIdx = 0;
                break;
            }
            default: {
                headIdx = 0;
                break;
            }
        }
    }
    if (ret == 0) {
        ret = cb(ctx, buf, bufSize);
    }

    free(buf);

    return ret;
}