# Description
This repository contains sources for assignments me and my team were given on the "Hardware Interfaces of Electronic Devices" subject. The goal of this repository is to _preserve_ my code for "the next generation of students" and to make sure I never lose that very code on my USB stick. 

## Libraries
#### Joystick library
A simplified library to provide a polling-based joystick input.

#### Display library
A rewrite of GLCD library that requires you to provide a custom bitmap font. Supports SDK fonts.

#### TODO: UART library
An abstraction layer for UART1 controller designed to simplify things. UART0, UART2 and UART3 support is planned but not critical.

## Licensing
* Libraries (`/lib/`) are licensed under GNU GPL 3.0.
* Assignment sources (`/projects/`) are licensed under GNU GPL 3.0.
* The lion's part of SDK headers (`/include/`) is licensed under what appears to be BSD 3-Clause.
* Certain SDK headers are licensed under what appears to be Public Domain so keep your eye on comments.

# Things that need to be complete
- [ ] Rewrite projects to use _my_ libraries instead of GLCD.
- [ ] Write the UART1 library, possibly add other three UART controllers.
