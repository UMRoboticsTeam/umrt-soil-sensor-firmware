# RS485 soil sensor to CAN bus
Reads temperature, humidity, electrical conductivity(EC) and pH data from the RS485 soil sensor and sends on CAN bus using ESP32 TWAI controller.

## Overview
polls RS485 soil sensor every 1 second before validates data with CRC16 check before accepting and packing into 8-bit CAN frame, then transmit data over CAN using 29-bit extended J1939 identifier

### hardware
* ESP32 WROOM-32 30 pin
* RS485 Soil Sensor
* MAX485 RS485 tranceiver module
* SN65HVD230 CAN 

### Connections and pinouts
* MAX485 RE to GPIO4
* MAX485 DE to GPIO 4
* MAX 485 RO to RX2 (GPIO16)
* MAX 485 DI to TX2 (GPIO17)
* SN65HVD230 CAN TX to GPIO5
* SN65HVD230 CAN RX to GPIO21
- all devices share common ground

## CAN frame format
8 byte message with data scaled x10 as a 16-bit int
- byte 0-1  | temperature (°C ×10)
- byte 2-3  | humidity (%RH x10)
- byte 4-5  | electric conductivity(x10)
- byte 6-7  | pH (x10)




