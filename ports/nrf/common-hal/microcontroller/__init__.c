/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Scott Shawcroft for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "common-hal/microcontroller/Pin.h"
#include "common-hal/microcontroller/Processor.h"

#include "shared-bindings/microcontroller/__init__.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-bindings/microcontroller/Processor.h"

#include "py/mperrno.h"
#include "py/runtime.h"
#include "supervisor/shared/translate.h"
#include "supervisor/filesystem.h"

#include "nrfx.h"
#include "nrfx_glue.h"
#include "nrf_clock.h"
#include "nrfx_rtc.h"
#include "tick.h"


// Count every 1/1024 seconds (closest to 1ms)
#define RTC_FREQUENCY (1024UL)

// RTC 0 is used by the SoftDevice
// RTC 1 is reserved for clock
// RTC 2 is used by deepsleep
STATIC const nrfx_rtc_t mcu_deepsleep_rtc = NRFX_RTC_INSTANCE(2);

STATIC const nrfx_rtc_config_t mcu_deepsleep_config = {
    .prescaler    = RTC_FREQ_TO_PRESCALER(RTC_FREQUENCY),
    .reliable     = 0,
    .tick_latency = 0, // ignored when reliable == 0
    .interrupt_priority = 6,
};

STATIC void rtc_interrupt_handler(nrfx_rtc_int_type_t int_type);

STATIC void rtc_interrupt_handler(nrfx_rtc_int_type_t int_type) {
    nrfx_rtc_cc_disable(&mcu_deepsleep_rtc, 0);
}

float common_hal_mcu_deepsleep(float timeout_f) {
    if (timeout_f < 0) mp_raise_ValueError(translate("timout must be positive"));
    if (timeout_f >= 16000) mp_raise_ValueError(translate("timout must be < 16000"));
    uint32_t timeout_i = (uint32_t)(1024*timeout_f);

    // Start the low-frequency clock (if it hasn't been started already)
    if (!nrf_clock_lf_is_running()) {
        nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
    }

    // Make sure RTC uninitialized.
    nrfx_rtc_uninit(&mcu_deepsleep_rtc);
    nrfx_rtc_counter_clear(&mcu_deepsleep_rtc);

    // Initialize and start.
    nrfx_rtc_init(&mcu_deepsleep_rtc, &mcu_deepsleep_config, rtc_interrupt_handler);
    nrfx_rtc_cc_set(&mcu_deepsleep_rtc, 0 /*channel*/, timeout_i, true /*enable irq*/);
    nrfx_rtc_enable(&mcu_deepsleep_rtc);

    // Enter deepsleep
    __SEV();
    __WFE();
    __WFE();

    // Resume after deepsleep
    uint32_t counter = nrfx_rtc_counter_get(&mcu_deepsleep_rtc);
    nrfx_rtc_disable(&mcu_deepsleep_rtc);
    float slept_sec = counter / 1024.0;

    // Ajust time.monotonic()
    // Attention: an interrupt handler running before this code will see
    // the "old" value of ticks_ms. But interrupt handlers should be used
    // with caution anyway.
    ticks_ms += (uint64_t)(1000*slept_sec);

    return slept_sec;
}

// This routine should work even when interrupts are disabled. Used by OneWire
// for precise timing.
void common_hal_mcu_delay_us(uint32_t delay) {
    NRFX_DELAY_US(delay);
}

void common_hal_mcu_disable_interrupts() {
    // Never do this if using SoftDevice!!!
}

void common_hal_mcu_enable_interrupts() {
}

void common_hal_mcu_on_next_reset(mcu_runmode_t runmode) {
    // TODO: see atmel-samd for functionality
}

void common_hal_mcu_reset(void) {
    filesystem_flush();
    NVIC_SystemReset();
}

// The singleton microcontroller.Processor object, bound to microcontroller.cpu
// It currently only has properties, and no state.
const mcu_processor_obj_t common_hal_mcu_processor_obj = {
    .base = {
        .type = &mcu_processor_type,
    },
};


