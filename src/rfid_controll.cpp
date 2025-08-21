#include <SPI.h>
#include <MFRC522.h>
#include "config.h"
#include "rfid_controll.h"

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte savedUid[4];   
bool uidSaved = false;

void waitForCardAndHandle() {
  while (true) {
    if (digitalRead(BTN_SELECT_PIN) == LOW) { 
        delay(100);
        return;
    }

    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(100);
      continue;
    }

    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX); Serial.print(" ");
    }
    Serial.println();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("UP - READ");
    display.println("DOWN - WRITE");
    display.println("SELECT - EXIT");
    display.display();

    if (digitalRead(BTN_SELECT_PIN) == LOW) { 
        return;
    }

    bool actionChosen = false;
    bool doWrite = false;

    while (!actionChosen) {
      if (digitalRead(BTN_SELECT_PIN) == LOW) { 
        delay(100);
        return;
      }
      if (digitalRead(BTN_UP_PIN) == LOW) { 
        doWrite = false;
        actionChosen = true;
      }
      if (digitalRead(BTN_DOWN_PIN) == LOW) { 
        doWrite = true;
        actionChosen = true;
      }
      delay(100);
    }

    if (!doWrite) {
      memcpy(savedUid, mfrc522.uid.uidByte, 4);
      uidSaved = true;
      Serial.println("UID saved!");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("UID saved!");
      display.display();
    } else {
      if (!uidSaved) {
        Serial.println("No saved UID!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("no UID!");
        display.display();
      } else {
        if (mfrc522.MIFARE_SetUid(savedUid, (byte)4, true)) {
          Serial.println("UID writed!");
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("UID writed!");
          display.display();
        } else {
          Serial.println("Error writed UID!");
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Error writed!");
          display.display();
        }
      }
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    delay(1500);
  }
}
