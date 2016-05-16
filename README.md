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

## Notes on using ST's usb cdc stack

Here is some tips to get up and running with cdc communication with a pc :
* Get a serial terminal like `gtkterm`on your pc
* have a look at the `usb/user/usbd_cdc_if.h` and `usb/user/usbd_cdc_if.c` files, they containt the two main functions you will have to use :
    * `CDC_Receive_FS` this is the function that get called back after receiving a packet from the pc, you should change its content to suit your application needs
    * `CDC_Transmit_FS` this is the function to call when you want to send out something to the pc
these are basically wrappers around the combo :
```C
USBD_CDC_SetTxBuffer(hUsbDevice_0, &buff_TX[0], *Len); // set packet to transmit
USBD_CDC_TransmitPacket(hUsbDevice_0); // transmit packet
```
and for receiving :
```C
USBD_CDC_SetRxBuffer(hUsbDevice_0, &buff_RX[0]); // set receive buffer location
USBD_CDC_ReceivePacket(hUsbDevice_0); // receive packet
```
* Then connect the board to the pc and check if everything is ok with the `dmesg` and `lsusb` commands
* Launch `gtkterm` (with `sudo` if you aren't in the `dialout` group)
* In `Configuration/Port` select the board's port (typically `ttyACM0` or `ttyUSB0`)
* Optionnally you can play around with the `local echo` and `CR LF auto` options in the `Configuration` menu
* You can send bytes by typing on your keyboard or send directly hexadecimal data with the `View/Send hexadecimal data` option
