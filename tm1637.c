#include "tm1637.h"
#include <gpiod.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define TM1637_I2C_COMM1 0x40
#define TM1637_I2C_COMM2 0xC0
#define TM1637_I2C_COMM3 0x80

//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t tm1637_digit_to_segment[] = {
    // XGFEDCBA
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // b
    0b00111001, // C
    0b01011110, // d
    0b01111001, // E
    0b01110001  // F
};

static const uint8_t minusSegments = 0b01000000;

struct tm1637 *tm1637_init(char *chip_path, uint8_t pin_clk, uint8_t pin_dio,
                           unsigned int bit_delay) {
  struct tm1637 *tm1637 = calloc(1, sizeof(struct tm1637));
  if (!tm1637)
    return NULL;

  tm1637->pin_clk = pin_clk;
  tm1637->pin_dio = pin_dio;
  tm1637->bit_delay = bit_delay;

  struct gpiod_line_settings *settings = gpiod_line_settings_new();
  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
  gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);
  gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
  gpiod_line_settings_set_drive(settings, GPIOD_LINE_DRIVE_OPEN_DRAIN);

  struct gpiod_line_config *line_config = gpiod_line_config_new();
  unsigned int offsets[] = {pin_clk, pin_dio};
  gpiod_line_config_add_line_settings(line_config, offsets, 2, settings);

  struct gpiod_request_config *req_config = gpiod_request_config_new();
  gpiod_request_config_set_consumer(req_config, "tm1637");

  struct gpiod_chip *chip = gpiod_chip_open(chip_path);
  if (!chip) {
    free(tm1637);
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_config);
    gpiod_request_config_free(req_config);
    return NULL;
  }

  tm1637->request = gpiod_chip_request_lines(chip, req_config, line_config);
  if (!chip) {
    free(tm1637);
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_config);
    gpiod_request_config_free(req_config);
    return NULL;
  }

  gpiod_chip_close(chip);
  gpiod_line_settings_free(settings);
  gpiod_line_config_free(line_config);
  gpiod_request_config_free(req_config);

  return tm1637;
}

void tm1637_bit_delay(struct tm1637 *tm1637) { usleep(tm1637->bit_delay); }

void tm1637_start(struct tm1637 *tm1637) {
  gpiod_line_request_set_value(tm1637->request, tm1637->pin_dio,
                               GPIOD_LINE_VALUE_INACTIVE);
  tm1637_bit_delay(tm1637);
}

void tm1637_stop(struct tm1637 *tm1637) {
  gpiod_line_request_set_value(tm1637->request, tm1637->pin_dio,
                               GPIOD_LINE_VALUE_INACTIVE);
  tm1637_bit_delay(tm1637);
  gpiod_line_request_set_value(tm1637->request, tm1637->pin_clk,
                               GPIOD_LINE_VALUE_ACTIVE);
  tm1637_bit_delay(tm1637);
  gpiod_line_request_set_value(tm1637->request, tm1637->pin_dio,
                               GPIOD_LINE_VALUE_ACTIVE);
  tm1637_bit_delay(tm1637);
}

bool tm1637_write_byte(struct tm1637 *tm1637, uint8_t b) {
  uint8_t data = b;

  // 8 Data Bits
  for (uint8_t i = 0; i < 8; i++) {
    // CLK low
    gpiod_line_request_set_value(tm1637->request, tm1637->pin_clk,
                                 GPIOD_LINE_VALUE_INACTIVE);
    tm1637_bit_delay(tm1637);

    if (data & 0x01)
      gpiod_line_request_set_value(tm1637->request, tm1637->pin_dio,
                                   GPIOD_LINE_VALUE_ACTIVE);
    else
      gpiod_line_request_set_value(tm1637->request, tm1637->pin_dio,
                                   GPIOD_LINE_VALUE_INACTIVE);

    tm1637_bit_delay(tm1637);

    // CLK high
    gpiod_line_request_set_value(tm1637->request, tm1637->pin_clk,
                                 GPIOD_LINE_VALUE_ACTIVE);
    tm1637_bit_delay(tm1637);
    data = data >> 1;
  }

  // Wait for acknowledge
  // CLK to zero
  gpiod_line_request_set_value(tm1637->request, tm1637->pin_clk,
                               GPIOD_LINE_VALUE_INACTIVE);

  gpiod_line_request_set_value(tm1637->request, tm1637->pin_dio,
                               GPIOD_LINE_VALUE_ACTIVE);
  tm1637_bit_delay(tm1637);

  // CLK high
  gpiod_line_request_set_value(tm1637->request, tm1637->pin_clk,
                               GPIOD_LINE_VALUE_ACTIVE);
  tm1637_bit_delay(tm1637);

  enum gpiod_line_value ack_val =
      gpiod_line_request_get_value(tm1637->request, tm1637->pin_dio);
  bool ack = (ack_val == GPIOD_LINE_VALUE_INACTIVE);

  gpiod_line_request_set_value(tm1637->request, tm1637->pin_clk,
                               GPIOD_LINE_VALUE_INACTIVE);
  tm1637_bit_delay(tm1637);

  return ack;
}

