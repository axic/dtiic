DTIIC
=====

This is an Arduino project implementing the same I2C-UART bridge as
Devantech's USB-ISS (http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm).

It can also be made compatible with the older USB-I2C product
(http://www.robot-electronics.co.uk/htm/usb_i2c_tech.htm).

The USB-ISS module is capable of I2C, SPI, UART and GPIO, but we
are limiting the functionality to I2C only.  It is aimed to be
a cheap replacement, especially if someone has an Arduino
compatible board at his/her disposal.

Note: this is a hobbyist project and not a real product.  If you are looking for using in production environment, go with Devantech's products.

Features
========

* It only supports the I2C part of the protocol.  Dummy serial and GPIO
methods are present, because the mode selector for I2C suggsets one of
them must be available to the end user.

* Supports only a hardware I2C implementation, but accepts the SW I2C mode selectors as well. All I2C speeds except 1 MHz are implemented.

* This sketch was tested on an Arduino Nano with the WCH CH340G USB UART.

* Sending direct I2C requests (the I2C_DIRECT command) is not supported.

* Turning on OLD_API aims to make this sketch compatible with the older
USB-I2C product from Devantech (http://www.robot-electronics.co.uk/htm/usb_i2c_tech.htm).

* Uses the serial port on Arduino, therefore no debug console is available.

* The D13 LED is on while processing happens.

* The sketch has been tested with the USB-ISS and USB I2C IO tools from Devantech.


Usage
=====

* Choose the UART speed by changing the following line in the sketch:

```#define UART_SPEED 192000```

All Devantech's applications seem to use this speed.  There is no support to change this speed during communication.

* Choose between the old and the new protocol by either enabling or disabling the following line:

```#define OLD_API 1```

* Compile and load a sketch to an Arduino.

* Connect the I2C, GND and 5V to your slave devices. Read more information about how to connect 3.3V devices here: http://playground.arduino.cc/Main/I2CBi-directionalLevelShifter

* Make sure the Arduino IDE is closed and is not claiming the serial port.


License
=======

    Copyright (C) 2015 Alex Beregszaszi

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
