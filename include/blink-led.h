/**
 * @file blink-led.h
 * @brief Blink LED
 * @author Higor Grigorio <higorgrigorio@gmail.com>
 * @version 1.0.0
 * @date 2020-05-01
 *
 */

#ifndef _BlinkLed_h_
#define _BlinkLed_h_

#include <Arduino.h>

#ifdef USE_BUILTIN_LED
#define BLINK_LED_PIN LED_BUILTIN
#else
#ifndef BLINK_LED_PIN
#warning "BLINK_LED_PIN is not defined. Using pin 2."
#define BLINK_LED_PIN 2
#endif // ! BLINK_LED_PIN
#endif // ! USE_BUILTIN_LED

/**
 * @brief Turn on the built-in LED
 */
auto TurnOnBuiltInLed() -> void
{
    digitalWrite(BLINK_LED_PIN, HIGH);
}

/**
 * @brief Turn off the built-in LED
 */
auto TurnOffBuiltInLed() -> void
{
    digitalWrite(BLINK_LED_PIN, LOW);
}

#endif // ! _BlinkLed_h_