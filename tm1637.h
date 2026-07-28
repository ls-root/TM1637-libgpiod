#ifndef TM1637_H
#define TM1637_H

#include <stdbool.h>
#include <stdint.h>

#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000
#define SEG_DP 0b10000000

#define DEFAULT_BIT_DELAY 100

struct tm1637 {
  uint8_t pin_clk;
  uint8_t pin_dio;
  uint8_t brightness;
  unsigned int bit_delay;
  struct gpiod_line_request *request;
};

// Initialize a `tm1637` struct, setting the clock and data pins.
//
// @param chip_path Path to gpiochip charachter device
// @param pin_clk The number of the digital pin connected to the clock pin of
// the module
// @param pin_dio The number of the digital pin connected to the DIO pin of the
// module
// @param bit_delay The delay, in microseconds, between bit transition on the
// serial bus connected to the display
struct tm1637 *tm1637_init(char *chip_path, uint8_t pin_clk, uint8_t pin_dio,
                           unsigned int bit_delay);

// Sets the brightness of the display.
//
// The setting takes effect when a command is given to change the data being
// displayed.
//
// @param brightness A number from 0 (lowes brightness) to 7 (highest
// brightness)
// @param on Turn display on or off
void tm1637_set_brightness(struct tm1637 *tm1637, uint8_t brightness, bool on);

// Display arbitrary data on the module
//
// This function receives raw segment values as input and displays them. The
// segment data is given as a byte array, each byte corresponding to a single
// digit. Within each byte, bit 0 is segment A, bit 1 is segment B etc. The
// function may either set the entire display or any desirable part on its own.
// The first digit is given by the @ref pos argument with 0 being the leftmost
// digit. The @ref length argument is the number of digits to be set. Other
// digits are not affected.
//
// @param segments An array of size @ref length containing the raw segment
// values
// @param length The number of digits to be modified
// @param pos The position from which to start the modification (0 - leftmost, 3
// - rightmost)
void tm1637_set_segments(struct tm1637 *tm1637, const uint8_t segments[],
                         uint8_t length, uint8_t pos);

// Clear the display
void tm1637_clear(struct tm1637 *tm1637);

// Display a decimal number
//
// Display the given argument as a decimal number.
//
// @param num The number to be shown
// @param leading_zero When true, leading zeros are displayed. Otherwise
// unnecessary digits are
//        blank. NOTE: leading zero is not supported with negative numbers.
// @param length The number of digits to set. The user must ensure that the
// number to be shown
//        fits to the number of digits requested (for example, if two digits are
//        to be displayed, the number must be between 0 to 99)
// @param pos The position of the most significant digit (0 - leftmost, 3 -
// rightmost)
void tm1637_show_number_dec(struct tm1637 *tm1637, int num, bool leading_zero,
                            uint8_t length, uint8_t pos);

// Display a hexadecimal number, with dot control
//
// Display the given argument as a hexadecimal number. The dots between the
// digits (or colon) can be individually controlled.
//
// @param num The number to be shown
// @param dots Dot/Colon enable. The argument is a bitmask, with each bit
// corresponding to a dot
//        between the digits (or colon mark, as implemented by each module).
//        i.e. For displays with dots between each digit:
//        * 0.000 (0b10000000)
//        * 00.00 (0b01000000)
//        * 000.0 (0b00100000)
//        * 0.0.0.0 (0b11100000)
//        For displays with just a colon:
//        * 00:00 (0b01000000)
//        For displays with dots and colons colon:
//        * 0.0:0.0 (0b11100000)
// @param leading_zero When true, leading zeros are displayed. Otherwise
// unnecessary digits are
//        blank
// @param length The number of digits to set. The user must ensure that the
// number to be shown
//        fits to the number of digits requested (for example, if two digits are
//        to be displayed, the number must be between 0 to 99)
// @param pos The position of the most significant digit (0 - leftmost, 3 -
// rightmost)
void tm1637_show_number_hex_ex(struct tm1637 *tm1637, uint16_t num,
                               uint8_t dots, bool leading_zero, uint8_t length,
                               uint8_t pos);

// Translate a single digit into 7 segment code
//
// The method accepts a number between 0 - 15 and converts it to the
// code required to display the number on a 7 segment display.
// Numbers between 10-15 are converted to hexadecimal digits (A-F)
//
// @param digit A number between 0 to 15
// @return A code representing the 7 segment image of the digit (LSB - segment
// A;
//         bit 6 - segment G; bit 7 - always zero)
uint8_t tm1637_encode_digit(uint8_t digit);

// Frees `tm1637` struct
//
// @param tm1637 pointer to `tm1637` struct created by `tm1637_init()`
void tm1637_destroy(struct tm1637 *tm1637);

#endif // TM1637_H
