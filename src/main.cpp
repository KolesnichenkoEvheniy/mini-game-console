#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <util/delay.h>
#include "globals.h"
#include "DinosaurGame.h"

void setup() {
  power.hardwareDisable(PWR_TIMER1 | PWR_TIMER2 | PWR_I2C | PWR_UART0);
  power.setSleepMode(POWERDOWN_SLEEP);
  power.bodInSleep(false);

  if (EEPROM[KEY_EE_ADDR] != EEPROM_KEY) {
    EEPROM[KEY_EE_ADDR] = EEPROM_KEY;
    EEPROM[BRIGHT_EE_ADDR] = 100;
    EEPROM[DINO_EE_ADDR] = 0;
    EEPROM[DINO_EE_ADDR + 1] = 0;
  }

  oledPower(true);
  oled.clear();

  ok.setTickMode(AUTO);
  up.setTickMode(AUTO);
  down.setTickMode(AUTO);
  left.setTickMode(AUTO);
  right.setTickMode(AUTO);

  left.setStepTimeout(100);
  right.setStepTimeout(100);

  /* Enable interrupts for all buttons */
  PCICR = 1 << PCIE1;
  // Activate on all pins where there are buttons
  PCMSK1 = 1 << PCINT8 | 1 << PCINT9 | 1 << PCINT10 | 1 << PCINT11 | 1 << PCINT12;
  globalSleepTimer = millis();

  // Setting up the ADC to measure the supply voltage
  ADMUX = DEFAULT << 6 | 0b1110;
  ADCSRA = 1 << ADEN | 0b101;
  for (uint8_t i = 0; i < 8; i++) {
    ADCSRA |= 1 << ADSC;
    while (ADCSRA & (1 << ADSC));
  }

}

void loop() {
  static uint8_t menuPtr = 2;

  if (left.isClick() or left.isStep()) {
    EEPROM[BRIGHT_EE_ADDR] = constrain(EEPROM[BRIGHT_EE_ADDR] - 5, 5, 100);
    oled.setContrast(map(EEPROM[BRIGHT_EE_ADDR], 0, 100, 0, 255));
  }

  if (right.isClick() or right.isStep()) {
    EEPROM[BRIGHT_EE_ADDR] = constrain(EEPROM[BRIGHT_EE_ADDR] + 5, 5, 100);
    oled.setContrast(map(EEPROM[BRIGHT_EE_ADDR], 0, 100, 0, 255));
  }

  if (up.isClick()) {
    menuPtr = constrain(menuPtr - 1, 2, APPS_AMOUNT + 1);
  }

  if (down.isClick()) {
    menuPtr = constrain(menuPtr + 1, 2, APPS_AMOUNT + 1);
  }

  if (ok.isClick()) {
    switch (menuPtr) {
      case 2: DinosaurGame(); break;
      case 3: break;
      case 4: break;
      case 5: break;
      case 6: break;
      case 7: break;
    }
  }

  // Main menu
  static uint32_t drawTimer = millis();
  if (millis() - drawTimer >= (1000 / MENU_FRAMERATE)) {
    drawTimer = millis();
    oled.clear();
    oled.setCursor(24, 2); oled.print(F("DINOSAUR GAME"));
    // oled.setCursor(24, 3); oled.print(F("NEW GAME NAME"));   // пустые заготовки
    // oled.setCursor(24, 4); oled.print(F("NEW GAME NAME"));
    // oled.setCursor(24, 5); oled.print(F("NEW GAME NAME"));
    // oled.setCursor(24, 6); oled.print(F("NEW GAME NAME"));
    // oled.setCursor(24, 7); oled.print(F("NEW GAME NAME"));

    oled.setCursor(0, menuPtr); oled.print('>');
    oled.setCursor(122, menuPtr); oled.print('<');
    oled.home(); oled.print(F("BRIGHT: "));
    oled.print(EEPROM[BRIGHT_EE_ADDR]); oled.print(" % ");
    batCheckDraw();
    oled.update();
  }

  if (millis() - globalSleepTimer > SLEEP_TIMEOUT) {
    goToSleep();
  }
}

/* -----------------------------------------------------  System ----------------------------------------------------- */

/* This interrupt is triggered by ANY action of ANY button */
ISR(PCINT1_vect) {
  globalSleepTimer = millis();
}

