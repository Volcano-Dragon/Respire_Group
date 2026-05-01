#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- Planet Buttons ----------------
// External pull-up to 5V, pressed = LOW
const int btnPins[8] = {A1, A2, A3, 9, 10, 11, 12, 13};

const char* planetNames[8] = {
  "Mercury", "Venus", "Mars",
  "Jupiter", "Saturn", "Uranus",
  "Neptune", "Moon"
};

const float gRatio[8] = {
  0.38, 0.91, 0.38,
  2.34, 1.06, 0.92,
  1.19, 0.165
};

int selectedPlanet = -1;

// ---------------- Keypad ----------------
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}   // * = decimal, # = calculate
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- State ----------------
String input = "";
bool resultShown = false;     // true after # shows result
bool resetArmed = false;      // becomes true when * pressed after result

// ---------------- UI ----------------
void showSelectPlanet() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Planet");
  lcd.setCursor(0, 1);
  lcd.print("Press Button");

  Serial.println("\n[UI] Select Planet");
}

void showWeightScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Earth Wt:");
  lcd.setCursor(0, 1);
  lcd.print(planetNames[selectedPlanet]);
  lcd.print(" Wt:");

  Serial.print("[UI] Selected: ");
  Serial.println(planetNames[selectedPlanet]);
}

void showEarthValueOnLCD() {
  // "Earth Wt:" is 9 chars, so value starts at col 9
  lcd.setCursor(9, 0);
  lcd.print("       ");   // clear previous area
  lcd.setCursor(9, 0);
  lcd.print(input);
}

void clearLine2KeepLabel() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print(planetNames[selectedPlanet]);
  lcd.print(" Wt:");
}

// ---------------- Button Read ----------------
int readPlanetButton() {
  for (int i = 0; i < 8; i++) {
    if (digitalRead(btnPins[i]) == LOW) {  // pressed = LOW
      delay(25); // debounce
      if (digitalRead(btnPins[i]) == LOW) {

        // IMPORTANT: do not block forever (D13 can be weird)
        unsigned long t0 = millis();
        while (digitalRead(btnPins[i]) == LOW) {
          if (millis() - t0 > 800) break;
        }

        Serial.print("[BTN] Pressed index=");
        Serial.print(i);
        Serial.print(" name=");
        Serial.println(planetNames[i]);

        return i;
      }
    }
  }
  return -1;
}

float stringToFloatSafe(const String &s) {
  if (s.length() == 0) return -1;
  if (s == "0.") return -1;
  if (s == ".") return -1;
  return s.toFloat();
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  for (int i = 0; i < 8; i++) {
    pinMode(btnPins[i], INPUT); // external pull-up resistors to 5V
  }

  showSelectPlanet();
}

// ---------------- Loop ----------------
void loop() {

  // -------- Planet selection mode --------
  if (selectedPlanet < 0) {
    int p = readPlanetButton();
    if (p >= 0) {
      selectedPlanet = p;
      input = "";
      resultShown = false;
      resetArmed = false;

      showWeightScreen();
      showEarthValueOnLCD(); // empty
    }
    return;
  }

  // -------- Keypad mode --------
  char k = keypad.getKey();
  if (!k) return;

  Serial.print("[KEY] ");
  Serial.println(k);

  // If result is shown and user starts typing a number, start fresh
  if (resultShown && (k >= '0' && k <= '9')) {
    input = "";
    resultShown = false;
    resetArmed = false;

    showWeightScreen();      // redraw labels, clear old result visually
    clearLine2KeepLabel();
  }

  // Digits
  if (k >= '0' && k <= '9') {
    if (input.length() < 7) {
      input += k;
      showEarthValueOnLCD();
    }
    return;
  }

  // Decimal (*)
  if (k == '*') {
    if (resultShown) {
      // special: after result, '*' arms reset-to-select-planet
      resetArmed = true;
      Serial.println("[STATE] resetArmed = true (press # to go back)");
      return;
    }

    // normal decimal entry
    if (input.length() == 0) {
      input = "0.";
      showEarthValueOnLCD();
      return;
    }
    if (input.indexOf('.') == -1) {
      input += ".";
      showEarthValueOnLCD();
    }
    return;
  }

  // Calculate / Confirm (#)
  if (k == '#') {

    // After result: * then # => back to select planet
    if (resetArmed) {
      Serial.println("[STATE] Back to planet selection");
      selectedPlanet = -1;
      input = "";
      resultShown = false;
      resetArmed = false;
      showSelectPlanet();
      return;
    }

    float earthWt = stringToFloatSafe(input);
    if (earthWt <= 0) {
      Serial.println("[ERR] Invalid Earth weight, clearing input");
      input = "";
      resultShown = false;
      resetArmed = false;

      showWeightScreen();
      showEarthValueOnLCD();
      clearLine2KeepLabel();
      return;
    }

    float otherWt = earthWt * gRatio[selectedPlanet];

    // Show result on line 2
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(planetNames[selectedPlanet]);
    lcd.print(" Wt:");
    lcd.print(otherWt, 2);

    Serial.print("[CALC] Earth=");
    Serial.print(earthWt);
    Serial.print(" -> ");    
    Serial.print(planetNames[selectedPlanet]);
    Serial.print("=");
    Serial.println(otherWt, 2);

    resultShown = true;
    resetArmed = false;
    return;
  }
}