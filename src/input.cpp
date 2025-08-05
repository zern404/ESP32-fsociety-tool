#include "config.h"

void start_input(String* password)
{
    short charIndex = 0;
    const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const short charsCount = strlen(chars);
    bool entering = true;

    while (entering) {
        display.clearDisplay();
        drawStatusBar();

        display.setCursor(0, 10);

        display.print("Pass: ");
        display.println(*password + "_");

        display.setCursor(0, 40);
        if (charIndex < charsCount) {
        display.print("Char: ");
        display.print(chars[charIndex]);
        } else if (charIndex == charsCount) {
        display.print("<< [DELETE] >>");
        } else {
        display.print("<< [READY] >>");
        }

        display.display();

        if (digitalRead(BTN_UP_PIN) == LOW) {
        charIndex = (charIndex + 1) % (charsCount + 2);
        delay(150);
        }

        if (digitalRead(BTN_DOWN_PIN) == LOW) {
        charIndex = (charIndex - 1 + charsCount + 2) % (charsCount + 2);
        delay(150);
        }

        if (digitalRead(BTN_SELECT_PIN) == LOW) {
        if (charIndex < charsCount) {
            *password += chars[charIndex];
        } else if (charIndex == charsCount) {
            if (password->length() > 0) {
            password->remove(password->length() - 1);
            }
        } else {
            entering = false; 
        }
        delay(150);
        }
    }
}