STATIC const mp_rom_map_elem_t mcu_pin_globals_table[] = {
  { MP_ROM_QSTR(MP_QSTR_P0_00), MP_ROM_PTR(&pin_P0_00) },
  { MP_ROM_QSTR(MP_QSTR_P0_01), MP_ROM_PTR(&pin_P0_01) },
  { MP_ROM_QSTR(MP_QSTR_P0_02), MP_ROM_PTR(&pin_P0_02) },
  { MP_ROM_QSTR(MP_QSTR_P0_03), MP_ROM_PTR(&pin_P0_03) },
  { MP_ROM_QSTR(MP_QSTR_P0_04), MP_ROM_PTR(&pin_P0_04) },
  { MP_ROM_QSTR(MP_QSTR_P0_05), MP_ROM_PTR(&pin_P0_05) },
  { MP_ROM_QSTR(MP_QSTR_P0_06), MP_ROM_PTR(&pin_P0_06) },
  { MP_ROM_QSTR(MP_QSTR_P0_07), MP_ROM_PTR(&pin_P0_07) },
  { MP_ROM_QSTR(MP_QSTR_P0_08), MP_ROM_PTR(&pin_P0_08) },
  { MP_ROM_QSTR(MP_QSTR_P0_09), MP_ROM_PTR(&pin_P0_09) },
  { MP_ROM_QSTR(MP_QSTR_P0_10), MP_ROM_PTR(&pin_P0_10) },
  { MP_ROM_QSTR(MP_QSTR_P0_11), MP_ROM_PTR(&pin_P0_11) },
  { MP_ROM_QSTR(MP_QSTR_P0_12), MP_ROM_PTR(&pin_P0_12) },
  { MP_ROM_QSTR(MP_QSTR_P0_13), MP_ROM_PTR(&pin_P0_13) },
  { MP_ROM_QSTR(MP_QSTR_P0_14), MP_ROM_PTR(&pin_P0_14) },
  { MP_ROM_QSTR(MP_QSTR_P0_15), MP_ROM_PTR(&pin_P0_15) },
  { MP_ROM_QSTR(MP_QSTR_P0_16), MP_ROM_PTR(&pin_P0_16) },
  { MP_ROM_QSTR(MP_QSTR_P0_17), MP_ROM_PTR(&pin_P0_17) },
  { MP_ROM_QSTR(MP_QSTR_P0_18), MP_ROM_PTR(&pin_P0_18) },
  { MP_ROM_QSTR(MP_QSTR_P0_19), MP_ROM_PTR(&pin_P0_19) },
  { MP_ROM_QSTR(MP_QSTR_P0_20), MP_ROM_PTR(&pin_P0_20) },
  { MP_ROM_QSTR(MP_QSTR_P0_21), MP_ROM_PTR(&pin_P0_21) },
  { MP_ROM_QSTR(MP_QSTR_P0_22), MP_ROM_PTR(&pin_P0_22) },
  { MP_ROM_QSTR(MP_QSTR_P0_23), MP_ROM_PTR(&pin_P0_23) },
  { MP_ROM_QSTR(MP_QSTR_P0_24), MP_ROM_PTR(&pin_P0_24) },
  { MP_ROM_QSTR(MP_QSTR_P0_25), MP_ROM_PTR(&pin_P0_25) },
  { MP_ROM_QSTR(MP_QSTR_P0_26), MP_ROM_PTR(&pin_P0_26) },
  { MP_ROM_QSTR(MP_QSTR_P0_27), MP_ROM_PTR(&pin_P0_27) },
  { MP_ROM_QSTR(MP_QSTR_P0_28), MP_ROM_PTR(&pin_P0_28) },
  { MP_ROM_QSTR(MP_QSTR_P0_29), MP_ROM_PTR(&pin_P0_29) },
  { MP_ROM_QSTR(MP_QSTR_P0_30), MP_ROM_PTR(&pin_P0_30) },
  { MP_ROM_QSTR(MP_QSTR_P0_31), MP_ROM_PTR(&pin_P0_31) },
#ifdef NRF52840
  { MP_ROM_QSTR(MP_QSTR_P1_00), MP_ROM_PTR(&pin_P1_00) },
  { MP_ROM_QSTR(MP_QSTR_P1_01), MP_ROM_PTR(&pin_P1_01) },
  { MP_ROM_QSTR(MP_QSTR_P1_02), MP_ROM_PTR(&pin_P1_02) },
  { MP_ROM_QSTR(MP_QSTR_P1_03), MP_ROM_PTR(&pin_P1_03) },
  { MP_ROM_QSTR(MP_QSTR_P1_04), MP_ROM_PTR(&pin_P1_04) },
  { MP_ROM_QSTR(MP_QSTR_P1_05), MP_ROM_PTR(&pin_P1_05) },
  { MP_ROM_QSTR(MP_QSTR_P1_06), MP_ROM_PTR(&pin_P1_06) },
  { MP_ROM_QSTR(MP_QSTR_P1_07), MP_ROM_PTR(&pin_P1_07) },
  { MP_ROM_QSTR(MP_QSTR_P1_08), MP_ROM_PTR(&pin_P1_08) },
  { MP_ROM_QSTR(MP_QSTR_P1_09), MP_ROM_PTR(&pin_P1_09) },
  { MP_ROM_QSTR(MP_QSTR_P1_10), MP_ROM_PTR(&pin_P1_10) },
  { MP_ROM_QSTR(MP_QSTR_P1_11), MP_ROM_PTR(&pin_P1_11) },
  { MP_ROM_QSTR(MP_QSTR_P1_12), MP_ROM_PTR(&pin_P1_12) },
  { MP_ROM_QSTR(MP_QSTR_P1_13), MP_ROM_PTR(&pin_P1_13) },
  { MP_ROM_QSTR(MP_QSTR_P1_14), MP_ROM_PTR(&pin_P1_14) },
  { MP_ROM_QSTR(MP_QSTR_P1_15), MP_ROM_PTR(&pin_P1_15) },
#endif
};
MP_DEFINE_CONST_DICT(mcu_pin_globals, mcu_pin_globals_table);
