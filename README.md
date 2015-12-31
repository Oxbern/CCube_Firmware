CCube_Firmware
==============

Requirements (deprecated)
------------

TODO: redo this section

1. Docker.io (docker + docker-compose)

    https://docs.docker.com/installation/debian/
    https://docs.docker.com/compose/install/

2. eclipse

    http://eclipse.ialto.com/technology/epp/downloads/release/luna/SR2/eclipse-cpp-luna-SR2-linux-gtk-x86_64.tar.gz


3. STCubeMX

    http://www.st.com/web/en/catalog/tools/PF257931#

Setup build environment
------------------------

In order to get build environment just execute the following command:

```bash
$ make tools
```

Generate firmware
-----------------

In order to build the firmware just execute the following command:

```bash
$ make
```

Flashing target:
----------------

In order to flash target board just execute the following command:

```bash
$ make burn
```

Debugging firmware with gdb.
----------------------------

In order to run gdb on target board just execute the following command:

```bash
$ make gdb
```
