# esp8266
Experiments on esp8266 "Thing" card by sparkfun,

# Hello world

The hookup tutorial on sparkfun introduces the Arduino firmware and 
either data upload to the web as client or serving an API as Wifi access-point.

https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide

I've used the access-point code as a base for experiments, 
but for a sensor data uploading makes more sense and for a controller 
we should hope for a future Wifi-direct P2P firmware for the ESP8266.

# Temperature measurement using DS1820

I adapted the code from this guide
http://iot-playground.com/2-uncategorised/41-esp8266-ds18b20-temperature-sensor-arduino-ide

It uses the Arduino library DallasTemperature from this repository
https://github.com/milesburton/Arduino-Temperature-Control-Library

GPIO 2 is the same as SDA on the I2C port.

# Temperature measurement using TMP102

The sparkfun board actually has a spot for the TMP102 chip ready for soldering on.

http://www.allthethings.nz/tmp102-on-sparkfun-esp8266-thing/

# Controlling WS2812 / NeoPixel led strips

With just one GPIO you can control a bunch of RGB LED's.

https://www.noisebridge.net/wiki/ESP8266/WS2812

I installed the recommended UART version of NeoPixelBus.

https://github.com/Makuna/NeoPixelBus/tree/UartDriven 

# Using HD motor as rotary input

An LM358 and a couple of resistors will make a usable digital input signal:
https://shanteacontrols.wordpress.com/2015/02/24/building-ceylon/

Powered by 3.3V the output will not be more than about 2V which is low but workable.

Connected to two coils of the harddrive motor the two inputs
will go low in sequence when the motor is turned and the
order will tell you the rotation direction.










