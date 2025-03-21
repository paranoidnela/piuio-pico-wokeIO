/*******************************************************************************************/
/*  SPDX-License-Identifier: MIT                                                           */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 48productions, therathatter, dj505, sugoku  */
/*  SPDX-FileCopyrightText: Copyright (c) 2019 Ha Thach (tinyusb.org)                      */
/*  https://github.com/sugoku/piuio-pico-brokeIO                                           */
/*******************************************************************************************/

#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

enum
{
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_MOUSE,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_GAMEPAD,
    REPORT_ID_COUNT
};

enum
{
    INPUT_MODE_PIUIO,
    INPUT_MODE_GAMEPAD,
    INPUT_MODE_LXIO,
    INPUT_MODE_KEYBOARD,
    INPUT_MODE_XINPUT,
    INPUT_MODE_SWITCH,
    INPUT_MODE_GAMECUBE,
    //INPUT_MODE_SERIAL, //no clue why this is even here tbh, wouldn't work anyway
    INPUT_MODE_COUNT
};


#endif /* USB_DESCRIPTORS_H_ */