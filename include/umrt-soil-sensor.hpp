//
// Created by Josh Shim on 2026-07-30.
//
#ifndef UMRT_SOIL_SENSOR_HPP
#define UMRT_SOIL_SENSOR_HPP

#include <Arduino.h>
#include "driver/twai.h"

//RS485
#define RS485_DE_PIN_DEFAULT 4
#define RS485_TXMODE HIGH
#define RS485_RXMODE LOW

//CAN
#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_21
#define CAN_ID     0x18FFEB00

class SoilSensor {
public:
  /** @brief Constructor for the SoilSensor class
   *  @param de_pin RS485 DE / RE pin
   *  @param serial_tx_pin Serial2 TX pin (sensor DI)
   *  @param serial_rx_pin Serial2 RX pin (sensor RO)
   */
  SoilSensor(uint8_t de_pin, uint8_t serial_tx_pin, uint8_t serial_rx_pin);

  void begin();
  void readSensorData();

  float getTemperature() const { return tem; }
  float getHumidity() const { return hum; }
  float getPH() const { return ph; }
  int getEC() const { return ec; }
private:
  uint8_t serial_de_pin;
  uint8_t serial_tx_pin;
  uint8_t serial_rx_pin;
  float tem, hum, ph;
  int ec;
  uint8_t Com[8] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09 };
  uint8_t readN(uint8_t *buf, size_t len);
  unsigned int CRC16_2(unsigned char *buf, int len);
};
#endif //UMRT_SOIL_SENSOR_HPP