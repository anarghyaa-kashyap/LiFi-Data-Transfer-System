#include <LiquidCrystal.h>
#include <DHT.h>

// LCD: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// DHT11
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LDR and buzzer
#define LDR_PIN    A0

// ── Hardcoded from diagnostic measurements ──
#define LIGHT_THRESHOLD 900
#define DASH_MIN        600   // ON  > 600ms = dash  (measured: ~908ms)
#define LETTER_GAP_MIN  1000  // OFF > 1190ms = letter done (measured: ~1194ms)
#define WORD_GAP_MIN    2300  // OFF > 2300ms = word boundary
#define NEW_MSG_GAP     3000  // OFF > 3000ms = message complete

// Morse lookup tables
const char* MORSE_TABLE[] = {
  ".-",   "-...", "-.-.", "-..",  ".",
  "..-.", "--.",  "....", "..",   ".---",
  "-.-",  ".-..", "--",   "-.",   "---",
  ".--.", "--.-", ".-.",  "...",  "-",
  "..-",  "...-", ".--",  "-..-", "-.--",
  "--.."
};
const char* MORSE_DIGITS[] = {
  "-----", ".----", "..---", "...--", "....-",
  ".....", "-....", "--...", "---..", "----."
};

char morseToChar(String morse) {
  for (int i = 0; i < 26; i++) {
    if (morse == MORSE_TABLE[i]) return 'A' + i;
  }
  for (int i = 0; i < 10; i++) {
    if (morse == MORSE_DIGITS[i]) return '0' + i;
  }
  return '?';
}

String receivedMessage = "";
String displayMessage  = "";
String currentMorse    = "";
bool   inMessage       = false;

void startNewMessage() {
  receivedMessage = "";
  displayMessage  = "";
  currentMorse    = "";
  inMessage       = true;
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  Serial.println("=== NEW MESSAGE STARTED ===");
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(displayMessage.substring(
    max(0, (int)displayMessage.length() - 16)));
}

void flushLetter() {
  if (currentMorse.length() == 0) return;

  char c = morseToChar(currentMorse);
  receivedMessage += c;
  displayMessage  += c;

  Serial.print("[LETTER] Morse: ");
  Serial.print(currentMorse);
  Serial.print(" -> '");
  Serial.print(c);
  Serial.print("'  |  Message: \"");
  Serial.print(receivedMessage);
  Serial.println("\"");

  currentMorse = "";
  updateLCD();
}

void printMsgComplete() {
  Serial.println("-----------------------");
  Serial.print("[MSG COMPLETE] \"");
  Serial.print(receivedMessage);
  Serial.print("\"");
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  if (!isnan(temp) && !isnan(hum)) {
    Serial.print("  |  Temp: ");
    Serial.print(temp, 1);
    Serial.print("C  Humidity: ");
    Serial.print(hum, 1);
    Serial.println("%");
  } else {
    Serial.println();
  }
  Serial.println("-----------------------");
  Serial.println("Waiting for next message...");
}

void setup() {
  lcd.begin(16, 2);
  dht.begin();
  Serial.begin(9600);

  Serial.println("=== LiFi Morse Receiver ===");
  Serial.println("--- LDR Calibration ---");
  Serial.println("Torch OFF readings:");
  for (int i = 0; i < 5; i++) {
    Serial.print("  LDR: ");
    Serial.println(analogRead(LDR_PIN));
    delay(100);
  }
  Serial.println("Switch torch ON in 2s...");
  delay(2000);
  Serial.println("Torch ON readings:");
  for (int i = 0; i < 5; i++) {
    Serial.print("  LDR: ");
    Serial.println(analogRead(LDR_PIN));
    delay(100);
  }
  Serial.println("-----------------------");
  Serial.print("Current threshold: ");
  Serial.println(LIGHT_THRESHOLD);
  Serial.println("Waiting for message...");
  Serial.println("-----------------------");

  lcd.setCursor(0, 0);
  lcd.print("LiFi Morse Ready");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");
}

void loop() {
  // ── DHT11 every 5 seconds ──────────────────────
  static unsigned long lastDHTRead = 0;
  if (millis() - lastDHTRead > 5000) {
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    lastDHTRead = millis();
    if (!isnan(temp) && !isnan(hum)) {
      lcd.setCursor(0, 1);
      lcd.print("T:");
      lcd.print((int)temp);
      lcd.print("C H:");
      lcd.print((int)hum);
      lcd.print("%  ");
      Serial.print("[ENV] Temp: ");
      Serial.print((int)temp);
      Serial.print("C  Humidity: ");
      Serial.print((int)hum);
      Serial.println("%");
    }
  }

  // ── Morse decoding ─────────────────────────────
  static bool          lastLight  = false;
  static unsigned long stateStart = 0;

  bool nowLight      = analogRead(LDR_PIN) > LIGHT_THRESHOLD;
  unsigned long now  = millis();
  unsigned long duration = now - stateStart;

  // ── Timeout: flush last letter on long silence ──
  if (!nowLight && inMessage && currentMorse.length() > 0
      && duration >= LETTER_GAP_MIN) {
    flushLetter();
  }

  // ── Timeout: end of message on very long silence ──
  if (!nowLight && inMessage && currentMorse.length() == 0
      && receivedMessage.length() > 0 && duration >= NEW_MSG_GAP) {
    printMsgComplete();
    inMessage = false;
  }

  if (nowLight != lastLight) {
    if (lastLight) {
      // ── Light just turned OFF: classify the pulse ──
      if (!inMessage) startNewMessage();

      if (duration >= DASH_MIN) {
        currentMorse += "-";
        Serial.print("[PULSE] DASH        (");
        Serial.print(duration);
        Serial.println("ms)");
      } else {
        currentMorse += ".";
        Serial.print("[PULSE] DOT         (");
        Serial.print(duration);
        Serial.println("ms)");
      }

    } else {
      // ── Light just turned ON: classify the gap ──
      if (duration >= NEW_MSG_GAP) {
        Serial.print("[GAP]   NEW MSG     (");
        Serial.print(duration);
        Serial.println("ms)");
        flushLetter();
        if (receivedMessage.length() > 0) {
          printMsgComplete();
          inMessage = false;
        }

      } else if (duration >= WORD_GAP_MIN) {
        Serial.print("[GAP]   WORD        (");
        Serial.print(duration);
        Serial.println("ms)");
        flushLetter();
        receivedMessage += ' ';
        displayMessage  += ' ';
        Serial.println("[WORD GAP] -> ' '");
        Serial.println("-----------------------");
        Serial.print("[MSG] \"");
        Serial.print(receivedMessage);
        Serial.println("\"");
        Serial.println("-----------------------");
        updateLCD();

      } else if (duration >= LETTER_GAP_MIN) {
        Serial.print("[GAP]   LETTER      (");
        Serial.print(duration);
        Serial.println("ms)");
        flushLetter();

      } else {
        Serial.print("[GAP]   SYMBOL      (");
        Serial.print(duration);
        Serial.println("ms)");
        // within same letter -- do nothing
      }
    }

    lastLight  = nowLight;
    stateStart = now;
  }
}
