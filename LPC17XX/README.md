# Hardware Interfaces of Electronic Devices
# Also known as LPC17xx programming


# Description
This repository contains sources for assignments me and my team were given on the "Hardware Interfaces of Electronic Devices" subject. The goal of this repository is to _preserve_ my code for "the next generation of students" and to make sure I never lose that very code on my USB stick. 

## Libraries
#### `lib_joy` - Joystick library.
Provides a simple interface to interact with the joystick hooked to certain GPIO pins.

#### `lib_lcd` - LCD driver library.
The (almost) full alternative for GLCD library, controls the GLCD and bitmapped fonts.  
SDK fonts from headers also can be supported if the according instance of `bfont` structure is defined that hooks to the font's bitmap buffer.  
Provides stdio-like functions to write data to the screen (`puts`, `printf` and `vprintf`)

#### `lib_lcd_xxx_WxH` - LCD bitmap fonts.
At least two fonts are taken from [spacerace/romfont](https://github.com/spacerace/romfont) and headers serve as an example of implementing a custom font.

#### `lib_led` - LED stack library.
Provides a simple interface to turn LEDS on the board on or off using a single 8-bit integer.

#### `lib_uart` - UART driver library.
Provides a POSIX-like interface for interacting with the `COM2` port on the board (the left port under the LCD screen).
Provides stdio-like functions to write data to the port (`puts`, `printf` and `vprintf`)

## Licensing
* Libraries (`/lib/`) are licensed under WTFPL.
* Assignment sources (`/projects/`) are licensed under WTFPL.
* The lion's part of SDK headers (`/include/`) is licensed under what appears to be BSD 3-Clause.
* Certain SDK headers are licensed under what appears to be Public Domain so keep your eye on comments.
