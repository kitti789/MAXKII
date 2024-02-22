#include <Wire.h>
#include <SDM120.h>

#define BAUD_RATE     9600
#define RS485_CONTROL 4
#define TICTAG_SDM_ID 1

SDM TictagSDM(Serial, BAUD_RATE, RS485_CONTROL);
float Current;  // Declare Current here

void setup() {
  Serial.begin(BAUD_RATE);
  TictagSDM.begin(TICTAG_SDM_ID);
}

void loop() {
  byte fpPrecision = 2; // < floating point precision
  SDM::Type type;

  for (int i = 0; i <= (int)SDM::NODE_ID; i++) {
    delay(100);
    type = SDM::VOLTAGE;
    type = (SDM::Type)(int)i;

    String info = "";
    String unit = "";

    switch (type) {
      case SDM::VOLTAGE:
        info = "Voltage: ";
        unit = "V";
        break;
      case SDM::CURRENT:
        info = "Current: ";
        Current = TictagSDM.getValue((SDM::Type)type);
        unit = "A";
        break;
      case SDM::FREQUENCY:
        info = "Frequency: ";
        unit = "Hz";
        break;
      case SDM::TOTAL_ACTIVE_ENERGY:
        info = "Total_active_energy: ";
        // Uncomment the line below if you want to use this value
        // float Total_active_energy = TictagSDM.getValue((SDM::Type)type);
        unit = "KWh";
        break;
      case SDM::BAUDRATE:
        info = "Baud rate: ";
        unit = "";
        break;
      case SDM::NODE_ID:
        info = "Node ID: ";
        unit = "";
        break;
      default:
        info = "N/A";
        unit = "N/A";
        break;
    }

    float value = TictagSDM.getValue((SDM::Type)type);
    Serial.print(info);
    Serial.print(value, fpPrecision);
    Serial.println(unit);
    delay(2000);
  }

  Serial.println("...................................................");

  Serial.println(Current);
  Serial.println(Current);

  // Your SDM120-related functionality here...
}