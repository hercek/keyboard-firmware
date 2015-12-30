Libusb needs WinUsb driver to properlly access the control endpoint
 of K80CS keyboard on MS Windows. KeyboardClient uses libusb.
 Therefore we need to install WinUsb for K80CS on windows to be able
 configure the keybard from KeyboardClient PC application.

To install the driver follow the instructions on this web page:
 http://www.libusb.org/wiki/winusb_driver_installation
Instead of modifying the inf files just use the prepared inf file
 which is in this directory.

Linux does not need any special driver.
