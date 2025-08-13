#include <LedControl.h>

const int CS = 5;
const int CLK = 6;
const int DIN = 4;

const int rightButton = 2;
const int leftButton = 3;

const int display = 0;

volatile int currentColumn = 7;
volatile int currentRow = 7;

volatile unsigned long lastInterruptTime = 0;

volatile int direction = 0;  // 0 = up, 1 = right, 2 = down, 3 = left

volatile int lastDirection = 0;

LedControl lc = LedControl(DIN, CLK, CS, 4);

volatile bool switchState = false;

const int wait = 300;

const int totalLength = 64;
int snakeLength = 1;

struct Position {
  int row;
  int col;
};

const int maxLength = 64;
Position snake[maxLength];
int headIndex = 0;

bool appleVisible = true;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 200;

Position apple;

void setup() {

  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    lc.shutdown(i, false);
    lc.setIntensity(i, 1);
    lc.clearDisplay(i);
  }

  randomSeed(analogRead(0));

  pinMode(rightButton, INPUT);
  pinMode(leftButton, INPUT);

  attachInterrupt(digitalPinToInterrupt(rightButton), rightButtonPressed, RISING);
  attachInterrupt(digitalPinToInterrupt(leftButton), leftButtonPressed, RISING);

  snake[0].row = currentRow;
  snake[0].col = currentColumn;

  lc.setLed(display, currentRow, currentColumn, true);
  getNewApple();
}

void loop() {


  delay(wait);

  if (!switchState) {
    if (direction == 0) {
      moveSnake(0, -1);
    } else if (direction == 1) {
      moveSnake(-1, 0);
    } else if (direction == 2) {
      moveSnake(0, 1);
    } else {
      moveSnake(1, 0);
    }
  } else {
    switchState = false;

    if (lastDirection == 2) {
      if (direction == 3) {
        direction = 1;
      } else if (direction == 1) {
        direction = 3;
      }
    }

    lastDirection = direction;
  }

  if (millis() - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = millis();
    appleVisible = !appleVisible;
    lc.setLed(display, apple.row, apple.col, appleVisible);
  }
}


void rightButtonPressed() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > wait) {
    direction = (direction + 1) % 4;
    switchState = true;
    lastInterruptTime = interruptTime;
  }
}

void leftButtonPressed() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > wait) {
    direction = (direction + 3) % 4;
    switchState = true;
    lastInterruptTime = interruptTime;
  }
}

void moveSnake(int dRow, int dCol) {
  // Calculate new head position
  int newRow = (snake[headIndex].row + dRow + 8) % 8;
  int newCol = (snake[headIndex].col + dCol + 8) % 8;

  for (int i = 0; i < snakeLength; i++) {
    int index = (headIndex + maxLength - i) % maxLength;
    if (snake[index].row == newRow && snake[index].col == newCol) {
      gameOver();
    }
  }

  // Update head
  headIndex = (headIndex + 1) % maxLength;
  snake[headIndex].row = newRow;
  snake[headIndex].col = newCol;

  if (newRow == apple.row && newCol == apple.col) {
    if (snakeLength < maxLength) {
      snakeLength++;
    }
    getNewApple();
  }

  lc.setLed(display, newRow, newCol, true);

  int tailIndex = (headIndex + maxLength - snakeLength) % maxLength;
  lc.setLed(display, snake[tailIndex].row, snake[tailIndex].col, false);
}

void getNewApple() {

  int appleRow = random(8);
  int appleCol = random(8);

  bool allGood = false;
  bool checkedAll = true;


  while (!allGood) {
    checkedAll = true;
    for (int i = 0; i < snakeLength; i++) {
      int index = (headIndex + maxLength - i) % maxLength;
      if (snake[index].row == appleRow && snake[index].col == appleCol) {
        appleRow = random(8);
        appleCol = random(8);
        checkedAll = false;
        break;
      }
    }

    if (checkedAll) {
      allGood = true;
    }
  }

  apple.row = appleRow;
  apple.col = appleCol;

  lc.setLed(display, appleRow, appleCol, true);
}

void gameOver() {
  for (int i = 0; i < 3; i++) {
    lc.clearDisplay(display);
    delay(150);
    for (int j = 0; j < snakeLength; j++) {
      int index = (headIndex + maxLength - j) % maxLength;
      lc.setLed(display, snake[index].row, snake[index].col, true);
    }
    delay(150);
  }

  while (true)
    ;
}
