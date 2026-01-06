#ifndef SMART_LIGHT_H
#define SMART_LIGHT_H

#include <Arduino.h>

class SmartLight {
private:
    int _pin;
    String _state; // "ON" or "OFF"

public:
    // Constructor
    SmartLight(int pin) {
        _pin = pin;
        _state = "OFF";
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }

    // Turn light ON
    void turnOn() {
        if (_state != "ON") {
            digitalWrite(_pin, HIGH);
            _state = "ON";
            Serial.print("Light on pin "); Serial.print(_pin); Serial.println(" -> ON");
        }
    }

    // Turn light OFF
    void turnOff() {
        if (_state != "OFF") {
            digitalWrite(_pin, LOW);
            _state = "OFF";
            Serial.print("Light on pin "); Serial.print(_pin); Serial.println(" -> OFF");
        }
    }

    // Set state from string (useful for Shadow updates)
    void setState(String newState) {
        if (newState == "ON") {
            turnOn();
        } else if (newState == "OFF") {
            turnOff();
        }
    }

    // Get current state
    String getState() {
        return _state;
    }
};

#endif
