#include "MicroPong.h"

#define GAME_SPEED 20    // стандартный период движения шарика
#define II_SPEED 30      // стандартный период движения противника
#define RANDOM_BOUNCE 1   // отбивать шарик в случайном направлении

// другие настройки
#define X_PLAYER_1 7
#define X_PLAYER_2 SCREEN_WIDTH - 7
#define RACKET_LEN 15
#define MAX_BALL_Y 63
#define RACKET_STEP 5
#define TOP_LINE_Y 10

int8_t ballPos[2];
int8_t ballSpeed[2];
uint32_t ballTimer, enemyTimer;
boolean btnFlag1, btnFlag2;
int8_t racketPos1, racketPos2 = TOP_LINE_Y + 1;
int8_t prevRacketPos1, prevRacketPos2 = 0;
byte count1, count2;
byte speedIncr = 0, iiIncr = 0;
uint16_t score = 0;
uint16_t bestScore = 0;

void dotSet(byte x, byte y) {
  oled.rect(x, y, x + 1, y + 1, OLED_FILL);
}

void dotClear(byte x, byte y) {
   oled.rect(x, y, x + 1, y + 1, OLED_CLEAR);
}

void redrawRacket() {
  for (byte i = prevRacketPos1; i < prevRacketPos1 + RACKET_LEN; i++) {
    dotClear(X_PLAYER_1, i);
  }
  for (byte i = racketPos1; i < racketPos1 + RACKET_LEN; i++) {
    dotSet(X_PLAYER_1, i);
  }
  prevRacketPos1 = racketPos1;
}

void redrawRacket2() {
  for (byte i = prevRacketPos2; i < prevRacketPos2 + RACKET_LEN; i++) {
    dotClear(X_PLAYER_2, i);
  }
  for (byte i = racketPos2; i < racketPos2 + RACKET_LEN; i++) {
    dotSet(X_PLAYER_2, i);
  }
  prevRacketPos2 = racketPos2;
}

void newRound() {
  randomSeed(millis());
  ballPos[0] = X_PLAYER_1 + 1;
  racketPos1 = random(TOP_LINE_Y + 1, MAX_BALL_Y - RACKET_LEN);
  ballPos[1] = racketPos1 + RACKET_LEN / 2;
  ballSpeed[0] = 2;
  ballSpeed[1] = (random(0, 2)) ? 1 : -1;

  redrawRacket();
  redrawRacket2();
  if (score >= 10) {
    speedIncr = 10;
    iiIncr = 70;
  }
  if (score >= 20) {
    speedIncr = 25;
    iiIncr = 150;
  }
  if (score >= 30) {
    speedIncr = 40;
    iiIncr = 190;
  }
}

void DrawGameOverAction(void) {
  resetButtonsSetup();
  batCheckDraw();

  oled.roundRect(0, 10, 127, 40, OLED_CLEAR); oled.roundRect(0, 10, 127, 40, OLED_STROKE);
  oled.setScale(2); oled.setCursor(7, 2); oled.print(F("GAME OVER!"));
  oled.setScale(1); oled.setCursor(3, 4); oled.print(F("<- MENU"));
  oled.setCursor(83, 4); oled.print(F("PLAY ->"));
  oled.update();
  if (score > bestScore) EEPROM.put(MICROPONG_EE_ADDR, score);
  while (1) {
    if (right.isClick()) PlayMicroPongGame();
    if (left.isClick()) {
      resetButtonsSetup();
      return;
    }
    if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {
      goToSleep();
    }
  }
}

