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

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(1) ),
  TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(2) )
};

struct ButtonState {
  bool lastState;
  unsigned long lastDebounce;
};

ButtonState btnMute = {HIGH, 0}, btnTab = {HIGH, 0};
ButtonState b1 = {HIGH, 0}, b2 = {HIGH, 0}, b3 = {HIGH, 0}, b4 = {HIGH, 0};

// ===== SUNETE =====
void soundSwitch() {
  if (!buzzerEnabled) return;
  for (int i = 100; i < 500; i += 20) { tone(BUZZER_PIN, i); delay(10); }
  noTone(BUZZER_PIN);
}

void soundVolUp() { if (buzzerEnabled) tone(BUZZER_PIN, 400, 35); }
void soundVolDown() { if (buzzerEnabled) tone(BUZZER_PIN, 150, 45); }

// ===== FUNCTII HID =====
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

void setTabColor() {
  switch (currentTab) {
    case 0: pixels.setPixelColor(0, pixels.Color(0, 0, 255));   break;
    case 1: pixels.setPixelColor(0, pixels.Color(0, 255, 0));   break;
    case 2: pixels.setPixelColor(0, pixels.Color(255, 0, 0));   break;
    case 3: pixels.setPixelColor(0, pixels.Color(0, 255, 255)); break;
    case 4: pixels.setPixelColor(0, pixels.Color(200, 200, 200)); break;
    case 5: pixels.setPixelColor(0, pixels.Color(128, 0, 128)); break;
  }
  pixels.show();
}

void setup() {
  // Pentru RP2040/ESP32 e nevoie de inițializarea dimensiunii EEPROM
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
  buzzerEnabled = (bState != 0); // Dacă e 255 (nou) sau 1, e ON. Doar 0 e OFF.

  setTabColor();
  lastEncA = digitalRead(ENC_A);
}

void loop() {
  // --- MUTE / TOGGLE BUZZER (2 SEC) ---
  static unsigned long mutePressTime = 0;
  bool muteState = digitalRead(MUTE_BUTTON);
  if (muteState == LOW) {
    if (mutePressTime == 0) mutePressTime = millis();
    if (millis() - mutePressTime > 2000) {
      buzzerEnabled = !buzzerEnabled;
      EEPROM.write(1, buzzerEnabled ? 1 : 0);
      EEPROM.commit(); // FORȚEAZĂ SALVAREA PE FLASH
      
      for(int i=0; i<2; i++){
        pixels.setPixelColor(0, pixels.Color(255,0,0)); pixels.show(); delay(200);
        pixels.setPixelColor(0, 0); pixels.show(); delay(200);
      }
      setTabColor();
      mutePressTime = millis();
    }
  } else {
    if (mutePressTime > 0 && millis() - mutePressTime < 2000) {
      sendConsumer(HID_USAGE_CONSUMER_MUTE);
    }
    mutePressTime = 0;
  }

  // --- SCHIMBARE TAB ---
  if (checkButton(TAB_BUTTON, btnTab)) {
    currentTab = (currentTab + 1) % 6;
    EEPROM.write(0, currentTab);
    EEPROM.commit(); // FORȚEAZĂ SALVAREA
    setTabColor();
    soundSwitch();
  }

  // --- VOLUM, MACRO & PIAN (Rămân neschimbate) ---
  int curA = digitalRead(ENC_A);
  if (curA != lastEncA) {
    if (curA == LOW) {
      if (digitalRead(ENC_B) == LOW) tickCounter++; else tickCounter--;
      if (abs(tickCounter) >= 3) {
        if (millis() - lastVolTime > VOL_COOLDOWN) {
          if (tickCounter > 0) { sendConsumer(HID_USAGE_CONSUMER_VOLUME_INCREMENT); soundVolUp(); }
          else { sendConsumer(HID_USAGE_CONSUMER_VOLUME_DECREMENT); soundVolDown(); }
          lastVolTime = millis();
        }
        tickCounter = 0;
      }
    }
    lastEncA = curA;
  }

  if (checkButton(BTN_1, b1)) {
    if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_L);
    else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_UP);
    else if (currentTab == 2) sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_C);
    else if (currentTab == 3) sendKey(0, HID_KEY_H);
    else if (currentTab == 4) sendKey(0, HID_KEY_1);
    else if (currentTab == 5) tone(BUZZER_PIN, 131, 250); 
  }
  if (checkButton(BTN_2, b2)) {
    if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_PERIOD);
    else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_DOWN);
    else if (currentTab == 2) sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_X);
    else if (currentTab == 3) sendKey(0, HID_KEY_P);
    else if (currentTab == 4) sendKey(0, HID_KEY_2);
    else if (currentTab == 5) tone(BUZZER_PIN, 147, 250); 
  }
  if (checkButton(BTN_3, b3)) {
    if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_ESCAPE);
    else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_LEFT);
    else if (currentTab == 2) sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_V);
    else if (currentTab == 3) sendKey(0, HID_KEY_Q);
    else if (currentTab == 4) sendKey(0, HID_KEY_3);
    else if (currentTab == 5) tone(BUZZER_PIN, 165, 250); 
  }
  if (checkButton(BTN_4, b4)) {
    if (currentTab == 0) sendKey(KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_D);
    else if (currentTab == 1) sendKey(KEYBOARD_MODIFIER_LEFTGUI, HID_KEY_ARROW_RIGHT);
    else if (currentTab == 2) sendKey(KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_Z);
    else if (currentTab == 3) sendKey(0, HID_KEY_E);
    else if (currentTab == 4) sendKey(0, HID_KEY_4);
    else if (currentTab == 5) tone(BUZZER_PIN, 175, 250); 
  }
}