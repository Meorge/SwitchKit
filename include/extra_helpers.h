#ifndef EXTRA_HELPERS_H
#define EXTRA_HELPERS_H

#define INV_LERP(a, b, v) ((v) - (a)) / ((b) - (a))

namespace SwitchKit {
void print_hex(uint8_t *p_buf, uint8_t p_size) {
    for (int i = 0; i < p_size; i++) {
        printf("%02X ", p_buf[i]);
    }
    printf("\n");
}
}

#endif // EXTRA_HELPERS_H
