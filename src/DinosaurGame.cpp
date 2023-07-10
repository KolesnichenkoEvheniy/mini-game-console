#include "DinosaurGame.h"

// Game constants
#define DINO_GROUND_Y 47
#define DINO_GRAVITY  0.195f
#define DINO_GAME_FPS 30

enum EnemyType{SmallCactus=0, BigCactus=1, Bird=2};

void PlayDinosaurGame(void) {
  down.setTimeout(160);
  ok.setTimeout(160);
  ok.setStepTimeout(160);

  uint8_t gameSpeed = 10;
  uint16_t score = 0;
  uint16_t bestScore = 0;
  int8_t oldEnemyPos = SCREEN_WIDTH;
  int8_t oldEnemyType = 0;
  int8_t newEnemyPos = SCREEN_WIDTH;
  int8_t newEnemyType = random(0, 3);
  bool dinoStand = true;
  bool legFlag = true;
  bool birdFlag = true;
  int8_t dinoY = DINO_GROUND_Y; // dino pos
  float dinoU = 0.0; // dino speed (vector down)

  EEPROM.get(DINO_EE_ADDR, bestScore);

  while (1) {
    if (left.isClick()) return;

    /* ------------------ User input ------------------ */
    if (ok.isClick() and dinoY == DINO_GROUND_Y) {
      dinoU = -2.8;
      dinoY -= 4;
    } else if ((ok.isHolded() or ok.isStep()) and dinoY == DINO_GROUND_Y) {
      dinoU = -3.4;
      dinoY -= 4;
    } else if (down.state()) {
      dinoU = 3.2;
      if (dinoY >= DINO_GROUND_Y) {
        dinoY = DINO_GROUND_Y;
        dinoU = 0.0;
      }
    }

    if (down.isHold() and dinoY >= DINO_GROUND_Y) {
      dinoStand = false;
    } else {
      dinoStand = true;
    }

    /* ------------------ Game processing ------------------ */
    static uint32_t scoreTimer = millis();
    if (millis() - scoreTimer >= 100) {
      scoreTimer = millis();
      score++;
      gameSpeed = constrain(map(score, 1000, 0, 4, 10), 4, 10);
    }

    static uint32_t enemyTimer = millis();
    if (millis() - enemyTimer >= gameSpeed) {
      enemyTimer = millis();
      if (--newEnemyPos < 16) {
        oldEnemyPos = newEnemyPos;
        oldEnemyType = newEnemyType;
        newEnemyPos = SCREEN_WIDTH;
        newEnemyType = random(0, 3);
      }
      if (oldEnemyPos >= -24) {
        oldEnemyPos--;
      }
    }

    static uint32_t legTimer = millis();
    if (millis() - legTimer >= 130) {
      legTimer = millis();
      legFlag = !legFlag;
    }

    static uint32_t birdTimer = millis();
    if (millis() - birdTimer >= 200) {
      birdTimer = millis();
      birdFlag = !birdFlag;
    }

    static uint32_t dinoTimer = millis();
    if (millis() - dinoTimer >= 15) {
      dinoTimer = millis();
      dinoU += (float)DINO_GRAVITY;
      dinoY += (float)dinoU;
      if (dinoY >= DINO_GROUND_Y) {
        dinoY = DINO_GROUND_Y;
        dinoU = 0.0;
      }
    }

    /* ------------------ Drawing ------------------ */
    static uint32_t oledTimer = millis();
    if (millis() - oledTimer >= (1000 / DINO_GAME_FPS)) {
      oledTimer = millis();

      oled.clear();
      batCheckDraw();
      // highest record
      oled.setCursor(0, 0); oled.print("HI");
      // currect score
      oled.setCursor(13, 0); oled.print(bestScore); oled.print(":"); oled.print(score);
      oled.line(0, 63, 127, 63); // draw line

      switch (oldEnemyType) {
        case EnemyType::SmallCactus:
          oled.drawBitmap(oldEnemyPos, 48, CactusSmall_bmp, 16, 16);
          break;
        case EnemyType::BigCactus:
          oled.drawBitmap(oldEnemyPos, 48, CactusBig_bmp, 24, 16);
          break;
        case EnemyType::Bird:
          oled.drawBitmap(oldEnemyPos, 35, birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16);
          break;
      }

      switch (newEnemyType) {
        case EnemyType::SmallCactus:
          oled.drawBitmap(newEnemyPos, 48, CactusSmall_bmp, 16, 16);                  
          break;
        case EnemyType::BigCactus:
          oled.drawBitmap(newEnemyPos, 48, CactusBig_bmp, 24, 16);                    
          break;
        case EnemyType::Bird:
          oled.drawBitmap(newEnemyPos, 35, birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16); 
          break;
      }


      // Tracing a Collision
      if (oldEnemyPos <= 16 and oldEnemyPos >= (oldEnemyType > 0 ? -24 : -16)) {
        if (oldEnemyType != EnemyType::Bird ? dinoY > 32 : dinoStand and dinoY > 19) {
          oled.drawBitmap(0, dinoY, DinoStandDie_bmp, 16, 16);
          oled.roundRect(0, 10, 127, 40, OLED_CLEAR); oled.roundRect(0, 10, 127, 40, OLED_STROKE);
          oled.setScale(2); oled.setCursor(7, 2); oled.print(F("GAME OVER!"));
          oled.setScale(1); oled.setCursor(3, 4); oled.print(F("<- MENU"));
          oled.setCursor(83, 4); oled.print(F("PLAY ->"));
          oled.update();
          if (score > bestScore) EEPROM.put(DINO_EE_ADDR, score);
          while (1) {
            if (right.isClick()) PlayDinosaurGame();
            if (left.isClick()) return;
            if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {
              goToSleep();
            }
          }
        }
      }

      if (dinoStand) {
        oled.drawBitmap(0, dinoY, legFlag ? DinoStandL_bmp : DinoStandR_bmp, 16, 16);
      } else {
        oled.drawBitmap(0, 56, legFlag ? DinoCroachL_bmp : DinoCroachR_bmp, 16, 8);
      }

      oled.update();
    }
  }
}

// Game Menu
void DinosaurGame(void) {
  while (true) {
    uint16_t bestScore = 0;
    EEPROM.get(DINO_EE_ADDR, bestScore);
    oled.clear();
    oled.roundRect(0, 9, 127, 46, OLED_STROKE);
    oled.setCursor(3, 0); oled.print(F("GOOGLE DINOSAUR GAME"));
    oled.setCursor(20, 6); oled.print(F("BEST SCORE:")); oled.print(bestScore);
    oled.setCursor(0, 7); oled.print(F("<- MENU"));
    oled.setCursor(85, 7); oled.print(F("PLAY ->"));
    oled.drawBitmap(10, 30, DinoStandL_bmp, 16, 16);
    oled.drawBitmap(46, 30, CactusBig_bmp, 24, 16);
    oled.drawBitmap(94, 20, BirdL_bmp, 24, 16);
    oled.update();
    while (true) {
      if (left.isClick()) {
        resetButtonsSetup();
        return;
      }
      if (right.isClick()) {
        PlayDinosaurGame();
        break;
      }
      if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {
        goToSleep();
      }
    }
  }
}
