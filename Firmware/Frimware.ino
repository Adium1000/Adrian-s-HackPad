//    ______     _                                 
//   / ____/____(_)___ ___ _      ______ _________       / / / /___ ______/ /__/ __ \____ _____/ /         
//  / /_  / ___/ / __ `__ \ | /| / / __ `/ ___/ _ \     / /_/ / __ `/ ___/ //_/ /_/ / __ `/ __  /   
// / __/ / /  / / / / / / / |/ |/ / /_/ / /  /  __/    / __  / /_/ / /__/ ,< / ____/ /_/ / /_/ / 
///_/ __/_/_ /_/_/ /_/ /_/|__/|__/\__,_/_/  _\___/    /_/ /_/\__,_/\___/_/|_/_/    \__,_/\__,_/     
    
// Firmware build 4 from 21.06.26

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#define MUTE_BUTTON 4   
#define ENC_A 3          
#define ENC_B 2          
#define LED_PIN 12
#define LED_POWER 11
#define TAB_BUTTON 6    
#define BUZZER_PIN 7

#define BTN_1 26
#define BTN_2 27
#define BTN_3 28
#define BTN_4 29

#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_USBD_HID usb_hid;

int currentTab = 0; 
bool buzzerEnabled = true;
int lastEncA = HIGH;
int tickCounter = 0;
unsigned long lastVolTime = 0;
const unsigned long VOL_COOLDOWN = 60;
unsigned long lastRepeatTime = 0; 

uint8_t targetR = 0, targetG = 0, targetB = 0;
uint8_t baseR = 0, baseG = 0, baseB = 0; // Culoarea de bază a tab-ului
uint8_t currentR = 0, currentG = 0, currentB = 0;
unsigned long lastLEDUpdate = 0;
const int transitionSpeed = 2; // Mai mic = Morph mult mai rapid

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(1) ),
  TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(2) )
};

struct ButtonState {
  bool lastState;
  unsigned long lastDebounce;
  unsigned long pressStartTime;
  bool repeatMode;
};

ButtonState btnMute = {HIGH, 0, 0, false}, btnTab = {HIGH, 0, 0, false};
ButtonState b1 = {HIGH, 0, 0, false}, b2 = {HIGH, 0, 0, false}, b3 = {HIGH, 0, 0, false}, b4 = {HIGH, 0, 0, false};

void soundSwitch() {
  if (!buzzerEnabled) return;
  for (int i = 100; i < 500; i += 20) { tone(BUZZER_PIN, i); delay(10); }
  noTone(BUZZER_PIN);
}
void soundVolUp() { if (buzzerEnabled) tone(BUZZER_PIN, 400, 35); }
void soundVolDown() { if (buzzerEnabled) tone(BUZZER_PIN, 150, 45); }
void soundDrrr() { if (buzzerEnabled) tone(BUZZER_PIN, 150, 20); }
void soundKnobClick() { if (buzzerEnabled) tone(BUZZER_PIN, 800, 30); }

void sendConsumer(uint16_t code) {
  if (!usb_hid.ready()) return;
  usb_hid.sendReport(2, &code, 2);
  delay(10);
  uint16_t empty = 0;
  usb_hid.sendReport(2, &empty, 2);
}

void sendKey(uint8_t modifier, uint8_t key) {
  if (!usb_hid.ready()) return;
  uint8_t keycode[6] = { key, 0, 0, 0, 0, 0 };
  usb_hid.keyboardReport(1, modifier, keycode);
  delay(50);
  usb_hid.keyboardRelease(1);
}

bool checkButton(uint8_t pin, ButtonState &btn) {
  bool state = digitalRead(pin);
  if (state != btn.lastState && (millis() - btn.lastDebounce) > 50) {
    btn.lastDebounce = millis();
    btn.lastState = state;
    if (state == LOW) return true;
  }
  btn.lastState = state;
  return false;
}

uint8_t clamp(int val) {
  if (val > 255) return 255;
  if (val < 0) return 0;
  return (uint8_t)val;
}

