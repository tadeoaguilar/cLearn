#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Moving average of the last 8 samples: ring buffer + running sum = O(1) per sample. */
enum { MA_N = 8 }; // a power of two, so the division is a shift
typedef struct {
    uint16_t buf[MA_N];
    uint32_t sum;
    uint8_t idx, count;
} MovingAvg;

static uint16_t ma_update(MovingAvg* f, uint16_t x) {
    f->sum -= f->buf[f->idx]; // drop the oldest sample (0 until the buffer fills)
    f->buf[f->idx] = x;
    f->sum += x;
    f->idx = (f->idx + 1) & (MA_N - 1);
    if (f->count < MA_N) f->count++;
    return (uint16_t)(f->sum / f->count);
}

/* Median of the last 5: a single spike can never be the middle value. */
typedef struct {
    uint16_t buf[5];
    uint8_t idx, count;
} Median5;

static uint16_t median_update(Median5* f, uint16_t x) {
    f->buf[f->idx] = x;
    f->idx = (uint8_t)((f->idx + 1) % 5);
    if (f->count < 5) f->count++;
    uint16_t s[5];
    memcpy(s, f->buf, sizeof s);
    for (int i = 1; i < f->count; i++) // insertion sort on a copy: 5 elements, so cheap
        for (int j = i; j > 0 && s[j - 1] > s[j]; j--) {
            uint16_t t = s[j];
            s[j] = s[j - 1];
            s[j - 1] = t;
        }
    return s[f->count / 2];
}

/* Exponential filter y += (x - y) / 8, in fixed point with 4 fractional bits to keep precision. */
typedef struct {
    int32_t y_q4; // value × 16
    int primed;
} Expo;

static uint16_t expo_update(Expo* f, uint16_t x) {
    int32_t x_q4 = (int32_t)x << 4;
    if (!f->primed) {
        f->y_q4 = x_q4;
        f->primed = 1;
    }
    f->y_q4 += (x_q4 - f->y_q4) / 8; // division, not >>, for negative differences (>> on negatives is impl-defined)
    return (uint16_t)((f->y_q4 + 8) >> 4);
}

int main(void) {
    MovingAvg ma = {0};
    Median5 med = {0};
    Expo ex = {0};
    uint32_t rng = 1;
    printf("  t   raw   avg8  median5  expo\n");
    for (int t = 0; t < 40; t++) {
        rng = rng * 1103515245u + 12345u;
        int noise = (int)((rng >> 16) % 21) - 10; // ±10 LSB
        int truth = 1000 + t * 5;                  // a slow ramp
        int raw = truth + noise;
        if (t == 12 || t == 27) raw = 4095;        // spikes: a loose wire, ESD...
        uint16_t a = ma_update(&ma, (uint16_t)raw);
        uint16_t m = median_update(&med, (uint16_t)raw);
        uint16_t e = expo_update(&ex, (uint16_t)raw);
        if (t % 3 == 0 || t == 12 || t == 13 || t == 27 || t == 28)
            printf("%3d  %4d  %5u  %7u  %4u%s\n", t, raw, a, m, e, raw == 4095 ? "   ← spike" : "");
    }
    printf("The average and the exponential filter get dragged up by spikes for several samples;\n"
           "the median ignores them. Real firmware often chains median → average.\n");
    return 0;
}
