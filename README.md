qhy5tviewer
================

This project originally by Jwackyto https://github.com/jwackito/qhy5tviewer

On this fork am just updating the code to make it compile in gcc 9 and also added a CMakeLists.txt file so to compile it with cmake


Requirements:
==========
  * cmake and gcc tool chain
  * libusb-compat (or equivalent) providing usb.h
  * sdl\_image
  * cfitsio

Get the code:
===========
Just clone it:
  
   git clone https://github.com/monticellifernando/qhy5tviewer.git



Compile it:
=========

```
cd qhy5tviewer
./build.sh
```

It was easy right?. Now you need to install few things. I.e. those in etc here, just copy them in your system (under /etc/) it should just work. To load the firmware it needs fxload tool

