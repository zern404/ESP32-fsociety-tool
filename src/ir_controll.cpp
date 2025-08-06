#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "config.h"

const uint16_t kIrLed = IR_PIN;
bool irSpam = false;
IRsend irsend(kIrLed);

void irSpamAllProtocols() {
    irsend.sendNEC(0xE0E040BF, 32);
    for (uint32_t code = 0; code < 0xFFFF; code += 0x111) {
    irsend.sendNEC(code, 32);
    delay(5);
    }

    for (uint16_t code = 0; code < 0x0FFF; code++) {
    irsend.sendSony(code, 12);
    delay(5);
    }

    for (uint16_t code = 0; code < 0x07FF; code++) {
    irsend.sendRC5(code, 13);
    delay(5);
    }

    for (uint32_t code = 0; code < 0x0FFFF; code++) {
    irsend.sendRC6(code, 20);
    delay(5);
    }
    for (uint32_t code = 0x100000; code < 0x100FFF; code++) {
    irsend.sendPanasonic(0x4004, code);
    delay(5);
    }

    for (uint16_t code = 0; code < 0xFFFF; code += 0x101) {
    irsend.sendJVC(code, 16, false);
    delay(5);
    }

    for (uint32_t code = 0xE0E040BF; code < 0xE0E0FFFF; code += 0x1000) {
    irsend.sendSAMSUNG(code, 32);
    delay(5);
    }

    for (uint16_t code = 0x1000; code < 0x1FFF; code++) {
    irsend.sendSharp(code, 15);
    delay(5);
    }

    for (uint16_t code = 0x0000; code < 0x1FFF; code++) {
    irsend.sendDenon(code, 15);
    delay(5);
    }

    for (uint32_t code = 0x20DF10EF; code < 0x20DF50FF; code += 0x1111) {
    irsend.sendLG(code, 28);
    delay(5);
    }

    for (uint32_t code = 0xA0A0A0A0; code < 0xA0A0FFFF; code += 0x1000) {
    irsend.sendWhynter(code, 32);
    delay(5);
    }
}