void tm1637_set_brightness(struct tm1637 *tm1637, uint8_t brightness, bool on) {
  tm1637->brightness = (brightness & 0x7) | (on ? 0x08 : 0x00);
}

void tm1637_set_segments(struct tm1637 *tm1637, const uint8_t segments[],
                         uint8_t length, uint8_t pos) {
  // Write COMM1
  tm1637_start(tm1637);
  tm1637_write_byte(tm1637, TM1637_I2C_COMM1);
  tm1637_stop(tm1637);

  // Write COMM2 + first digit address
  tm1637_start(tm1637);
  tm1637_write_byte(tm1637, TM1637_I2C_COMM2 + (pos & 0x03));

  // Write the data bytes
  for (uint8_t k = 0; k < length; k++)
    tm1637_write_byte(tm1637, segments[k]);

  tm1637_stop(tm1637);

  // Write COMM3 + brightness
  tm1637_start(tm1637);
  tm1637_write_byte(tm1637, TM1637_I2C_COMM3 + (tm1637->brightness & 0x0f));
  tm1637_stop(tm1637);
}

void tm1637_clear(struct tm1637 *tm1637) {
  uint8_t data[] = {0, 0, 0, 0};
  tm1637_set_segments(tm1637, data, 4, 0);
}

uint8_t tm1637_encode_digit(uint8_t digit) {
  return tm1637_digit_to_segment[digit & 0x0f];
}

void tm1637_show_dots(uint8_t dots, uint8_t *digits) {
  for (int i = 0; i < 4; ++i) {
    digits[i] |= (dots & 0x80);
    dots <<= 1;
  }
}

void tm1637_show_number_base_ex(struct tm1637 *tm1637, int8_t base,
                                uint16_t num, uint8_t dots, bool leading_zero,
                                uint8_t length, uint8_t pos) {
  bool negative = false;
  if (base < 0) {
    base = -base;
    negative = true;
  }

  uint8_t digits[4];

  if (num == 0 && !leading_zero) {
    // Singular case - take care separately
    for (uint8_t i = 0; i < (length - 1); i++)
      digits[i] = 0;
    digits[length - 1] = tm1637_encode_digit(0);
  } else {
    // uint8_t i = length-1;
    // if (negative) {
    //	// Negative number, show the minus sign
    //     digits[i] = minusSegments;
    //	i--;
    // }

    for (int i = length - 1; i >= 0; --i) {
      uint8_t digit = num % base;

      if (digit == 0 && num == 0 && leading_zero == false)
        // Leading zero is blank
        digits[i] = 0;
      else
        digits[i] = tm1637_encode_digit(digit);

      if (digit == 0 && num == 0 && negative) {
        digits[i] = minusSegments;
        negative = false;
      }

      num /= base;
    }
  }

  if (dots != 0) {
    tm1637_show_dots(dots, digits);
  }

  tm1637_set_segments(tm1637, digits, length, pos);
}

void tm1637_show_number_dec_ex(struct tm1637 *tm1637, int num, uint8_t dots,
                               bool leading_zero, uint8_t length, uint8_t pos) {
  tm1637_show_number_base_ex(tm1637, num < 0 ? -10 : 10, num < 0 ? -num : num,
                             dots, leading_zero, length, pos);
}

void tm1637_show_number_dec(struct tm1637 *tm1637, int num, bool leading_zero,
                            uint8_t length, uint8_t pos) {
  tm1637_show_number_dec_ex(tm1637, num, 0, leading_zero, length, pos);
}

void tm1637_show_number_hex_ex(struct tm1637 *tm1637, uint16_t num,
                               uint8_t dots, bool leading_zero, uint8_t length,
                               uint8_t pos) {
  tm1637_show_number_base_ex(tm1637, 16, num, dots, leading_zero, length, pos);
}

void tm1637_destroy(struct tm1637 *tm1637) {
  if (!tm1637)
    return;
  if (tm1637->request)
    gpiod_line_request_release(tm1637->request);
  free(tm1637);
}
