#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <util/delay.h>

// Buttons
#define WAKEUP_PRESS  500
#define BUTTON_OK     A4
#define BUTTON_UP     A3
#define BUTTON_DOWN   A2
#define BUTTON_LEFT   A1
#define BUTTON_RIGHT  A0

// Display
#define OLED_VCC1      6
#define OLED_VCC0      7
#define OLED_RST       8
#define OLED_DC        9
#define OLED_CS        10
#define OLED_SPI_SPEED 4000000ul
#define SCREEN_WIDTH 128

// Power
#define INTERNAL_REF  1100
#define BATTERY_FULL  3200
#define BATTERY_EMPTY 2000
#define SLEEP_TIMEOUT 30000

/* EEPROM */
#define EEPROM_KEY          0xB1  // EEPROM KEY
#define KEY_EE_ADDR         0     // EEPROM KEY address
#define BRIGHT_EE_ADDR      1     // Display brightness address in EEPROM
#define DINO_EE_ADDR        2     // "Dinosaur game" address in EEPROM
#define MICROPONG_EE_ADDR   3     // "MicroPong game" address in EEPROM

// Menu
#define MENU_FRAMERATE  30
#define APPS_AMOUNT     2

#include <GyverButton.h>
#include <GyverPower.h>
#include <GyverOLED.h>


// Objects
extern GyverOLED <SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;
extern GButton ok;
extern GButton up;
extern GButton down;
extern GButton left;
extern GButton right;

extern uint32_t globalSleepTimer;

void oledPower(bool state);

void goToSleep(void);

void batCheckDraw(void);

#endif // __GLOBALS_H__