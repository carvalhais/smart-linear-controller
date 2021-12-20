# Smart Linear Amplifier

The idea behind this project is to create a "Smart" Linear Amplifier Controller to allow an Amateur "Ham" Transceiver to control an extenal amplifier with minimum or no manual intervention, this includes but not limited to: amplifier selelection (HF/VHF), band bass filter selection, TX/RX switching.

DISCLAIMER 1: This is a work in progress and wasn't tested in real world.

DISCLAIMER 2: I'm not a C/C++ programmer. Although I tried to keep the code well organized for easy maintenance, probably there are tons of errors and bad pratices.

## Compatible tranceivers

Initially this project is aimed at QRP transceivers with single RF port, and the ICOM IC-705 with Bluetooth CI-V port is used as the test rig. Other transceivers can be added in the future.

## Features

So far the following features are "working" (at least in the protoboard)

 - ICOM CI-V communication (frequency, mode and TX/RX status (PTT))
 - 320x240 TFT display using the excelent [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) Library (fast updates, smooth font rendering)
 - POWER/SWR bargraphs with peak hold
 - Amp selection (VHF/UHF), TX/RX, and LPF selection GPIOs

## Hardware used

This project uses the ESP32 processor and the ILI9341 TFT display.

## Development IDE

[PlatformIO](https://platformio.org/) was choosen as the IDE because it uses Visual Studio Code and allow a better code and dependency management. If this is your first time, download VS Code and install the PlatformIO extension.

## License

This code can be used for personal and non-commercial projects for free. If you profit from it in any form you must ask for a commercial license.

A visible credit to the original developer (PU5VEN) is always welcome.

## Author

You can reach me at my QRZ page [PU5VEN](https://www.qrz.com/db/pu5ven). or open an issue