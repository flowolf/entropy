/*
 * User interface.
 * Button and LED handling.
 *
 * Copyright 2014, 2015 Mycelium SA, Luxembourg.
 *
 * This file is part of Mycelium Entropy.
 *
 * Mycelium Entropy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.  See file GPL in the source code
 * distribution or <http://www.gnu.org/licenses/>.
 *
 * Mycelium Entropy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <led.h>
#include <ioport.h>

#include "sys/conf_system.h"
#include "sys/now.h"
#include "ui.h"

static int led_pulses, led_pause;

static void ast_per_interrupt(void)
{
    static int cnt;

    ast_clear_interrupt_flag(AST, AST_INTERRUPT_PER);

    if (cnt >= 0)
        LED_Toggle(LED0);
    if (++cnt >= led_pulses)
        cnt = -led_pause;
}

void ui_init(void)
{
    LED_On(LED0);

    // Initialise AST for LED blinking.
    ast_init();

    // Initialise button.
    ioport_set_pin_dir(BUTTON_0_PIN, IOPORT_DIR_INPUT);
    ioport_set_pin_mode(BUTTON_0_PIN, IOPORT_MODE_PULLUP |
            IOPORT_MODE_GLITCH_FILTER);
}

// Device is busy: rapid blinking.
void ui_busy(void)
{
    LED_Off(LED0);
    ast_write_periodic0_value(AST, AST_PER_1HZ - 4);    // ~8 Hz flashing
    ast_clear_interrupt_flag(AST, AST_INTERRUPT_PER);
    ast_set_callback(AST, AST_INTERRUPT_PER, ast_per_interrupt, AST_PER_IRQn,
                     CONFIG_AST_IRQ_PRIO);
}

// Error.
void ui_error(int code)
{
    LED_Off(LED0);
    led_pulses = code << 1;
    led_pause = 8;
    ast_write_periodic0_value(AST, AST_PER_1HZ - 3);    // ~4 Hz flashing
}

// Good firmware: blink the two LEDs alternately, railway crossing-style.
void ui_ok(void)
{
    led_pulses = 0;
    led_pause = 0;
    LED_Toggle(LED1);
    ast_write_periodic0_value(AST, AST_PER_1HZ - 2);    // ~2 Hz flashing
}

// Check the button.
bool ui_button_pressed(void)
{
    return ioport_get_pin_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE;
}
