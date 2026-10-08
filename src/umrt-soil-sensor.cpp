//
// created by Josh Shim on 2026-07-30.
//

#include <Arduino.h>
#include "driver/twai.h"
#include "umrt-soil-sensor.hpp"

// Constructor
SoilSensor::SoilSensor(uint8_t de_pin, uint8_t serial_tx_pin, uint8_t serial_rx_pin)
  : serial_de_pin(de_pin), serial_tx_pin(serial_tx_pin), serial_rx_pin(serial_rx_pin),
    tem(0), hum(0), ph(0), ec(0) {
}

void SoilSensor::begin() {
  Serial2.begin(9600, SERIAL_8N1, serial_rx_pin, serial_tx_pin);
  if (serial_de_pin >= 0) {
    pinMode(serial_de_pin, OUTPUT);
    digitalWrite(serial_de_pin, RS485_RXMODE);
  }
}

SoilSensor sensor(RS485_DE_PIN_DEFAULT, 17, 16); // dePin, serialTxPin(GPIO17), serialRxPin(GPIO16)

void sendCANFrame();
void printTWAIStatus();

void setup() {
  Serial.begin(115200);
  sensor.begin();

  //configure and start TWAI driver
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    Serial.println("TWAI driver install failed");
    return;
  }
  if (twai_start() != ESP_OK) {
    Serial.println("TWAI start failed");
    return;
  }
  Serial.println("TWAI started");
}

void loop() {
  sensor.readSensorData(); //read sensor data from RS485

  Serial.print("TEM = "); Serial.print(sensor.getTemperature(), 1);
  Serial.print(" HUM = "); Serial.print(sensor.getHumidity(), 1);
  Serial.print(" EC = "); Serial.print(sensor.getEC());
  Serial.print(" PH = "); Serial.println(sensor.getPH(), 1);

  sendCANFrame(); //pack and send CAN frame with sensor readings

  printTWAIStatus(); // shows error counters and error state for frame

  delay(1000);
}

void sendCANFrame() {

  int16_t tem_i = (int16_t)(sensor.getTemperature() * 10);
  int16_t hum_i = (int16_t)(sensor.getHumidity() * 10);
  int16_t ec_i  = (int16_t)(sensor.getEC()  * 10);
  int16_t ph_i  = (int16_t)(sensor.getPH()  * 10);

  twai_message_t message;
  message.identifier = CAN_ID;
  message.extd = 1;
  message.rtr = 0;
  message.data_length_code = 8;

  //little-endian
  //pack CAN frame data
  message.data[0] = tem_i & 0xFF;
  message.data[1] = (tem_i >> 8) & 0xFF;
  message.data[2] = hum_i & 0xFF;
  message.data[3] = (hum_i >> 8) & 0xFF;
  message.data[4] = ec_i & 0xFF;
  message.data[5] = (ec_i >> 8) & 0xFF;
  message.data[6] = ph_i & 0xFF;
  message.data[7] = (ph_i >> 8) & 0xFF;

  esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(100));
  if (result == ESP_OK) {
    Serial.println("CAN frame sent OK");
  } else {
    Serial.printf("CAN transmit FAILED, error code: %d\n", result);
  }
}

void printTWAIStatus() {
  //print current TWAI state and error counts
  twai_status_info_t status;
  twai_get_status_info(&status);
  Serial.printf("TWAI state: %d  TX errors: %d  RX errors: %d  TX queue: %d  bus errors: %d\n",
                status.state, status.tx_error_counter, status.rx_error_counter,
                status.msgs_to_tx, status.bus_error_count);
}

void SoilSensor::readSensorData(void) {
  uint8_t Data[13] = { 0 };
  bool flag = true;

  while (flag) { //loop until valid response received
    delay(100);
    if (serial_de_pin >= 0) digitalWrite(serial_de_pin, RS485_TXMODE);
    Serial2.write(Com, 8);
    Serial2.flush();
    if (serial_de_pin >= 0) digitalWrite(serial_de_pin, RS485_RXMODE);

    delay(50);
    uint8_t n = readN(Data, 13);

    if (n == 13 && CRC16_2(Data, 11) == (Data[11] * 256 + Data[12])) {
      hum = (Data[3] * 256 + Data[4]) / 10.00;
      tem = (Data[5] * 256 + Data[6]) / 10.00;
      ec  = Data[7] * 256 + Data[8];
      ph  = (Data[9] * 256 + Data[10]) / 10.00;
      flag = false;
    }
    while (Serial2.available()) Serial2.read();
  }
}

//helper function to read a specified number of bytes from Serial2 with a timeout
uint8_t SoilSensor::readN(uint8_t *buf, size_t len) {
  size_t offset = 0, left = len;
  long curr = millis();
  while (left) {
    if (Serial2.available()) {
      buf[offset++] = Serial2.read();
      left--;
    }
    if (millis() - curr > 500) break;
  }
  return offset;
}

//process received data and calculate CRC16
unsigned int SoilSensor::CRC16_2(unsigned char *buf, int len) {
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (unsigned int)buf[pos];
    for (int i = 8; i != 0; i--) {
      if ((crc & 0x0001) != 0) { crc >>= 1; crc ^= 0xA001; }
      else crc >>= 1;
    }
  }
  return ((crc & 0x00ff) << 8) | ((crc & 0xff00) >> 8);
}