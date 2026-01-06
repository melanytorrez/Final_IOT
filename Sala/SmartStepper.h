#ifndef SMART_STEPPER_H
#define SMART_STEPPER_H

#include <Stepper.h>

class SmartStepper {
private:
    Stepper _stepper;
    String _state; // "OPEN", "CLOSED"
    int _stepsForFullOpen;
    int _speed;

public:
    SmartStepper(int stepsPerRev, int in1, int in3, int in2, int in4) 
        : _stepper(stepsPerRev, in1, in2, in3, in4) { // Note sequence 1-3-2-4 for Stepper lib with ULN2003
        _state = "CLOSED";
        _stepsForFullOpen = stepsPerRev / 4; // 90 degrees approx (adjust as needed)
        _speed = 10;
        _stepper.setSpeed(_speed);
    }

    void setSpeed(int speed) {
        _speed = speed;
        _stepper.setSpeed(_speed);
    }

    void open() {
        if (_state != "OPEN") {
            Serial.println("[Stepper] Opening window...");
            _stepper.step(_stepsForFullOpen);
            _state = "OPEN";
            Serial.println("[Stepper] Window OPEN.");
        }
    }

    void close() {
        if (_state != "CLOSED") {
            Serial.println("[Stepper] Closing window...");
            _stepper.step(-_stepsForFullOpen);
            _state = "CLOSED";
            Serial.println("[Stepper] Window CLOSED.");
        }
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
