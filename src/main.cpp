#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <util/delay.h>
#include "globals.h"
#include "DinosaurGame.h"

void setup() {
  power.hardwareDisable(PWR_TIMER1 | PWR_TIMER2 | PWR_I2C | PWR_UART0); // Выключаем лишнее
  power.setSleepMode(POWERDOWN_SLEEP);                                  // Спать будем глубоко
  power.bodInSleep(false);                                              // И без BOD'a

  if (EEPROM[KEY_EE_ADDR] != EEPROM_KEY) {    // Проверка EEPROM на первое включение
    EEPROM[KEY_EE_ADDR] = EEPROM_KEY;         // При первом включении устанавливаем все как надо
    EEPROM[BRIGHT_EE_ADDR] = 100;
    EEPROM[DINO_EE_ADDR] = 0;
    EEPROM[DINO_EE_ADDR + 1] = 0;
  }

  oledPower(true);              // Включаем и инициализируем дисплей
  oled.clear();                 // Сразу очищаем его

  ok.setTickMode(AUTO);         // Настраиваем все кнопки на авто-опрос
  up.setTickMode(AUTO);
  down.setTickMode(AUTO);
  left.setTickMode(AUTO);
  right.setTickMode(AUTO);

  left.setStepTimeout(100);     // Настраиваем таймауты удержания
  right.setStepTimeout(100);

  /* Настриаваем прерывания по всем кнопкам - чтобы отслеживать нажатия */
  PCICR = 1 << PCIE1;                                                               // Включаем прерывание по действию всех кнопок
  PCMSK1 = 1 << PCINT8 | 1 << PCINT9 | 1 << PCINT10 | 1 << PCINT11 | 1 << PCINT12;  // Активируем на всех пинах, где есть кнопки
  globalSleepTimer = millis();                                                      // Сброс глобального таймера сна

  /* Настриаваем АЦП для измерения напряжения питания */
  ADMUX = DEFAULT << 6 | 0b1110;      // Опорное - AVCC, вход АЦП к внутреннему опорному
  ADCSRA = 1 << ADEN | 0b101;         // Вкл. АЦП + средн. скорость АЦП
  for (uint8_t i = 0; i < 8; i++) {   // Несколько ложных преобразований - отфильтровать мусор
    ADCSRA |= 1 << ADSC;              // Запускаем преобразование
    while (ADCSRA & (1 << ADSC));     // Ждем окончания
  }

}

void loop() {
  static uint8_t menuPtr = 2;  // Положение указателя меню

  /* Обработка кнопок в главном меню */
  if (left.isClick() or left.isStep()) {  // Влево - уменьшить и сохранить яркость
    EEPROM[BRIGHT_EE_ADDR] = constrain(EEPROM[BRIGHT_EE_ADDR] - 5, 5, 100); // Уменьшаем значение в EEPROM [5-100%]
    oled.setContrast(map(EEPROM[BRIGHT_EE_ADDR], 0, 100, 0, 255));          // Устанавливаем яркость дисплея
  }

  if (right.isClick() or right.isStep()) {  // Вправо - увеличить и сохранить яркость
    EEPROM[BRIGHT_EE_ADDR] = constrain(EEPROM[BRIGHT_EE_ADDR] + 5, 5, 100); // Увеличиваем значение в EEPROM [5-100%]
    oled.setContrast(map(EEPROM[BRIGHT_EE_ADDR], 0, 100, 0, 255));          // Устанавливаем яркость дисплея
  }

  if (up.isClick()) {                       // Вверх - выбрать пункт выше
    menuPtr = constrain(menuPtr - 1, 2, APPS_AMOUNT + 1); // Двигаем указатель в пределах меню
  }

  if (down.isClick()) {                     // Вниз - выбрать пункт ниже
    menuPtr = constrain(menuPtr + 1, 2, APPS_AMOUNT + 1); // Двигаем указатель в пределах меню
  }

  if (ok.isClick()) {                       // Ок - перейти в приложение
    switch (menuPtr) {                      // В зависимости от пункта меню
      case 2: DinosaurGame(); break;        // Вызываем нужное
      case 3: break;
      case 4: break;
      case 5: break;
      case 6: break;
      case 7: break;
    }
  }

  /* Отрисовка главного меню */
  static uint32_t drawTimer = millis();
  if (millis() - drawTimer >= (1000 / MENU_FRAMERATE)) {        // По таймеру на миллис
    drawTimer = millis();
    oled.clear();                                               // Чистим дисплей
    oled.setCursor(24, 2); oled.print(F("DINOSAUR GAME"));      // Выводим название приложений
    // oled.setCursor(24, 3); oled.print(F("NEW GAME NAME"));   // пустые заготовки
    // oled.setCursor(24, 4); oled.print(F("NEW GAME NAME"));
    // oled.setCursor(24, 5); oled.print(F("NEW GAME NAME"));
    // oled.setCursor(24, 6); oled.print(F("NEW GAME NAME"));
    // oled.setCursor(24, 7); oled.print(F("NEW GAME NAME"));

    oled.setCursor(0, menuPtr); oled.print('>');                // Выводим левый указатель
    oled.setCursor(122, menuPtr); oled.print('<');              // Выводим правый указатель
    oled.home(); oled.print(F("BRIGHT: "));                     // Выводим яркость
    oled.print(EEPROM[BRIGHT_EE_ADDR]); oled.print(" % ");      // Из EEPROM
    batCheckDraw();                                             // Проверка и отрисовка заряда
    oled.update();                                              // Выводим изображение на дисплей
  }

  if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {            // Проверка глобального таймера сна
    goToSleep();                                                // Если кнопки долго не нажимались - идем спать
  }
}

/* ----------------------------------------------------- Сервисные функции ----------------------------------------------------- */

/* Это прерывание вызывается при ЛЮБОМ действии ЛЮБОЙ кнопки */
ISR(PCINT1_vect) {
  globalSleepTimer = millis();  // Обновляем глобальный таймер нажатий
}

