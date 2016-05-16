# CCube_Firmware

## Requirements

### 1. ARM GCC cross-compiler, debugger and various packages

```bash
    sudo apt-get install --no-install-recommends --yes \
            build-essential \
            intltool \
            git \
            ca-certificates \
            automake \
            gcc \
            make \
            cmake \
            binutils \
            libc6-dev \
            gcc-arm-none-eabi \
            gdb-arm-none-eabi \
            libnewlib-arm-none-eabi \
            binutils-arm-linux-gnueabi \
            autoconf \
            pkg-config \
            libusb-1.0.0-dev \
            wget
```

### 2. st-link utility

Project can be found here: https://github.com/texane/stlink

Here is a list of commands which should download and compile st-link:

```bash
git clone https://github.com/texane/stlink.git
cd stlink
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### 3. (Optional) STCubeMX to edit the project configuration

http://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-configurators-and-code-generators/stm32cubemx.html

## Build'n Run firmware

Create a `build` folder if you still don't have it in your project tree, set the `stlink` path in your makefile then just :

* `make`      : to compile and link your hex file
* `make burn` : to flash the target board
* `make gdb`  : to debug your program after launching the `st-utils` utility in another shell
