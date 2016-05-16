CCube_Firmware
==============

Requirements
------------

1. ARM GCC

```bash
sudo apt-get install gcc-arm-none-eabi
```

2. st-link utility

Project can be found here: https://github.com/texane/stlink

Here is a list of commands which should download and compile st-link:

```bash
git clone https://github.com/texane/stlink.git
cd stlink
mkdir build && cd build
sudo apt-get install build-essential pkg-config intltool cmake libusb-1.0.0-dev
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

3. (Optional) STCubeMX to edit the project configuration

http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html

Generate firmware
-----------------

In order to build the firmware just execute the following command:

```bash
make
```

Caution: you need to create a `build` directory first.

Flash target
------------

In order to flash target board just execute the following command:

```bash
make burn
```

Caution: you need to edit the STLINK variable in the makefile first.

Debug firmware with gdb
-----------------------

In order to run gdb on target board just execute the following command:

```bash
make gdb
```
