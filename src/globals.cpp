#include "globals.h"

GyverOLED <SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;
GButton ok(BUTTON_OK);
GButton up(BUTTON_UP);
GButton down(BUTTON_DOWN);
GButton left(BUTTON_LEFT);
GButton right(BUTTON_RIGHT);

uint32_t globalSleepTimer = 0;


/* Включение и выключения OLED */
void oledPower(bool state) {                                          // Включение / выключение оледа
  if (state) {                                                        // Включаем
    pinMode(OLED_VCC1, OUTPUT);                                       // Питающие пины как выходы
    pinMode(OLED_VCC0, OUTPUT);                                       // Питающие пины как выходы
    digitalWrite(OLED_VCC1, HIGH);                                    // Питающие пины в HIGH
    digitalWrite(OLED_VCC0, HIGH);                                    // Питающие пины в HIGH
    _delay_ms(15);                                                    // Даем время дисплею оклематься
    oled.init();                                                      // Инициализируем дисплей
    oled.setContrast(map(EEPROM[BRIGHT_EE_ADDR], 0, 100, 0, 255));    // Восстанавливаем яркость из EEPROM
  } else {                                                            // Выключаем
    for (uint8_t i = EEPROM[BRIGHT_EE_ADDR]; i; i--) {                // Плавно от установленной яркости
      oled.setContrast(i);                                            // Гасим дисплей
      _delay_ms(10);                                                  // С задержкой для плавности
    }
    oled.setPower(false);                                             // Выключаем программно
    digitalWrite(OLED_VCC1, LOW);                                     // Питающие пины в LOW
    digitalWrite(OLED_VCC0, LOW);                                     // Питающие пины в LOW
    pinMode(OLED_VCC1, INPUT);                                        // Питающие пины как входы
    pinMode(OLED_VCC0, INPUT);                                        // Питающие пины как входы
  }
}

/* Уход в сон и возврат из сна */
void goToSleep(void) {
  bool wakeup;                                // Флаг пробуждения
  uint32_t timer;                             // Таймер
  oledPower(false);                           // Выключаем олед
  PCMSK1 = 1 << PCINT12;                      // Прерывание и пробуждение только по "OK"
  while (true) {                              // Бесконечный цикл
    power.sleep(SLEEP_FOREVER);               // << Уходим в сон
    wakeup = false;                      // >> проснулись, сбросили флаг
    timer = millis();                // Обновили таймер
    while (ok.state()) {                      // Пока нажата кнопка
      if (millis() - timer > WAKEUP_PRESS) {  // Если кнопка нажата дольше указанного - можем просыпаться
        wakeup = true;                        // Ставим флаг
      }
    } if (wakeup) break;                      // Как только кнопка отпущена, смотрим - если флаг стоит, просыпаемся
  }
  PCMSK1 = 1 << PCINT8 | 1 << PCINT9 | 1 << PCINT10 | 1 << PCINT11 | 1 << PCINT12;  // Возвращаем обратно прерывание по всем кнопкам
  oledPower(true);                            // Подрубаем олед заного
}

/* Тестирование батареи и вывод заряда на экран */
void batCheckDraw(void) {
  static uint32_t measureTimer = millis() + 3500;  // Таймер АЦП (Стартует сразу)
  static uint8_t batCharge = 0;                    // "Заряд" батареи

  if (millis() - measureTimer >= 3000) {
    measureTimer = millis();
    /* Измеряем напряжение питания + усредняем */
    ADCSRA |= 1 << ADSC;                // Запускаем преобразование
    while (ADCSRA & (1 << ADSC));       // Ждем
    /* Пересчитываем напряжение в условный заряд */
    batCharge = constrain(map((INTERNAL_REF * 1024UL) / ADC, BATTERY_EMPTY, BATTERY_FULL, 0, 12), 0, 12);
  }

  /* Рисуем батарейку */
  oled.setCursorXY(110, 0);                             // Положение на экране
  oled.drawByte(0b00111100);                            // Пипка
  oled.drawByte(0b00111100);                            // 2 штуки
  oled.drawByte(0b11111111);                            // Передняя стенка
  for (uint8_t i = 0; i < 12; i++) {                    // 12 градаций
    if (i < 12 - batCharge) oled.drawByte(0b10000001);   // Рисуем пустые
    else oled.drawByte(0b11111111);                     // Рисуем полные
  } oled.drawByte(0b11111111);                          // Задняя стенка
}