#include <Keyboard.h>
#include <Adafruit_NeoPixel.h>

// ---------- PINI ----------
const int btnPins[5] = {26, 27, 28, 29, 6};

#define ENC_A 3
#define ENC_B 2
#define ENC_SW 4

#define LED_PIN 7
#define LED_COUNT 8

Adafruit_NeoPixel leds(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int lastEncA = HIGH;
uint16_t rainbowOffset = 0;

// ---------- SETUP ----------
void setup() {
  delay(3000); // SIGURANȚĂ

  for (int i = 0; i < 5; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
  }

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  leds.begin();
  leds.show();

  Keyboard.begin();
}

// ---------- LOOP ----------
void loop() {
  rainbow();

  // ---- BUTOANE ----
  if (digitalRead(btnPins[0]) == LOW) {
    ctrlShiftEsc();
  }
  if (digitalRead(btnPins[1]) == LOW) {
    altF4();
  }
  if (digitalRead(btnPins[2]) == LOW) {
    winD();
  }
  if (digitalRead(btnPins[3]) == LOW) {
    winL();
  }
  if (digitalRead(btnPins[4]) == LOW) {
    altTabTabEnter();
  }

  // ---- ENCODER ROTIRE ----
  int encA = digitalRead(ENC_A);
  if (encA != lastEncA) {
    if (digitalRead(ENC_B) != encA) {
      Keyboard.write(KEY_MEDIA_VOLUME_UP);
      flashAll(0, 0, 255);
    } else {
      Keyboard.write(KEY_MEDIA_VOLUME_DOWN);
      flashAll(255, 0, 0);
    }
  }
  lastEncA = encA;

  // ---- BUTON ENCODER ----
  if (digitalRead(ENC_SW) == LOW) {
    Keyboard.write(KEY_MEDIA_PLAY_PAUSE);
    flashAll(0, 255, 0);
    delay(300);
  }
}

// ---------- MACRO FUNCȚII ----------
void ctrlShiftEsc() {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_ESC);
  delay(100);
  Keyboard.releaseAll();
  flashButton(0);
}

void altF4() {
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press(KEY_F4);
  delay(100);
  Keyboard.releaseAll();
  flashButton(1);
}

void winD() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('d');
  delay(100);
  Keyboard.releaseAll();
  flashButton(2);
}

void winL() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('l');
  delay(100);
  Keyboard.releaseAll();
  flashButton(3);
}

void altTabTabEnter() {
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press(KEY_TAB);
  delay(150);
  Keyboard.release(KEY_TAB);
  delay(150);
  Keyboard.press(KEY_TAB);
  delay(150);
  Keyboard.release(KEY_TAB);
  Keyboard.release(KEY_LEFT_ALT);
  delay(150);
  Keyboard.press(KEY_RETURN);
  delay(100);
  Keyboard.releaseAll();
  flashButton(4);
}

// ---------- LED EFFECTE ----------
void rainbow() {
  for (int i = 0; i < LED_COUNT; i++) {
    uint16_t hue = rainbowOffset + i * 65536L / LED_COUNT;
    leds.setPixelColor(i, leds.gamma32(leds.ColorHSV(hue)));
  }
  leds.show();
  rainbowOffset += 256;
  delay(20);
}

void flashButton(int index) {
  leds.setPixelColor(index, leds.Color(255, 255, 255));
  leds.show();
  delay(150);
}

void flashAll(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds.setPixelColor(i, leds.Color(r, g, b));
  }
  leds.show();
  delay(120);
}
