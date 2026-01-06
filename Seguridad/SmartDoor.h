#ifndef SMART_DOOR_H
#define SMART_DOOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

class SmartDoor {
private:
    Servo _servo;
    int _pin;
    String _state; // "OPEN" or "CLOSED"
    int _angleOpen;
    int _angleClosed;

public:
    // Constructor
    SmartDoor(int pin, int angleOpen = 90, int angleClosed = 0) {
        _pin = pin;
        _angleOpen = angleOpen;
        _angleClosed = angleClosed;
        _state = "CLOSED";
    }

    // Initialize the servo (must be called in setup)
    void begin() {
        _servo.attach(_pin);
        _servo.write(_angleClosed); // Default to closed
    }

    void open() {
        if (_state != "OPEN") {
            _servo.write(_angleOpen);
            _state = "OPEN";
            Serial.print("Door on pin "); Serial.print(_pin); Serial.println(" -> OPEN");
        }
    }

    void close() {
        if (_state != "CLOSED") {
            _servo.write(_angleClosed);
            _state = "CLOSED";
            Serial.print("Door on pin "); Serial.print(_pin); Serial.println(" -> CLOSED");
        }
    }

    void setSpeed(int speed) {
        // ESP32Servo doesn't support speed control directly like Steppers, 
        // but we can add delay logic here if needed in future refactors.
    }

    // Open temporarily (for RFID access)
    void openMomentarily(int durationMs) {
        open();
        delay(durationMs);
        close();
    }

    void setState(String newState) {
        if (newState == "OPEN") open();
        else if (newState == "CLOSED") close();
    }

    String getState() {
        return _state;
    }
};

#endif
