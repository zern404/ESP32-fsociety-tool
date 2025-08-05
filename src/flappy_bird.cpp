#include "config.h"
#include "flappy_bird.h"
#include <Wire.h>

const int fb_birdX = 20;
float fb_birdY;
float fb_velocity;
const float fb_gravity = 0.35;
const float fb_jumpStrength = -2.8;

int fb_pipeX[FB_NUM_PIPES];
int fb_pipeGapY[FB_NUM_PIPES];

int fb_score;
bool fb_gameOver;
unsigned long fb_lastFlapTime = 0;

void fb_resetGame() {
  fb_birdY = SCREEN_HEIGHT / 2;
  fb_velocity = 0;
  fb_score = 0;
  fb_gameOver = false;
  for (int i = 0; i < FB_NUM_PIPES; i++) {
    fb_pipeX[i] = SCREEN_WIDTH + i * (SCREEN_WIDTH / FB_NUM_PIPES);
    fb_pipeGapY[i] = random(10, SCREEN_HEIGHT - FB_GAP_HEIGHT - 10);
  }
}

void fb_drawBird() {
  display.fillCircle(fb_birdX, (int)fb_birdY, 3, SSD1306_WHITE);
}

void fb_drawPipe(int x, int gapY) {
  display.fillRect(x, 0, FB_PIPE_WIDTH, gapY, SSD1306_WHITE);
  display.fillRect(x, gapY + FB_GAP_HEIGHT, FB_PIPE_WIDTH, SCREEN_HEIGHT - (gapY + FB_GAP_HEIGHT), SSD1306_WHITE);
}

void fb_updatePipes() {
  for (int i = 0; i < FB_NUM_PIPES; i++) {
    fb_pipeX[i] -= FB_PIPE_SPEED;
    if (fb_pipeX[i] + FB_PIPE_WIDTH < 0) {
      fb_pipeX[i] = SCREEN_WIDTH;
      fb_pipeGapY[i] = random(10, SCREEN_HEIGHT - FB_GAP_HEIGHT - 10);
      fb_score++;
    }
  }
}

bool fb_checkCollision() {
  if (fb_birdY <= 0 || fb_birdY >= SCREEN_HEIGHT) return true;
  for (int i = 0; i < FB_NUM_PIPES; i++) {
    if (fb_birdX + 3 >= fb_pipeX[i] && fb_birdX - 3 <= fb_pipeX[i] + FB_PIPE_WIDTH) {
      if (fb_birdY - 3 <= fb_pipeGapY[i] || fb_birdY + 3 >= fb_pipeGapY[i] + FB_GAP_HEIGHT) {
        return true;
      }
    }
  }
  return false;
}

void fb_flap() {
  static bool flapHeld = false;
  bool pressed = digitalRead(BTN_DOWN_PIN) == LOW;

  if (pressed && !flapHeld && millis() - fb_lastFlapTime > 150) {
    fb_velocity = fb_jumpStrength;
    fb_lastFlapTime = millis();
    flapHeld = true;
  }

  if (!pressed) {
    flapHeld = false;
  }
}

void runFlappyBird() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  fb_resetGame();

  while (true) {
    fb_flap();

    if (!fb_gameOver) {
      fb_velocity += fb_gravity;
      fb_birdY += fb_velocity;
      fb_updatePipes();
      if (fb_checkCollision()) fb_gameOver = true;
    } else {
      if (digitalRead(BTN_DOWN_PIN) == LOW) {
        delay(300);
        fb_resetGame();
      }
      if (digitalRead(BTN_SELECT_PIN) == LOW) {
        delay(300);
        return;
      }
    }

    display.clearDisplay();

    for (int i = 0; i < FB_NUM_PIPES; i++) {
      fb_drawPipe(fb_pipeX[i], fb_pipeGapY[i]);
    }

    fb_drawBird();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Score: ");
    display.print(fb_score);

    if (fb_gameOver) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Score: ");
        display.print(fb_score);
        display.setTextSize(2);
        display.setCursor(10, 10);
        display.println("Game Over");
        display.setTextSize(1);
        display.setCursor(40, 40);
        display.println("Down=Restart");
        display.println("Select=Exit");
    }

    display.display();
    delay(25);
  }
}
