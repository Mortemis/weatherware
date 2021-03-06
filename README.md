# Weatherware

## STM32 weather station

**********************************************************

Used modules:

Module        | Description 			      	  | Interface 
--------------|-------------------------|-----------
ENC28J60      | Ethernet                | SPI1		   
DHT11         | Relative humidity & temp| GPIO B12	 
DS18B20       | Temperature 				        | GPIO B13	 
GY-BME280     | Atmosphere pressure & temp | I2C1		   
Photoresistor | Lighting				            | GPIO A0 
Buzzer        | Sounds                              | GPIO B6   
### Chip pinout:
![Chip pinout image](docs/chip.jpg)

### Circuit diagram:
![Circuit image](docs/circuit.jpg)

ENC28J60 SPI Connection:

Pin name | MCU pin | Description
---|---|---
3.3 | 3.3 | Power input
GND | GND | Ground
SI  | A7  | MOSI
SO  | A6  | MISO
SCK | A5  | Clock
CS  | A4  | Chip select 
INT | A2  | Interrupts