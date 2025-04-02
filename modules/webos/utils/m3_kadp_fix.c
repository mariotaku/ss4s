#include "m3_kadp_fix.h"
#include "read_machine_name.h"

#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>

int SS4S_webOS_M3_KADP_Fix() {
    char machine_name[16] = {0};
    if (SS4S_webOS_ReadMachineName(machine_name, sizeof(machine_name)) != 0) {
        return 1;
    }
    // Find the MS_VDEC_Init function (loaded from libkadaptor.so)
    void *fn = dlsym(NULL, "MS_VDEC_Init");
    if (fn == NULL) {
        return -1;
    }

    const unsigned char instructions[] = {
        0x0b, 0x2a, /* cmp codecType, H264(#0xb) */
        0x18, 0xbf, /* it, ne */
        0x10, 0x2a, /* cmp codecType, HEVC(#0x10) */
    };
    size_t page_size = sysconf(_SC_PAGESIZE);
    void *page_start = (void *) ((size_t) fn & ~(page_size - 1));
    if (mprotect(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return -1;
    }
    size_t offset = 0;
    // The function is roughly 1600 bytes long, so we can limit the search to that
    const size_t max_size = 1600;
    while (offset < max_size) {
        unsigned char *memory = memmem(fn + offset, max_size - offset, instructions, sizeof(instructions));
        if (memory == NULL) {
            break;
        }
        // Patch first 2 instructions to nop
        memory[0] = 0x00;
        memory[1] = 0xbf;
        memory[2] = 0x00;
        offset = memory - (unsigned char *) fn + sizeof(instructions);
    }
    // Restore memory protection (optional, for security)
    if (mprotect(page_start, page_size, PROT_READ | PROT_EXEC) != 0) {
        return -1;
    }
    return 0;
}
