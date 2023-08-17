# ECOlibrium Remote Sensor Module (ESP32)

This project comprises two distinct sensor modules designed to monitor environmental conditions in both indoor and outdoor settings. By leveraging various sensors, the modules provide real-time data, ensuring a comprehensive understanding of environmental parameters in both contexts. <br>
The indoor and outdoor modules share similarities in design, but differences exist in the functionality of some sensors.

## Description
### General WiFi/Web Server Functionality
Software for these modules were developed in Arduino IDE, as it is compatible with the ESP32. The modules use the WiFiManager for seamless WiFi connection management. This allows the module to automatically connect to available networks when powered on. For serving web content, ESPAsyncWebServer allows for asynchronous handling of web requests, improving the efficacy of the web interface. The data from the sensors is presented via a user-friendly HTML interface, enabling users to have clear insights into the environmental parameters being monitored.
### Sensors Used for Indoor Module
SCD40: Measures carbon dioxide concentrations, crucial for monitoring indoor air quality and ventilation efficiency.<br>
SEN55: measures particulate matter, humidity, temperature, VOCs (Volatile Organic compounds), and NOx (Nitrogen Oxide) to provide a clear picture of air quality.
### Sensors Used for Outdoor Module
PMS5003: Measures varous sizes of particulate matter. <br>
PMS7003: Measures varous sizes of particulate matter (Interchangeable with the PMS5003). <br>
BME280: Measures temperature, humidity, and barometric pressure, and altitude. <br>
SHT31: Measures temperature and humidty (Interchangeable with the BME280). <br>
SGP41: Measures VOCs and NOx, integrating with the BME280 to obtain essential temperature and humidity data for accurate algorithmic readings. <br>
Detailed explanations of the functionalities and associated libraries will be provided below.

## Getting Started
### Wiring
### SCD40
| ESP32 Pin | Sensor Pin | Description |
|:-:|:-:|:--|
| `3.3V`(BUS) | `Vcc` | The SCD40 runs off of 5 and 3.3 volts, but we chose to use 3.3V |
| `GND`(BUS)| `GND` | Ground |
| `IO22` |  `SCL` | I2C clock line |
| `IO21`| `SDA` | I2C data line |
### SEN55
| ESP32 Pin | Sensor Pin | Description |
|:-:|:-:|:--|
| `5V`(BUS) | `Vcc` | The SEN55 runs off of 5V |
| `GND`(BUS)| `GND` | Ground |
| `IO22` |  `SCL` | I2C clock line |
| `IO21`| `SDA` | I2C data line |

Refer to datasheets for pin numbers and their respective assignments if it is not specified
## Outdoor Module 
![image](https://github.com/Isaacamar/ECOlibrium/assets/113270099/028262cc-c490-4796-8d30-b34dddfed141)
### PMS7003
| ESP32 Pin | Sensor Pin | Description |
|:-:|:-:|:--|
| `5V`(BUS) | `Vcc` | The PMS5003 runs off of 5V. |
| `GND`(BUS)| `GND` | Ground |
| `IO22` |  `SCL` | I2C clock line |
| `IO21`| `SDA` | I2C data line |
### BME280/SGP41
| ESP32 Pin | Sensor Pin | Description |
|:-:|:-:|:--|
| `3v3`(BUS) | `Vcc` | The BME280 runs off of 3.3V. |
| `GND`(BUS)| `GND` | Ground |
| `IO22` |  `SCL` | I2C clock line |
| `IO21`| `SDA` | I2C data line |
| `DNC`| `SDO` | Ignore, Do not connect |
| `DNC`| `CSB` | Ignore, Do not connect |

### Dependencies

We developed the Sensor module in Arduino IDE version 1.8.19, since this version accommodated all of our libraries<br>
# Connecting ESP32 to Arduino IDE

Setting up the ESP32 with the Arduino IDE requires a few steps. This guide will walk you through the process.

## 1. Installing the Arduino IDE

Before you can program the ESP32, you need to install the Arduino IDE.

- Download the Arduino IDE from the [official Arduino website](https://www.arduino.cc/en/Main/Software).

## 2. Setting up the ESP32 Board Support

The Arduino IDE doesn't come with support for the ESP32 by default. You have to add it manually.

### Add the ESP32 Board URL to Arduino IDE

- Open Arduino IDE.
- Go to `File` > `Preferences`.
- In the "Additional Boards Manager URLs" field, enter the following URL:
  `https://dl.espressif.com/dl/package_esp32_index.json`
- Click OK.

### Install ESP32 Board Support

- Go to `Tools` > `Board` > `Boards Manager`.
- In the search bar, type "ESP32".
- Look for "esp32 by Espressif Systems" and click on the Install button.

### Select the ESP32 Dev Module

- After installation, go to `Tools` > `Board`.
- From the list, select `ESP32 Dev Module`.

## 3. Installing Sensirion Drivers (if needed)

If your computer doesn't recognize the ESP32, you might need drivers for the USB-to-Serial chip on the board.

For the ESP32, the common chip is the CP210x from Silicon Labs.

- [Download the drivers](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers) from the Silicon Labs website.

## 4. Verifying the Connection

- Connect the ESP32 to your computer using a micro-USB cable.
- In the Arduino IDE, go to `Tools` > `Port` and select the COM port the ESP32 is connected to. The exact number can vary.
- You can now upload your sketches to the ESP32.

# Installing `.zip` Libraries in Arduino IDE

Some of these libraries might not be available directly via the Library Manager and need to be manually added using the `.zip` format.

## 1. Download Library

- First, download the desired library in `.zip` format onto your computer. 
  > **Note:** For both modules, the `ESPAsyncWebServer.h` library must be downloaded as a `.zip` library.

## 2. Open Arduino IDE

- Launch the Arduino IDE software on your computer.

## 3. Add the Library

- Go to the menu and navigate to: `Sketch` > `Include Library` > `Add .ZIP Library...`.
- A file dialog will open. Navigate to the location where you saved the downloaded `.zip` file.
- Select the `.zip` file and click 'Open'.

## 4. Confirm the Installation

- Once the library is added, restart the Arduino IDE.
- You can verify the installation by going to: `File` > `Examples`. The new library should be listed there.

## 5. Utilize the Library in Your Projects

- When you create or edit sketches, you can include the library by selecting: `Sketch` > `Include Library` > [Your Library Name], or include at the beginning of the code in the format below

### Outdoor Module Libraries
```
#include <WiFiManager.h> 
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <NOxGasIndexAlgorithm.h>
#include <SensirionI2CSgp41.h>
#include <VOCGasIndexAlgorithm.h>
#include <Wire.h>
#include <time.h>
#include "PMS.h"
```
### Indoor Module Libraries
```
#include <WiFiManager.h> 
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <time.h>
```

* Describe any prerequisites, libraries, OS version, etc., needed before installing program.
* ex. Windows 10

### Installing

* How/where to download your program
* Any modifications needed to be made to files/folders

### Executing program

* How to run the program
* Step-by-step bullets
```
code blocks for commands
```

## Help

Any advise for common problems or issues.
```
command to run if program contains helper info
```

## Authors

Contributors names and contact info

Arav Sharma <br>
Isaac Amar <br>
Manhim Liu <br>
Jonas Margono

## Version History

* 0.2
    * Various bug fixes and optimizations
    * See [commit change]() or See [release history]()
* 0.1
    * Initial Release

## License

This project is licensed under the [NAME HERE] License - see the LICENSE.md file for details

## Acknowledgments

Inspiration, code snippets, etc.
 
