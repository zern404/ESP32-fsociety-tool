#ifndef IR_CONTROLL_H
#define IR_CONTROLL_H

#include <IRremoteESP8266.h>
#include <IRsend.h>

extern IRsend irsend;
extern bool irSpam; 

void irSpamAllProtocols();

#endif