**The code here after some point is untested, I will ensure it works as soon as I get my hands on the board again**  

# Description
* This repository contains source code for every assignment I've done with [Keil MCB1700](https://keil.com/arm/mcb1700), a Cortex M3 based evaluation board.
* All the code in `lib/` and `/projects` is licensed under the GNU General Public License version so my classmates can slack and re-use the code if they want.
* The `include/` subdirectory contains basic headers _needed_ for remote development without installing Keil SDK and without direct access to the board.
* VSCode's C++ extention is configured to lint the code for ARM32.

#### TO-DO list
- [ ] Set up a separate Keil studio project for all my code.
- [x] Re-write the display library because it's horrible.
- [ ] Include some host-sided code when we will get to UARTs and whatnot.

