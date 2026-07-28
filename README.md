# TM1637 (C / libgpiod port)

A full C port of [avishorp/TM1637](https://github.com/avishorp/TM1637) (original ESP/Arduino) using libgpiod for GPIO access on Linux platforms (Raspberry Pi and other single-board computers exposing /dev/gpiochip*).

This project ports the original Arduino/ESP API to plain C. All original functionality is implemented; since the original exposed a C++ `TM1637Display` object, the port uses a `tm1637` struct and a set of functions that are the original method names converted to snake_case with a `tm1637_` prefix. Take a look at the header file for more information.

# Usage
- Create a `tm1637` instance with `tm1637_init()` which returns a pointer to a `tm1637` struct.
- Call the ported functions; every original method is available as `tm1637_<method_name_in_snake_case>(tm1637 *dev, ...)`.
- Destroy/cleanup the instance with `tm1637_destroy(tm1637 *dev)`

## Requirements
- Linux with libgpiod (the userland libgpiod library installed)
- C compiler (gcc or clang)
- Permission to access /dev/gpiochip* (run as root or configure udev/group permissions)

Wiring & gpiochip notes
- Connect TM1637 CLK and DIO to two GPIO lines.
- Typical wiring:
  - TM1637 CLK -> GPIO line (example: gpiochip0 line 4)
  - TM1637 DIO -> GPIO line (example: gpiochip0 line 17)
  - VCC -> 3.3V
  - GND -> GND
- libgpiod uses chip names and line offsets (not BCM board numbers). Use:
  - `gpiodetect` to list chips
  - `gpioinfo gpiochip0` to list line offsets

## Building
- Copy `tm1637.c` and `tm1637.h` into your project.
- Example compile line:
  ```bash
  gcc my_project.c tm1637.c -lgpiod -o my_project
  ```
