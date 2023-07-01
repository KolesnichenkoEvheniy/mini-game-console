#include "DinosaurGame.h"

/* ---------------------------------------- 2. Реализация игры ----------------------------------------*/
#define DINO_GROUND_Y 47        // Позиция динозавра по вертикали
#define DINO_GRAVITY  0.195f    // Значение гравитации динозавра
#define DINO_GAME_FPS 30        // Скорость обновления дисплея

void PlayDinosaurGame(void) {
  down.setTimeout(160);         // Настраиваем удобные таймауты удержания
  ok.setTimeout(160);
  ok.setStepTimeout(160);

startDinoGame:                         // Начало игры
  uint8_t gameSpeed = 10;              // Скорость игры
  uint16_t score = 0;                  // Текущий счет
  uint16_t bestScore = 0;              // Рекорд
  int8_t oldEnemyPos = 128;            // Позиция старого противника (тот, что уже заходит за горизонт)
  int8_t oldEnemyType = 0;             // Тип старого противника (тот, что уже заходит за горизонт)
  int8_t newEnemyPos = 128;            // Позиция нового противника (тот, что только выходит изза горизонта)
  int8_t newEnemyType = random(0, 3);  // Тип нового противника - определяем случайно
  bool dinoStand = true;               // Динозавр стоит на земле
  bool legFlag = true;                 // Флаг переключения ног динозавра
  bool birdFlag = true;                // Флаг взмахов птицы
  int8_t dinoY = DINO_GROUND_Y;        // Позиция динозавра по вертикали (изначально на земле)
  float dinoU = 0.0;                   // Скорость динозавра (вектор направлен вниз)

  EEPROM.get(DINO_EE_ADDR, bestScore); // Читаем рекорд из EEPROM

  while (1) {                                                   // Бесконечный цикл игры
    if (left.isClick()) return;                                 // Клик кнопки влево мгновенно возвращает нас в игровое меню

    /* ------------------ User input ------------------ */
    if (ok.isClick() and dinoY == DINO_GROUND_Y) {                           // Клик по ОК и динозавр стоит на земле (слабый прыжок)
      dinoU = -2.8;                                                          // Прибавляем скорости по направлению вверх
      dinoY -= 4;                                                            // Подкидываем немного вверх
    } else if ((ok.isHolded() or ok.isStep()) and dinoY == DINO_GROUND_Y) {  // Удержание ОК и динозавр стоит на земле (сильный прыжок)
      dinoU = -3.4;                                                          // Прибавляем скорости по направлению вверх
      dinoY -= 4;                                                            // Подкидываем немного вверх
    } else if (down.state()) {                                               // Нажатие ВНИЗ
      dinoU = 3.2;                                                           // Прибавляем скорости по направлению к земле
      if (dinoY >= DINO_GROUND_Y) {                                          // Если динозавр коснулся земли
        dinoY = DINO_GROUND_Y;                                               // Ставим его на землю
        dinoU = 0.0;                                                         // Обнуляем скорость
      }
    }

    if (down.isHold() and dinoY >= DINO_GROUND_Y) {                          // Удержание ВНИЗ и дино стоит на земле
      dinoStand = false;                                                     // Переходим в присяд
    } else {
      dinoStand = true;                                                      // Иначе встаем обратно
    }

    /* ------------------ Game processing ------------------ */
    static uint32_t scoreTimer = millis();                                   // Таймер подсчета очков
    if (millis() - scoreTimer >= 100) {
      scoreTimer = millis();
      score++;                                                               // Увеличиваем счет
      gameSpeed = constrain(map(score, 1000, 0, 4, 10), 4, 10);              // Увеличиваем скорость игры! (10 - медленно, 4 - очень быстро)
    }

    static uint32_t enemyTimer = millis();                                   // Таймер кинематики противников
    if (millis() - enemyTimer >= gameSpeed) {                                // Его период уменьшается с ростом счета
      enemyTimer = millis();
      if (--newEnemyPos < 16) {                                              // Как только НОВЫЙ противник приближается к динозавру
        oldEnemyPos = newEnemyPos;                                           // Новый противник становится старым
        oldEnemyType = newEnemyType;                                         // И копирует тип нового к себе
        newEnemyPos = 128;                                                   // Между тем новый противник выходит изза горизонта
        newEnemyType = random(0, 3);                                         // Имея каждый раз новый случайный тип
      }
      if (oldEnemyPos >= -24) {                                              // Двигаем старый пока он полностью не скроется за горизонтом
        oldEnemyPos--;                                                       // Двигаем старый
      }
    }

    static uint32_t legTimer = millis();                                     // Таймер анимации ног динозавра
    if (millis() - legTimer >= 130) {
      legTimer = millis();
      legFlag = !legFlag;                                                    // Он просто переключает флаг
    }

    static uint32_t birdTimer = millis();                                    // Таймер анимации взмахов птицы
    if (millis() - birdTimer >= 200) {
      birdTimer = millis();
      birdFlag = !birdFlag;                                                  // Он тоже просто переключает флаг!
    }

    static uint32_t dinoTimer = millis();                                    // Таймер кинематики динозавра
    if (millis() - dinoTimer >= 15) {                                        // С периодом DT
      dinoTimer = millis();
      dinoU += (float)DINO_GRAVITY;                                          // Увеличиваем скорость
      dinoY += (float)dinoU;                                                 // И соответственно координату (динозавр падает)
      if (dinoY >= DINO_GROUND_Y) {                                          // При касании с землей
        dinoY = DINO_GROUND_Y;                                               // Ставим динозвра на землю
        dinoU = 0.0;                                                         // Тормозим его до нуля
      }
    }

    /* ------------------ Drawing ------------------ */
    static uint32_t oledTimer = millis();                                    // Таймер отрисовки игры!
    if (millis() - oledTimer >= (1000 / DINO_GAME_FPS)) {                    // Привязан к FPS игры
      oledTimer = millis();

      oled.clear();                                                                                     // Чистим дисплей
      batCheckDraw();                                                                                   // Проверка и рисование батарейки
      oled.setCursor(0, 0); oled.print("HI");                                                           // Выводим рекорд
      oled.setCursor(13, 0); oled.print(bestScore); oled.print(":"); oled.print(score);                 // Рекорд:текущий счет
      oled.line(0, 63, 127, 63);                                                                        // Рисуем поверхность земли (линия)

      switch (oldEnemyType) {                                                                           // Выбираем старого противника
        case 0: oled.drawBitmap(oldEnemyPos, 48, CactusSmall_bmp, 16, 16);                   break;     // Рисуем маленький кактус
        case 1: oled.drawBitmap(oldEnemyPos, 48, CactusBig_bmp, 24, 16);                     break;     // Рисуем большой кактус
        case 2: oled.drawBitmap(oldEnemyPos, 35, birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16);  break;     // Рисуем птицу (выбираем одну из двух картинок для анимации)
      }

      switch (newEnemyType) {                                                                           // Выбираем нового противника
        case 0: oled.drawBitmap(newEnemyPos, 48, CactusSmall_bmp, 16, 16);                     break;   // Рисуем маленький кактус
        case 1: oled.drawBitmap(newEnemyPos, 48, CactusBig_bmp, 24, 16);                       break;   // Рисуем большой кактус
        case 2: oled.drawBitmap(newEnemyPos, 35, birdFlag ? BirdL_bmp : BirdR_bmp, 24, 16);    break;   // Рисуем птицу (выбираем одну из двух картинок для анимации)
      }

      if (oldEnemyPos <= 16 and oldEnemyPos >= (oldEnemyType > 0 ? -24 : -16)) {                        // Если противник в опасной зоне (Отслеживаем столкновения)
        if (oldEnemyType != 2 ? dinoY > 32 : dinoStand and dinoY > 19) {                                // Выбираем условие столкновения в зависимости от типа противника 
          oled.drawBitmap(0, dinoY, DinoStandDie_bmp, 16, 16);                                          // Столкнулись - рисуем погибшего динозавра :(  
          oled.roundRect(0, 10, 127, 40, OLED_CLEAR); oled.roundRect(0, 10, 127, 40, OLED_STROKE);      // Очищаем и обводим область
          oled.setScale(2); oled.setCursor(7, 2); oled.print(F("GAME OVER!"));                          // Выводим надпись   
          oled.setScale(1); oled.setCursor(3, 4); oled.print(F("<- MENU"));                             // Выводим подсказку
          oled.setCursor(83, 4); oled.print(F("PLAY ->"));                                              // Выводим подсказку
          oled.update();                                                                                // Отрисовка картинки на дисплей
          if (score > bestScore) EEPROM.put(DINO_EE_ADDR, score);                                       // Если новый рекорд - обновляем его
          while (1) {                                                                                   // Бесконечный цикл 
            if (right.isClick()) goto startDinoGame;                                                    // Нажали на правую - начинаем сначала
            if (left.isClick()) return;                                                                 // Нажали на левую - вернулись в меню
            if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {                                          // Отслеживание таймаута сна
              goToSleep();                                                                              // Уходим в сон по необходимости
            }
          }
        }
      }

      if (dinoStand) {                                                                                  // Если все окей, столкновения нет и дино стоит в полный рост
        oled.drawBitmap(0, dinoY, legFlag ? DinoStandL_bmp : DinoStandR_bmp, 16, 16);                   // Выводим в полный рост с анимацией переступания  
      } else {                                                                                          // Дино пригнулся
        oled.drawBitmap(0, 56, legFlag ? DinoCroachL_bmp : DinoCroachR_bmp, 16, 8);                     // Выводим пригнувшимся, тоже с анимацией ног
      }

      oled.update();                                                                                    // Финальная отрисовка на дисплей
    }
  }
}

