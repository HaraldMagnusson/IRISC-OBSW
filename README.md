# IRICS-OBSW

## Making project
##### Harald Magnusson says:

"Ah, first cmake needs to do its thing, so either run `make gen` in the top dir or go to build/ and run `cmake ..` there. Also if you want to use clang you have to first to `export CC=clang` & `export CXX=clang++`.

After you have done that you can just run `make` like normal.

It will also detect if the `CmakeList.txt` file has changed and regenerate the cmake files if needed."
_- The Man himself._

## Required Libraries

`cfitsio` - library to handle .fit files

`ftd2xx` - drivers for rs422 to usb converter, download from: https://www.ftdichip.com/Drivers/D2XX.htm