void setTabColor() {
  switch (currentTab) {
    case 0: baseR = 0;   baseG = 0;   baseB = 255; break; 
    case 1: baseR = 0;   baseG = 255; baseB = 0;   break; 
    case 2: baseR = 255; baseG = 0;   baseB = 0;   break; 
    case 3: baseR = 0;   baseG = 255; baseB = 255; break; 
    case 4: baseR = 200; baseG = 200; baseB = 200; break; 
    case 5: baseR = 128; baseG = 0;   baseB = 128; break; 
  }
  targetR = baseR; targetG = baseG; targetB = baseB;
}

void updateLED() {
  if (millis() - lastLEDUpdate > transitionSpeed) {
    lastLEDUpdate = millis();
    bool changed = false;

    if (currentR < targetR) { currentR++; changed = true; } else if (currentR > targetR) { currentR--; changed = true; }
    if (currentG < targetG) { currentG++; changed = true; } else if (currentG > targetG) { currentG--; changed = true; }
    if (currentB < targetB) { currentB++; changed = true; } else if (currentB > targetB) { currentB--; changed = true; }

    if (!changed) {
      targetR = baseR; targetG = baseG; targetB = baseB;
    }

    pixels.setPixelColor(0, pixels.Color(currentR, currentG, currentB));
    pixels.show();
  }
}

void setup() {
  EEPROM.begin(256);
  pinMode(MUTE_BUTTON, INPUT_PULLUP);
  pinMode(ENC_A, INPUT_PULLUP); pinMode(ENC_B, INPUT_PULLUP);
  pinMode(TAB_BUTTON, INPUT_PULLUP);
  pinMode(BTN_1, INPUT_PULLUP); pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP); pinMode(BTN_4, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_POWER, OUTPUT); digitalWrite(LED_POWER, HIGH);

  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();
  pixels.begin();
  
  currentTab = EEPROM.read(0); if (currentTab > 5) currentTab = 0;
  uint8_t bState = EEPROM.read(1);
  buzzerEnabled = (bState != 0);

  setTabColor();
  currentR = baseR; currentG = baseG; currentB = baseB;
  pixels.setPixelColor(0, pixels.Color(currentR, currentG, currentB));
  pixels.show();

  lastEncA = digitalRead(ENC_A);
}

