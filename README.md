# Smart Linear Amplifier Controller

The idea behind this project is to create a "Smart" Linear Amplifier Controller to allow an Amateur "Ham" Transceiver to control an external amplifier with minimum or no manual intervention, this includes but not limited to: amplifier selection (HF/VHF), band bass filter selection, TX/RX switching.

DISCLAIMER 1: This is a work in progress with very limited test in real world.

DISCLAIMER 2: I'm not a C/C++ programmer. Although I tried to keep the code well organized for easy maintenance, probably there are tons of errors and bad practices. This is my first MCU based project and was used as a learning lab.

## Compatible transceivers

Initially this project is aimed at QRP transceivers with single RF port, and the ICOM IC-705 with Bluetooth CI-V port is used as the test rig. Other transceivers can be added in the future.

## Features

So far the following features are "working" (at least in the protoboard)

 - ICOM CI-V communication (frequency, mode and TX/RX status (PTT))
 - 480x320 TFT display using the excellent [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) Library (fast updates, smooth font rendering)
 - Temperature and FAN sensor/controller
 - Input and Output RF sensors
 - CAN BUS Power Supply support/control (Eltek and Huawei PSUs)
 - Amp selection (VHF/UHF), TX/RX, and LPF selection GPIOs

## User Interface

Tons on nice DIY amplifiers borning nowadays are still based on 70's 16x4 dot matrix displays, mostly because User Interface and graphics are a time consuming task and are usually left aside to focus on the RF stuff. 

The goal of this project is that it could be used as a platform for any kind on amplifier and you could focus on the electronics and easily adapt this code to suit your needs. 

The ESP32 SPI is capable of doing smooth font rendering (anti-aliased) and fast screen updates giving the same user experience of a known-brand transceiver.

![TFT Screen Capture](https://raw.githubusercontent.com/aldoir/smart-linear-controller/main/assets/screen-capture.png)

## Hardware used

This project uses the ESP32 processor. The current [PlatformIO.ini](platformio.ini) settings works with the WT32-SC01 LCD/Touch/ESP32 board available on the AliExpress. Other boards can be easily added as new env section.

## Development IDE

[PlatformIO](https://platformio.org/) was chosen as the IDE because it uses Visual Studio Code and allow a better code and dependency management. If this is your first time, download VS Code and install the PlatformIO extension.

## License

This code can be used for personal and non-commercial projects for free. If you profit from it in any form you must ask for a commercial license.

A visible credit to the original developer (PU5VEN) is always welcome.

## Author

You can reach me at my QRZ page [PU5VEN](https://www.qrz.com/db/pu5ven) or open an issue