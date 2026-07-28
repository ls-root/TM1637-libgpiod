#include "tm1637.h"
#include <unistd.h>

// Module connection pins (Digital Pins)
#define CLK 20
#define DIO 21

// The amount of time (in seconds) between tests
#define TEST_DELAY 2

const uint8_t SEG_DONE[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,         // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // O
    SEG_C | SEG_E | SEG_G,                         // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G          // E
};

int main() {
  struct tm1637 *dev =
      tm1637_init("/dev/gpiochip0", CLK, DIO, DEFAULT_BIT_DELAY);
  int k;
  uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
  uint8_t blank[] = {0x00, 0x00, 0x00, 0x00};
  tm1637_set_brightness(dev, 0x0f, true);

  // All segments on
  tm1637_set_segments(dev, data, 4, 0);
  sleep(TEST_DELAY);

  // Selectively set different digits
  data[0] = tm1637_encode_digit(0);
  data[1] = tm1637_encode_digit(1);
  data[2] = tm1637_encode_digit(2);
  data[3] = tm1637_encode_digit(3);
  tm1637_set_segments(dev, data, 4, 0);
  sleep(TEST_DELAY);

  /*
  for(k = 3; k >= 0; k--) {
        display.setSegments(data, 1, k);
        delay(TEST_DELAY);
        }
  */

  tm1637_clear(dev);
  tm1637_set_segments(dev, data + 2, 2, 2);
  sleep(TEST_DELAY);

  tm1637_clear(dev);
  tm1637_set_segments(dev, data + 2, 2, 1);
  sleep(TEST_DELAY);

  tm1637_clear(dev);
  tm1637_set_segments(dev, data + 1, 3, 1);
  sleep(TEST_DELAY);

  // Show decimal numbers with/without leading zeros
  tm1637_show_number_dec(dev, 0, false, 4, 0); // Expect: ___0
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, 0, true, 4, 0); // Expect: 0000
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, 1, false, 4, 0); // Expect: ___1
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, 1, true, 4, 0); // Expect: 0001
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, 301, false, 4, 0); // Expect: _301
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, 301, true, 4, 0); // Expect: 0301
  sleep(TEST_DELAY);
  tm1637_clear(dev);
  tm1637_show_number_dec(dev, 14, false, 2, 1); // Expect: _14_
  sleep(TEST_DELAY);
  tm1637_clear(dev);
  tm1637_show_number_dec(dev, 4, true, 2, 2); // Expect: __04
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, -1, false, 4, 0); // Expect: __-1
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, -12, false, 4, 0); // Expect: _-12
  sleep(TEST_DELAY);
  tm1637_show_number_dec(dev, -999, false, 4, 0); // Expect: -999
  sleep(TEST_DELAY);
  tm1637_clear(dev);
  tm1637_show_number_dec(dev, -5, false, 3, 0); // Expect: _-5_
  sleep(TEST_DELAY);
  tm1637_show_number_hex_ex(dev, 0xf1af, 0, false, 4, 0); // Expect: f1Af
  sleep(TEST_DELAY);
  tm1637_show_number_hex_ex(dev, 0x2c, 0, false, 4, 0); // Expect: __2C
  sleep(TEST_DELAY);
  tm1637_show_number_hex_ex(dev, 0xd1, 0, true, 4, 0); // Expect: 00d1
  sleep(TEST_DELAY);
  tm1637_clear(dev);
  tm1637_show_number_hex_ex(dev, 0xd1, 0, true, 2, 0); // Expect: d1__
  sleep(TEST_DELAY);

  // Brightness Test
  for (k = 0; k < 4; k++)
    data[k] = 0xff;
  for (k = 0; k < 7; k++) {
    tm1637_set_brightness(dev, k, true);
    tm1637_set_segments(dev, data, 4, 0);
    sleep(TEST_DELAY);
  }

  // On/Off test
  for (k = 0; k < 4; k++) {
    tm1637_set_brightness(dev, 7, false); // Turn off
    tm1637_set_segments(dev, data, 4, 0);
    sleep(TEST_DELAY);
    tm1637_set_brightness(dev, 7, true); // Turn on
    tm1637_set_segments(dev, data, 4, 0);
    sleep(TEST_DELAY);
  }

  // Done!
  tm1637_set_segments(dev, SEG_DONE, 4, 0);

  while (1)
    ;
}
