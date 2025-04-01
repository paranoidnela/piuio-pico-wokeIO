# piuio-pico (wokeIO edition)

This is a Pump it Up IO board (PIUIO) clone based on the RP2040 microcontroller.

This has been adapted from the RP2040 variant of the brokeIO, **B**edrock's **r**eplacement for **o**bsolete **K**orean hardwar**e IO** to create a replacement controller for third party mats effectively creating a affordable path into PIU cab ownership. 

wokeIO stands for **W**orse (than) **o**bsolete **K**orean hardwar**e IO**.  

If you are looking to set up your wokeIO, check out the [user guide](docs/USERGUIDE.md).

<!-- 
TODO make a video on this, I'm too lazy
You can find a demo video of the wokeIO in action [here](https://www.youtube.com/watch?v=dQw4w9WgXcQ).
!-->

## wokeIO Features

- Replacement controller board for PIU soft and hard pads
- Multiple USB modes: PIUIO, Gamepad (DInput), LXIO, Keyboard, XInput, Nintendo Switch
- Built-in debouncing
- 1000Hz polling rate (in theory)
- Updatable through USB, no separate tool required

## Disclaimers

Also, using a hand controller with games connected to the official Pump it Up online service is considered cheating by Andamiro and may lead to actions being taken against your account, this may extend to modified pads, use at your own risk.

## Firmware Setup

Steps tested on WSL and thus should work on Linux also:

- Add the SDK via your method of choice (see the [pico-sdk](https://github.com/raspberrypi/pico-sdk) repo, though by default the below steps should download the SDK)
- (If downloading the SDK separately from a Git repository) Run `git submodule update --init` inside the SDK directory to set up the other required libraries (namely tinyusb)
- Create the "build" folder inside the "piuio-pico" folder.
- Inside the build folder, run `cmake ..`, then run `` make -j`nproc` ``. You'll get a .UF2 file in the build folder that you can upload!
- With the brokeIO unplugged from the PC, hold the "BOOT" button on the brokeIO, then plug in the brokeIO (or while plugged in, press the "RESET" button).
- The brokeIO will now show up as a removable drive in your OS (RPI-RP2). Copy the .UF2 file onto the drive, and the code will be uploaded!

The last two steps can be done to upload new firmware to a brokeIO on most devices that can read and write to USB flash drives.

I got it to work on macos installing `cmake` and `arm-none-eabi-gcc` with macports and then  
`git clone https://github.com/paranoidnela/piuio-pico-wokeIO`  
`cd piuio-pico-wokeIO`  
`mkdir build`  
`cd build`
`cmake ..`
`make`
built binary will be in the build directory with filename `piuio_pico.uf2`, you can paste this in your RPI-RP2 and it should work

## Credits

This project is based off of the [tinyusb device USB examples](https://github.com/hathach/tinyusb/tree/master/examples/device) (specifically webusb_serial and hid_generic_input).
To support different device modes, a large amount of code was repurposed from the tinyusb drivers, HID code and descriptors found in [GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE).
In addition, the st7789_lcd example from Raspberry Pi's [pico-examples](https://github.com/raspberrypi/pico-examples/tree/master/pio/st7789_lcd) repository were used for software SPI.

Protocol information from the [PIUIO_Arduino](https://github.com/ckdur/PIUIO_Arduino/) and [piuio_clone](https://github.com/racerxdl/piuio_clone/) repositories.
A few short snippets of code were also derived from piuio_clone, which is under the GPLv3 license but express permission has been granted by racerxdl to relicense these snippets under MIT.
Thank you to DinsFire64 for your reverse engineering work on the LXIO!
Thank you to sugoku for writing the brokeIO code to allow for mode switching

## License Notice

All code is licensed under the MIT License unless otherwise indicated, `usb_hid_keys.h` is under public domain.