void loop() {
  updateLED();

  static unsigned long mutePressTime = 0;
  if (digitalRead(MUTE_BUTTON) == LOW) {
    if (mutePressTime == 0) mutePressTime = millis();
    if (millis() - mutePressTime > 2000) { 
      buzzerEnabled = !buzzerEnabled;
      EEPROM.write(1, buzzerEnabled ? 1 : 0);
      EEPROM.commit();
      for(int i=0; i<2; i++){
        pixels.setPixelColor(0, pixels.Color(255,0,0)); pixels.show(); delay(200);
        pixels.setPixelColor(0, 0); pixels.show(); delay(200);
      }
      setTabColor();
      mutePressTime = millis();
    }
  } else {
    if (mutePressTime > 0 && millis() - mutePressTime < 2000) {
      soundKnobClick();
      if (currentTab == 1 || currentTab == 2) sendKey(0, HID_KEY_ENTER); 
      else sendConsumer(HID_USAGE_CONSUMER_MUTE);
    }
    mutePressTime = 0;
  }

  if (checkButton(TAB_BUTTON, btnTab)) {
    currentTab = (currentTab + 1) % 6;
    EEPROM.write(0, currentTab); EEPROM.commit();
    setTabColor(); 
    soundSwitch();
  }

  int curA = digitalRead(ENC_A);
  if (curA != lastEncA) {
    if (curA == LOW) {
      bool up = (digitalRead(ENC_B) == LOW);
      if (up) tickCounter++; else tickCounter--;
      
      if (abs(tickCounter) >= 1) {
        if (millis() - lastVolTime > VOL_COOLDOWN) {
          int offset = (tickCounter > 0) ? 50 : -50;
          targetR = clamp(baseR + offset);
          targetG = clamp(baseG + offset);
          targetB = clamp(baseB + offset);

          if (currentTab == 1) { 
            if (tickCounter > 0) { sendKey(0, HID_KEY_ARROW_RIGHT); soundVolUp(); }
            else { sendKey(0, HID_KEY_ARROW_LEFT); soundVolDown(); }
          } else if (currentTab == 2) { 
            if (tickCounter > 0) { sendKey(0, HID_KEY_ARROW_DOWN); soundVolUp(); }
            else { sendKey(0, HID_KEY_ARROW_UP); soundVolDown(); }
          } else { 
            if (tickCounter > 0) { sendConsumer(HID_USAGE_CONSUMER_VOLUME_INCREMENT); soundVolUp(); }
            else { sendConsumer(HID_USAGE_CONSUMER_VOLUME_DECREMENT); soundVolDown(); }
          }
          lastVolTime = millis();
        }
        tickCounter = 0;
      }
    }
    lastEncA = curA;
  }

  if (currentTab == 2) {
    bool s3 = digitalRead(BTN_3);
    if (s3 == LOW) {
      if (b3.pressStartTime == 0) b3.pressStartTime = millis();
      if (millis() - b3.pressStartTime > 400) {
        if (millis() - lastRepeatTime > 100) {
          sendKey(KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_ARROW_LEFT);
          soundDrrr();
          lastRepeatTime = millis();
          b3.repeatMode = true;
        }
      }
    } else {
      if (b3.pressStartTime > 0 && !b3.repeatMode) {
        sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_V);
        if (buzzerEnabled) tone(BUZZER_PIN, 400, 30);
      }
      b3.pressStartTime = 0; b3.repeatMode = false;
    }

    bool s4 = digitalRead(BTN_4);
    if (s4 == LOW) {
      if (b4.pressStartTime == 0) b4.pressStartTime = millis();
      if (millis() - b4.pressStartTime > 400) {
        if (millis() - lastRepeatTime > 100) {
          sendKey(KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_ARROW_RIGHT);
          soundDrrr();
          lastRepeatTime = millis();
          b4.repeatMode = true;
        }
      }
    } else {
      if (b4.pressStartTime > 0 && !b4.repeatMode) {
        sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_Z);
        if (buzzerEnabled) tone(BUZZER_PIN, 400, 30);
      }
      b4.pressStartTime = 0; b4.repeatMode = false;
    }
  }

  if (checkButton(BTN_1, b1)) {
    if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_L);
    else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_UP);
    else if (currentTab == 2) { sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_C); if(buzzerEnabled) tone(BUZZER_PIN, 400, 30); }
    else if (currentTab == 3) sendKey(0, HID_KEY_H);
    else if (currentTab == 4) sendKey(0, HID_KEY_1);
    else if (currentTab == 5) tone(BUZZER_PIN, 131, 250); 
  }
  if (checkButton(BTN_2, b2)) {
    if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_PERIOD);
    else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_DOWN);
    else if (currentTab == 2) { sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_X); if(buzzerEnabled) tone(BUZZER_PIN, 400, 30); }
    else if (currentTab == 3) sendKey(0, HID_KEY_P);
    else if (currentTab == 4) sendKey(0, HID_KEY_2);
    else if (currentTab == 5) tone(BUZZER_PIN, 147, 250); 
  }
  
  if (currentTab != 2) {
    if (checkButton(BTN_3, b3)) {
      if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_ESCAPE);
      else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_LEFT);
      else if (currentTab == 3) sendKey(0, HID_KEY_Q);
      else if (currentTab == 4) sendKey(0, HID_KEY_3);
      else if (currentTab == 5) tone(BUZZER_PIN, 165, 250);
    }
    if (checkButton(BTN_4, b4)) {
      if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_D);
      else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_RIGHT);
      else if (currentTab == 3) sendKey(0, HID_KEY_E);
      else if (currentTab == 4) sendKey(0, HID_KEY_4);
      else if (currentTab == 5) tone(BUZZER_PIN, 175, 250);
    }
  }
}