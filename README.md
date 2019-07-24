5ohBEE <img src="50bee.png" width="200px" alt="bee art by https://github.com/ivycraft">
======

Setting up your Board
---------------------
1. udev rules: copy helpers/99-USBasp.rules to /etc/udev/rules.d/ (this assumes you are using the USBasp programmer, update your IDE accordingly)
2. Add the board to Arduino IDE: Unzip helpers/5ohBee.tar.gz in your Arduino sketch directory, restart the IDE, and set your board to "ATmega128RFA1 Dev Board" (under "Sparkfun AVR Boards")
3. Install dependencies: In the IDE, select "Sketch" -> "Include Library" -> "Add .ZIP Library..." to add helpers/SmartResponseXE-lib.zip to the project
4. ???
5. Profit! Add the contents of the sketch directory to your workspace, compile, and upload!

Keymap
------
The keymap is documented on lines 64 - 90 of SmartResponseXE/blob/master/SmartResponseXE.cpp.  Brief hints below:

Most of the ASCII-printable range is mapped.

Function and Special Keys:
    - Left Column: 0xF0 - 0xF4 (Top to bottom)
    - Right Column: 0xF5 - 0xF9 (Top to bottom)
    - Left: 0x02
    - Right: 0x03
    - Up: 0x04
    - Down: 0x05
    - Menu: 0x01

Unmapped Sym Combos:
    - Sym+2
    - Sym+3
    - Sym+6
    - Sym+Z
    - Sym+X
    - Sym+M
    - Sym+N
