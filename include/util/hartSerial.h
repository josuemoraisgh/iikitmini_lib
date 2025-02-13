#ifndef HART_SERIAL_H
#define HART_SERIAL_H

class HartSerial {
public:
    HartSerial(uint32_t baudRate = 115200) {
        this->baudRate = baudRate;
    }

    void begin() {
        Serial.begin(baudRate);
        Serial2.begin(1200, SERIAL_8N1, 16, 17,true);
    }

    void update() {
        while (Serial.available()) {
            Serial2.write(Serial.read());
        }
        while (Serial2.available()) {
            Serial.write(Serial2.read());
        }
    }

private:
    uint32_t baudRate;
};

#endif