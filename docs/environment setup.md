To develop and debug our first project on ESP32 boards, we need to finish several configuration procedures, step by step.

    Toolchain, that is used to compile applications for ESP32
    ESP-IDF, that includes all of necessary APIs for ESP32
    OpenOCD, that could provide on-chip debugging support via JTAG interface
    VSCode, that is a code editor which could develop and debug your code.


Toolchain


Setup Toolchain
https://docs.espressif.com/projects/esp-idf/en/latest/get-started/linux-setup.html


sudo apt-get install gcc git wget make libncurses-dev flex bison gperf python python-pip python-setuptools python-serial python-cryptography python-future python-pyparsing

Toolchain Setup

ESP32 toolchain for Linux is available for download from Espressif website:

    for 64-bit Linux:

    https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz

Get ESP-IDF
https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#get-started-get-esp-idf