/* ---------------------------------------- 2. Меню игры ----------------------------------------*/
void DinosaurGame(void) {                                                       // Главное меню игры
  while (true) {                                                                // Бесконечный цикл
    uint16_t bestScore = 0;                                                     // Лучший счет
    EEPROM.get(DINO_EE_ADDR, bestScore);                                        // Берем его из EEPROM
    oled.clear();                                                               // Очистка дисплея
    oled.roundRect(0, 9, 127, 46, OLED_STROKE);                                 // Отрисовка интерфейса
    oled.setCursor(3, 0); oled.print(F("GOOGLE DINOSAUR GAME"));                // Отрисовка интерфейса
    oled.setCursor(20, 6); oled.print(F("BEST SCORE:")); oled.print(bestScore); // Вывод рекорда
    oled.setCursor(0, 7); oled.print(F("<- MENU"));                             // Отрисовка интерфейса
    oled.setCursor(85, 7); oled.print(F("PLAY ->"));                            // Отрисовка интерфейса
    oled.drawBitmap(10, 30, DinoStandL_bmp, 16, 16);                            // Вывод картинок
    oled.drawBitmap(46, 30, CactusBig_bmp, 24, 16);                             // Вывод картинок
    oled.drawBitmap(94, 20, BirdL_bmp, 24, 16);                                 // Вывод картинок
    oled.update();                                                              // Вывод на дисплей
    while (true) {                                                              // Вложенный бесконечный цикл
      if (left.isClick()) return;                                               // Нажатие на левую кнопку - возврат в меню
      if (right.isClick()) {                                                    // Нажатие на правую - начать играть
        PlayDinosaurGame();                                                     // Запускаем игру
        break;                                                                  // При выходе из игры переходим к отрисовке
      }
      if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {                        // Отслеживаем таймаут сна
        goToSleep();                                                            // Уходим в сон если долго не трогали кнопки
      }
    }
  }
}
