#include "config.h"
#include "snake_game.h"

struct Point {
  int x;
  int y;
};

int snakeGameMenu(int score) {
  int option = 0;
  while (true) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 5);
    display.print("Game Over");

    display.setTextSize(1);
    display.setCursor(10, 30);
    display.print("Score: ");
    display.print(score);

    display.setCursor(10, 45);
    if (option == 0) display.print("> Continue");
    else display.print("  Continue");

    display.setCursor(10, 55);
    if (option == 1) display.print("> Exit");
    else display.print("  Exit");

    display.display();

    if (!digitalRead(BTN_UP_PIN)) {
      option = 0;
      delay(200);
    } else if (!digitalRead(BTN_DOWN_PIN)) {
      option = 1;
      delay(200);
    } else if (!digitalRead(BTN_SELECT_PIN)) {
      delay(300);
      return option;
    }
  }
}

void startSnakeGame() {
  delay(300);
  while (true) {
    Point snake[MAX_SNAKE_LENGTH];
    int snake_length = 5;
    int dir = 0; 
    Point food;
    bool running = true;
    int score = 0;

    unsigned long lastMove = millis();
    const int moveDelay = 120; 

    for (int i = 0; i < snake_length; i++) {
      snake[i].x = (SCREEN_WIDTH/2) / SNAKE_BLOCK - i;
      snake[i].y = (SCREEN_HEIGHT/2) / SNAKE_BLOCK;
    }

    food.x = random(0, SCREEN_WIDTH / SNAKE_BLOCK);
    food.y = random(0, SCREEN_HEIGHT / SNAKE_BLOCK);

    while (running) {
      if (!digitalRead(BTN_UP_PIN)) { 
        dir = (dir + 3) % 4; 
        delay(150);
      } 
      else if (!digitalRead(BTN_DOWN_PIN)) {
        dir = (dir + 1) % 4;
        delay(150);
      } 
      else if (!digitalRead(BTN_SELECT_PIN)) {
        dir = (dir + 1) % 4;
        delay(150);
      }

      if (millis() - lastMove >= moveDelay) {
        lastMove = millis();

        int dirX = 0, dirY = 0;
        if (dir == 0) { dirX = 1; dirY = 0; } 
        if (dir == 1) { dirX = 0; dirY = 1; }  
        if (dir == 2) { dirX = -1; dirY = 0; }  
        if (dir == 3) { dirX = 0; dirY = -1; }  

        for (int i = snake_length-1; i > 0; i--) {
          snake[i] = snake[i-1];
        }
        snake[0].x += dirX;
        snake[0].y += dirY;

        if (snake[0].x < 0 || snake[0].y < 0 || snake[0].x >= SCREEN_WIDTH/SNAKE_BLOCK || snake[0].y >= SCREEN_HEIGHT/SNAKE_BLOCK) {
          running = false;
        }

        for (int i = 1; i < snake_length; i++) {
          if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            running = false;
          }
        }

        if (snake[0].x == food.x && snake[0].y == food.y) {
          if (snake_length < MAX_SNAKE_LENGTH) snake_length++;
          score++;
          food.x = random(0, SCREEN_WIDTH / SNAKE_BLOCK);
          food.y = random(0, SCREEN_HEIGHT / SNAKE_BLOCK);
        }

        display.clearDisplay();
        display.fillRect(food.x * SNAKE_BLOCK, food.y * SNAKE_BLOCK, SNAKE_BLOCK, SNAKE_BLOCK, WHITE);

        for (int i = 0; i < snake_length; i++) {
          display.fillRect(snake[i].x * SNAKE_BLOCK, snake[i].y * SNAKE_BLOCK, SNAKE_BLOCK, SNAKE_BLOCK, WHITE);
        }

        display.display();
      }
    }

    int choice = snakeGameMenu(score);
    if (choice == 1) break;
  }
}