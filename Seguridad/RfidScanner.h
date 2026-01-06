#ifndef RFID_SCANNER_H
#define RFID_SCANNER_H

#include <SPI.h>
#include <MFRC522.h>

class RfidScanner {
private:
    MFRC522 _mfrc522;
    int _ssPin;
    int _rstPin;

public:
    RfidScanner(int ssPin, int rstPin) : _mfrc522(ssPin, rstPin), _ssPin(ssPin), _rstPin(rstPin) {}

    void begin() {
        SPI.begin();
        _mfrc522.PCD_Init();
        Serial.println("RFID Scanner Initialized.");
    }

    // Checks if a card is present and reads it. Returns true if read.
    bool scanCard(String &uidOut) {
        // 1. Check if new card is present
        if (!_mfrc522.PICC_IsNewCardPresent()) {
            return false;
        }
        // 2. verify if NUID has been readed
        if (!_mfrc522.PICC_ReadCardSerial()) {
            return false;
        }

        // 3. Format UID
        uidOut = "";
        for (byte i = 0; i < _mfrc522.uid.size; i++) {
            char hex[3];
            sprintf(hex, "%02X", _mfrc522.uid.uidByte[i]);
            uidOut += hex;
        }

        // 4. Halt PICC
        _mfrc522.PICC_HaltA();
        return true;
    }
};

#endif
