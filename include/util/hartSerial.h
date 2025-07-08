#ifndef HART_SERIAL_H
#define HART_SERIAL_H

#include <Arduino.h>
class HartSerial {
public:
    void begin() {
        Serial.begin(1200, SERIAL_8O1);           // Serial USB (PACTware) — 1200 8O1
        hartSerial.begin(1200, SERIAL_8O1, 16, 17); // UART2 (Modem HART) — 1200 8O1, TX=17, RX=16      
    }

    void update() {
        // PACTware → HART Modem
        while (Serial.available()) {
            hartSerial.write(Serial.read());
        }
        // HART Modem → PACTware
        while (hartSerial.available()) {
            Serial.write(hartSerial.read());
        }
    }

private:
    HardwareSerial &hartSerial = Serial2;
    uint32_t baudRate;
};

#endif