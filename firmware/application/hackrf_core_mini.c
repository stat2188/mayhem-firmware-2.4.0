#include <stddef.h>

#include "gpio.h"
#include "gpio_lpc.h"

typedef enum {
    LED1 = 0,
    LED2 = 1,
    LED3 = 2,
    LED4 = 3,
} led_t;

/* GPIO Output PinMux
 *
 * NOTE: gpio_t (from gpio.h) is typedef const struct gpio* — a POINTER type.
 * The array below uses struct gpio (the full type defined in gpio_lpc.h).
 *
 * Stack: 0 bytes (static storage)
 * SRAM:  48 bytes (4 × struct gpio, 12 bytes each)
 * Flash: 48 bytes (initializer data, .rodata)
 */
static struct gpio gpio_led[] = {
    GPIO(2, 1),
    GPIO(2, 2),
    GPIO(2, 8),
#ifdef RAD1O
    GPIO(5, 26),
#endif
};

/*
 * Initialize all LED pins as outputs.
 * gpio_init() in gpio_lpc.c sets all port dir registers to 0 (input);
 * without this call, writing to set/clr registers silently does nothing.
 *
 * Stack: ~16 bytes (one pointer + loop index)
 * SRAM:  0 bytes
 * Flash: ~24 bytes
 */
static void leds_init(void) {
    for (size_t i = 0u; i < sizeof(gpio_led) / sizeof(gpio_led[0]); i++) {
        gpio_output((gpio_t)&gpio_led[i]);
    }
}

void delay(uint32_t duration) {
    while (duration--) {
        /* cannot be optimized out */
        __asm__ volatile("nop");
    }
}

void delay_us_at_mhz(uint32_t us, uint32_t mhz) {
    /* overflow-safe multiply */
    uint64_t cycles64 = (uint64_t)us * (uint64_t)mhz;
    if (cycles64 > UINT32_MAX) {
        cycles64 = UINT32_MAX;
    }
    delay((uint32_t)cycles64);
}

void led_on(const led_t led) {
    gpio_set((gpio_t)&gpio_led[led]);
}

void led_off(const led_t led) {
    gpio_clear((gpio_t)&gpio_led[led]);
}

void halt_and_flash(const uint32_t duration) {
    /* Ensure LED pins are configured as outputs before first use */
    leds_init();

    /* blink LED1, LED2, and LED3 */
    while (1) {
        led_on(LED1);
        led_on(LED2);
        led_on(LED3);
        delay(duration);
        led_off(LED1);
        led_off(LED2);
        led_off(LED3);
        delay(duration);
    }
}
