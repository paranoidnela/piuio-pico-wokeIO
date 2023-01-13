# piuio-pico (brokeIO edition)

This is a Pump it Up IO board (PIUIO) clone based on the RP2040 microcontroller.

This has been adapted for the RP2040 variant of the brokeIO, **B**edrock's **r**eplacement for **o**bsolete **K**orean hardwar**e IO**.

## Firmware Setup
[See the Pi Pico SDK repo](https://github.com/raspberrypi/pico-sdk) for instructions on preparing the build environment. All the required CMake files should already be present and ready for compilation.

General overview:
 - Add the SDK via your method of choice (see the pico-sdk repo, though you may be able to skip this step)
 - Run `git submodule update --init` inside the SDK directory to set up the other required libraries (namely tinyusb)
 - Create the "build" folder inside the "piuio-pico" folder.
 - Inside the build folder, run `cmake ..`, then run `` make -j`nproc` ``. You'll get a .UF2 file in the build folder that you can upload to the Pico!
 - With the brokeIO unplugged from the PC, hold the "BOOT" button on the brokeIO, then plug in the brokeIO or hit the "RESET" button.
 - The brokeIO will now show up as a "flash drive" in your OS. Copy the .UF2 file onto the flash drive, and the code will be uploaded!


## PIU Online Notice
Using a hand controller with games connected to the official Pump it Up online service is considered cheating by Andamiro and may lead to actions being taken against your account.
As such, this use case is not officially endorsed by piuio-pico.


## Credits
This project is based off of the [tinyusb device USB examples](https://github.com/hathach/tinyusb/tree/master/examples/device) (specifically webusb_serial and hid_generic_input)
In addition, the st7789_lcd example from Raspberry Pi's [pico-examples](https://github.com/raspberrypi/pico-examples/tree/master/pio/st7789_lcd) repository were used for software SPI.

Protocol information from the [PIUIO_Arduino](https://github.com/ckdur/PIUIO_Arduino/) and [piuio_clone](https://github.com/racerxdl/piuio_clone/) repositories.
A few short snippets of code were also derived from piuio_clone, which is under the GPLv3 license but express permission has been granted by racerxdl to relicense these snippets under BSD-3.