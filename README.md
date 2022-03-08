| Supported Targets | ESP32 | 
| ----------------- | ----- | 

# SPI ILI9486 ESP_IDF library

This is a library made to control the 3.5 Inch RPI ILI9486 display.
Some functions are self-written and some are ported over from the Adafruit GFX library.


## Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.


## Future plans

- In the future I want to replace the copied code from Adafruit GFX lib with my own
platform optimized functions. 

- Create a c++ version of this lib