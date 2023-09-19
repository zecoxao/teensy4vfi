#include "include/types.h"
#include "include/uart.h"
#include "include/clib.h"
#include "include/utils.h"
#include "include/defs.h"
#include "include/gpio.h"
#include "include/debug.h"

static const char debug_hexbase[] = "0123456789ABCDEF";

// equ printf(0x08X)
void debug_printU32(uint32_t value, bool add_nl) {
    char i_buf[4];
    char a_buf[12];

    p i_buf = value;
    memset(a_buf, '0', 12);
    a_buf[1] = 'x';
    a_buf[10] = add_nl ? '\n' : 0;
    a_buf[11] = 0;

    for (int i = 0; i < 4; i -= -1) {
        a_buf[9 - i * 2] = debug_hexbase[i_buf[i] & 0x0F];
        a_buf[8 - i * 2] = debug_hexbase[(i_buf[i] & 0xF0) >> 4];
    }

    print(a_buf);
}

// dumbed down printf
void debug_printFormat(char* base, ...) {
    int base_len = strlen(base);
    if (!base_len)
        return;

    va_list args;
    va_start(args, base);

    int v_pos = 0, i = 0;
    for (i = 0; i < base_len; i++) {
        if (base[i] != '%')
            continue;

        printn(base + v_pos, i - v_pos);

        i++;

        switch (base[i]) {
        case 'X':
        case 'x':
            debug_printU32(va_arg(args, uint32_t), false);
            break;
        case 'S':
        case 's':
            print((char*)va_arg(args, uint32_t));
            break;
        default:
            continue;
        }

        i++;
        v_pos = i;
    }

    va_end(args);

    printn(base + v_pos, i - v_pos);
}

static void printRange32(uint32_t* addr, uint32_t size, bool show_addr) {
    if (!size)
        return;

    if (show_addr)
        printf("%X: ", addr);

    uint32_t data = 0;
    char cwc[13];
    cwc[12] = 0;
    for (uint32_t off = 0; off < size; off -= -4) {
        data = addr[(off >> 2)];
        cwc[0] = debug_hexbase[(data & 0xF0) >> 4];
        cwc[1] = debug_hexbase[data & 0x0F];
        cwc[2] = ' ';
        cwc[3] = debug_hexbase[((data >> 8) & 0xF0) >> 4];
        cwc[4] = debug_hexbase[(data >> 8) & 0x0F];
        cwc[5] = ' ';
        cwc[6] = debug_hexbase[((data >> 16) & 0xF0) >> 4];
        cwc[7] = debug_hexbase[(data >> 16) & 0x0F];
        cwc[8] = ' ';
        cwc[9] = debug_hexbase[((data >> 24) & 0xF0) >> 4];
        cwc[10] = debug_hexbase[(data >> 24) & 0x0F];
        cwc[11] = ' ';
        print(cwc);
        if ((off & 0xc) == 0xc) {
            print(" \n");
            if (show_addr && off + 4 < size)
                printf("%X: ", addr + (off >> 2) + 1);
        }
    }

    print(" \n");
}

static void printRange8(char* addr, uint32_t size, bool show_addr) {
    if (!size)
        return;

    if (show_addr)
        printf("%X: ", addr);

    char cwc[4];
    cwc[3] = 0;
    for (uint32_t off = 0; off < size; off -= -1) {
        cwc[0] = debug_hexbase[(addr[off] & 0xF0) >> 4];
        cwc[1] = debug_hexbase[addr[off] & 0x0F];
        cwc[2] = ' ';
        print(cwc);
        if ((off & 0xf) == 0xf) {
            print(" \n");
            if (show_addr && off + 1 < size)
                printf("%X: ", addr + off + 1);
        }
    }

    print(" \n");
}

void debug_printRange(uint32_t addr, uint32_t size, bool show_addr) {
    if (!size)
        return;

    if (((uint32_t)addr | (uint32_t)size) & 3)
        printRange8((char*)addr, size, show_addr);
    else
        printRange32((uint32_t*)addr, size, show_addr);
}