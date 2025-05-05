#ifndef EXTRA_HELPERS_H
#define EXTRA_HELPERS_H

#define INV_LERP(a, b, v) ((v) - (a)) / ((b) - (a))

void print_hex(uint8_t *buf, uint8_t size) {
    for (int i = 0; i < size; i++) {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

#endif // EXTRA_HELPERS_H