void PlayMicroPongGame(void) {
  down.setTimeout(160);
  up.setTimeout(160);

  score = 0;
  bestScore = 0;
  EEPROM.get(MICROPONG_EE_ADDR, bestScore);

  randomSeed(millis());
  newRound();

  while (1) {
    if (left.isClick()) return;

    if (millis() - ballTimer >= (GAME_SPEED - speedIncr)) {
      oled.clear();
      batCheckDraw();
      
      oled.line(0, TOP_LINE_Y, 127, TOP_LINE_Y); // draw line
      oled.setCursor(0, 0); oled.print("HI");
      // currect score
      oled.setCursor(13, 0); oled.print(bestScore); oled.print(":"); oled.print(score);
      oled.line(0, 63, 127, 63); // draw line

      redrawRacket();
      redrawRacket2();

      ballTimer = millis();
      int8_t prevPos[2];
      for (byte i = 0; i < 2; i++) {
        prevPos[i] = ballPos[i];
        ballPos[i] += ballSpeed[i];
      }

      if (ballPos[0] < X_PLAYER_1) {
        if (!(prevPos[1] >= racketPos1 && prevPos[1] <= (racketPos1 + RACKET_LEN))) {
          count2++;
          DrawGameOverAction();
          return;
        } else {
          ballPos[0] = prevPos[0];
          ballSpeed[0] = -ballSpeed[0];
          if (RANDOM_BOUNCE) ballSpeed[1] *= (random(0, 2)) ? 1 : -1;
        }
      }
     
      if (ballPos[1] <= TOP_LINE_Y + 1) {
        ballPos[1] = TOP_LINE_Y + 1;
        ballSpeed[1] = -ballSpeed[1];
      }

      // Tracing a collision
      if (ballPos[0] > X_PLAYER_2) {
        if (!(prevPos[1] >= racketPos2 && prevPos[1] <= (racketPos2 + RACKET_LEN))) {
          score++;

          DrawGameOverAction();
          return;
        } else {
          ballPos[0] = prevPos[0];
          ballSpeed[0] = -ballSpeed[0];
          if (RANDOM_BOUNCE) ballSpeed[1] *= (random(0, 2)) ? 1 : -1;
        }
      }

      if (ballPos[1] > MAX_BALL_Y - 1) {
        ballPos[1] = MAX_BALL_Y - 1;
        ballSpeed[1] = -ballSpeed[1];
      }

      // dotClear(prevPos[0], prevPos[1]);
      dotSet(ballPos[0], ballPos[1]);

      if (up.isClick() || up.isHold()) {
        racketPos1 -= RACKET_STEP;

        if (racketPos1 < TOP_LINE_Y + 1) racketPos1 = TOP_LINE_Y + 1;
        redrawRacket();
      }

      if (down.isClick() || down.isHold()) {
        racketPos1 += RACKET_STEP;
        
        if (racketPos1 > (MAX_BALL_Y - RACKET_LEN)) racketPos1 = (MAX_BALL_Y - RACKET_LEN);
        redrawRacket();
      }

      oled.update();
    }

    if (millis() - enemyTimer >= (II_SPEED - iiIncr)) {
      enemyTimer = millis();
      // racketPos2 + RACKET_LEN / 2 > ballPos[1]
      if (racketPos2 > ballPos[1]) racketPos2--;
      else racketPos2++;
      racketPos2 = constrain(racketPos2, 0, MAX_BALL_Y - RACKET_LEN);
      redrawRacket2();
      oled.update();
    }

    static uint32_t scoreTimer = millis();
    if (millis() - scoreTimer >= 1000) {
      scoreTimer = millis();
      score++;
    }

    if (score > bestScore) EEPROM.put(MICROPONG_EE_ADDR, score);
  }
}

// Game Menu
void MicroPongGame(void) {
  while (true) {
    uint16_t bestScore = 0;
    EEPROM.get(MICROPONG_EE_ADDR, bestScore);

    oled.clear();
    oled.roundRect(0, 9, 127, 46, OLED_STROKE);
    oled.setCursor(3, 0); oled.print(F("MICROPONG GAME"));
    oled.setCursor(20, 6); oled.print(F("BEST SCORE:")); oled.print(bestScore);
    oled.setCursor(0, 7); oled.print(F("<- MENU"));
    oled.setCursor(85, 7); oled.print(F("PLAY ->"));

    // Game image (demo rockets with a ball in the middle)
    oled.rect(46, 22, 48, 37, OLED_FILL); // rocket 1
    oled.rect(76, 22, 78, 37, OLED_FILL); // rocket 2
    oled.rect(61, 30, 62, 31, OLED_FILL); // ball

    oled.update();
    while (true) {
      if (left.isClick()) {
        resetButtonsSetup();
        return;
      }
      if (right.isClick()) {
        PlayMicroPongGame();
        break;
      }
      if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {
        goToSleep();
      }
    }
  }
}