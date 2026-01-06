#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <Arduino.h>

class MotionSensor {
private:
    int _pin;
    String _state; // "DETECTED", "NOT_DETECTED"
    // Using volatile for ISR safety if we used interrupts, 
    // but simple polling is often enough for "presence" logic like this.
    // For this implementation we will use simple polling in the loop provided 
    // it's non-blocking enough or we use the existing ISR pattern.
    
    // Pattern from original code used ISR. Implementing cleaner polling wrapper.
    unsigned long _lastDebounceTime;
    int _lastPinReading;

public:
    MotionSensor(int pin) {
        _pin = pin;
        pinMode(_pin, INPUT_PULLUP);
        _state = "NOT_DETECTED";
        _lastDebounceTime = 0;
        _lastPinReading = LOW;
    }

    // Returns true if state changed
    bool check() {
        int reading = digitalRead(_pin);
        
        // Simple debounce / state change detection
        if (reading != _lastPinReading) {
             _lastDebounceTime = millis();
        }
        _lastPinReading = reading;

        String newState = (reading == HIGH) ? "DETECTED" : "NOT_DETECTED";
        
        if (newState != _state) {
            _state = newState;
            return true; // State changed
        }
        return false;
    }

    String getState() {
        return _state;
    }
};

#endif
