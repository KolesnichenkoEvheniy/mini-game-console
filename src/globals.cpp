#include "globals.h"

GyverOLED <SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;
GButton ok(BUTTON_OK);
GButton up(BUTTON_UP);
GButton down(BUTTON_DOWN);
GButton left(BUTTON_LEFT);
GButton right(BUTTON_RIGHT);

uint32_t globalSleepTimer = 0;

void oledPower(bool state) {
  if (state) {
    pinMode(OLED_VCC1, OUTPUT);
    pinMode(OLED_VCC0, OUTPUT);
    digitalWrite(OLED_VCC1, HIGH);
    digitalWrite(OLED_VCC0, HIGH);
    _delay_ms(15);
    oled.init();
    oled.setContrast(map(EEPROM[BRIGHT_EE_ADDR], 0, 100, 0, 255));
  } else {
    for (uint8_t i = EEPROM[BRIGHT_EE_ADDR]; i; i--) {
      oled.setContrast(i);
      _delay_ms(10);
    }
    oled.setPower(false);
    digitalWrite(OLED_VCC1, LOW);
    digitalWrite(OLED_VCC0, LOW);
    pinMode(OLED_VCC1, INPUT);
    pinMode(OLED_VCC0, INPUT);
  }
}

void goToSleep(void) {
  bool wakeup;
  uint32_t timer;
  oledPower(false);
  // Interrupt and wake up only on "OK"
  PCMSK1 = 1 << PCINT12;
  while (true) {
    power.sleep(SLEEP_FOREVER);
    wakeup = false;
    timer = millis();
    while (ok.state()) {
      if (millis() - timer > WAKEUP_PRESS) { // hold button long enough
        wakeup = true;
      }
    } if (wakeup) break;
  }
  // Interruprions by buttons
  PCMSK1 = 1 << PCINT8 | 1 << PCINT9 | 1 << PCINT10 | 1 << PCINT11 | 1 << PCINT12;
  oledPower(true);
}

void batCheckDraw(void) {
  static uint32_t measureTimer = millis() + 3500;
  static uint8_t batCharge = 0;

  if (millis() - measureTimer >= 3000) {
    measureTimer = millis();
    // We measure the supply voltage + average
    ADCSRA |= 1 << ADSC;
    while (ADCSRA & (1 << ADSC));
    // We convert the voltage into a conditional charge
    batCharge = constrain(map((INTERNAL_REF * 1024UL) / ADC, BATTERY_EMPTY, BATTERY_FULL, 0, 12), 0, 12);
  }

  // Drawing the battery level
  oled.setCursorXY(110, 0);
  oled.drawByte(0b00111100);
  oled.drawByte(0b00111100);
  oled.drawByte(0b11111111);
  for (uint8_t i = 0; i < 12; i++) {
    if (i < 12 - batCharge) oled.drawByte(0b10000001);
    else oled.drawByte(0b11111111);
  } oled.drawByte(0b11111111);
}

void resetButtonsSetup(void) {
  ok.resetStates();
  left.resetStates();
  right.resetStates();
  up.resetStates();
  down.resetStates();

  ok.setStepTimeout(100);
  up.setStepTimeout(100);
  down.setStepTimeout(100);
  left.setStepTimeout(100);
  right.setStepTimeout(100);
  
  ok.setTimeout(500);
  left.setTimeout(500);
  right.setTimeout(500);
  up.setTimeout(500);
  down.setTimeout(500);

  ok.setStepTimeout(400);
}