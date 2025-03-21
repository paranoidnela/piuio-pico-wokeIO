# wokeIO

This is a user guide for the wokeIO, **W**orse (than) **o**bsolete **K**orean hardwar**e IO**.  

## Introduction

brokeIO is a replacement for the controller in aftermarket pads, that emulates the I/O board used in Pump It Up machines.

This project is *not* a replacement for the PIUIO or LXIO, if you have a real cabinet use brokeIO instead.

wokeIO is intended as the middleground between swapping your crusty LTEK with a piuio-pico and sticking to even crustier stepmania atrocities, which is why wokeIO is capable of emulating several kinds of USB devices, such as:

- PIUIO
- Gamepad (DInput)
- LXIO
- Keyboard
- XInput (Xbox 360 Controller)
- Nintendo Switch
- Nintendo GameCube Adapter

wokeIO much like brokeIO polls at 1000hz, so it's theoretically nicer than boards found in pre LXIO cabs, debouncing is not yet supported because I am incredibly lazy, I'll get to it sometime.

Currently, most options will require you to modify the code as the config system brokeIO created has been messed up by yours truly, once again, if I can be bothered I'll fix it.

NOTE: I completely got rid of all of the lighting code because I don't really need it and I don't see a reason to implement lighting support on something intended to go in third party pads.

## Installation

check the code for a pinout, it should more or less match the `piuio-pico` pinout, lights excluded  
solder your grounds to ground
solder your sensor wires to each input pin
figure out a usb solutions and you're pretty much good

## Configuration

There is a config mode built into the brokeIO which allows you to switch between different USB modes. To enter it,
hold the service button for two seconds. 

| Number | Lights | Mode |
| --- | --- | --- |
| 0 | --- | PIUIO |
| 1 | --O | Gamepad (DInput) |
| 2 | -O- | LXIO |
| 3 | -OO | Keyboard |
| 4 | O-- | XInput (Xbox 360 Controller) |
| 5 | O-O | Nintendo Switch |
| 6 | OO- | Nintendo GameCube Adapter |

To select a mode, choose with the test button. To exit the config mode, hold the service button for two seconds, and the brokeIO will save your settings and reset.

To update the firmware, while holding the BOOT button, either plug in the brokeIO or press the RESET button if plugged in already. A storage device called RPI-RP2 will show up on the host computer, and you can drag or copy the .UF2 firmware file into the drive, which
will automatically upload and flash the wokeIO.

If you accidentally enter the firmware uploading mode, resetting or unplugging/replugging the wokeIO will exit it.
