#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <util/delay.h>

/* Пины кнопок */
#define WAKEUP_PRESS  500
#define BUTTON_OK     A4
#define BUTTON_UP     A3
#define BUTTON_DOWN   A2
#define BUTTON_LEFT   A1
#define BUTTON_RIGHT  A0

/* Пины дисплея */
#define OLED_VCC1      6
#define OLED_VCC0      7
#define OLED_RST       8
#define OLED_DC        9
#define OLED_CS        10
#define OLED_SPI_SPEED 4000000ul

/* Параметры источника питания */
#define INTERNAL_REF  1100
#define BATTERY_FULL  3200
#define BATTERY_EMPTY 2000
#define SLEEP_TIMEOUT 10000 // Таймаут, после которого устройство переходит в сон, если не трогать кнопки (в мс)

/* Параметры EEPROM */
#define EEPROM_KEY      0xB1  // Ключ EEPROM
#define KEY_EE_ADDR     0     // Адрес ключа в EEPROM
#define BRIGHT_EE_ADDR  1     // Адрес яркости дисплея в EEPROM
#define DINO_EE_ADDR    2     // Адрес рекорда для игры "Dinosaur game" 

/* Параметры меню */
#define MENU_FRAMERATE  30    // Частота кадров в меню (FPS)
#define APPS_AMOUNT     1     // Количество игр в меню

#include <GyverButton.h>
#include <GyverPower.h>
#include <GyverOLED.h>



/* Обьекты */
extern GyverOLED <SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;
extern GButton ok;
extern GButton up;
extern GButton down;
extern GButton left;
extern GButton right;

extern uint32_t globalSleepTimer;

/* Включение и выключения OLED */
void oledPower(bool state);

/* Уход в сон и возврат из сна */
void goToSleep(void);

/* Тестирование батареи и вывод заряда на экран */
void batCheckDraw(void);

#endif // __GLOBALS_H__