#ifndef RFID_CONTROLL_H
#define RFID_CONTROLL_H

#include <MFRC522.h>

#define SS_PIN 5  
#define RST_PIN 12 
#define SCK_PIN 15 
#define MISO_PIN 4
#define MOSI_PIN 13 

extern MFRC522 mfrc522;

void waitForCardAndHandle();

#endif