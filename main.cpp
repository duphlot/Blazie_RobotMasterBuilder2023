#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// Note definitions
#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978

const int maxn = 100;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
int count, now = 2;
int local[] = {1, 7, 12}, temp = 0;
int button[] = {10, 12, 9, 11, 8, 13};
int loa = 7, gas = A0, checkgas = 0;

struct Pair {
  int fi;
  int se;
};

SoftwareSerial sim(3, 2);
Pair sav[maxn];
Servo servo;
String _buffer;
int _timeout;
String number = "+84938736140";

void setup() {
  servo.attach(6);
  servo.write(180);
  _buffer.reserve(50);
  sim.begin(9600);
  for (int i = 0; i < 7; i++) {
    pinMode(button[i], INPUT_PULLUP);
  }
  pinMode(gas, INPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("welcome!");
  delay(1000);
}

String _readSerial() {
  _timeout = 0;
  while (!sim.available() && _timeout < 12000) {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
  return "";
}

void updateSerial() {
  delay(500);
  while (Serial.available()) {
    sim.write(Serial.read());
  }
  while (sim.available()) {
    Serial.write(sim.read());
  }
}

void SendMessage(String SMS) {
  sim.println("AT+CMGF=1");
  updateSerial();
  sim.println("AT+CMGS=\"+84919818297\"");
  updateSerial();
  sim.print(SMS);
  updateSerial();
  sim.write(26);
}

void RecieveMessage() {
  sim.println("AT");
  updateSerial();
  sim.println("AT+CMGF=1");
  updateSerial();
  sim.println("AT+CNMI=2,2,0,0,0");
  updateSerial();
}

void callNumber() {
  sim.print(F("ATD"));
  sim.print(number);
  sim.print(F(";rn"));
  _buffer = _readSerial();
  Serial.println(_buffer);
}

void sos() {
  int notes[] = {261, 392, 440, 392, 349, 329, 293, 261, 392, 349, 329, 293, 392, 349, 329, 293};
  for (int i = 0; i < 16; i++) {
    tone(loa, notes[i], 200);
    delay(500);
    noTone(loa);
    if (i % 4 == 3) {
      delay(750);
    }
  }
  delay(500);
}

void showtime(int second) {
  lcd.setCursor(0, 1);
  lcd.print(second / 3600);
  lcd.print("h");
  lcd.print(second / 60 % 60);
  lcd.print("m");
  lcd.print(second % 60);
}

void isGasleaked() {
  checkgas = analogRead(gas);
  if (checkgas > 500) {
    Serial.write(checkgas);
    servo.write(180);
    lcd.clear();
    lcd.print("gas leakage");
    lcd.setCursor(0, 1);
    lcd.print("detected");
    SendMessage("gas leakage detected");
    while (1) {
      sos();
    }
  }
}

void count_time(int time) {
  for (int s = 1; s <= time; s++) {
    isGasleaked();
    lcd.setCursor(0, 1);
    lcd.print("                  ");
    showtime(s);
    delay(800);
  }
}

int choose_time() {
  count = 0;
  while (1) {
    isGasleaked();
    if (digitalRead(button[1]) == 0) {
      count++;
      showtime(count);
    }
    if (digitalRead(button[0]) == 0) {
      count = max(count - 1, 0);
      showtime(count);
    }
    delay(50);
    if (digitalRead(button[4]) == 0) {
      return count;
    }
  }
}

void choose(int lo, int hi) {
  if (digitalRead(button[3]) == 0) {
    now = max(now - 1, lo);
    tone(loa, NOTE_A5, 200);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(local[now], 1);
    lcd.print("**");
    delay(700);
  }
  if (digitalRead(button[2]) == 0) {
    now = min(now + 1, hi);
    tone(loa, NOTE_A5, 200);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(local[now], 1);
    lcd.print("**");
    delay(700);
  }
}

void choose_mode() {
  lcd.clear();
  lcd.print("Choose mode!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("high medium low");
  lcd.setCursor(12, 1);
  lcd.print("**");
  now = 2;
  while (1) {
    choose(0, 2);
    isGasleaked();
    if (digitalRead(button[4]) == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("timer     back->");
      delay(3000);
      sav[temp++] = {now, choose_time()};
      if (sav[temp - 1].se != -1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("more mode ?");
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("YES    NO");
        now = 0;
        lcd.setCursor(local[now], 1);
        lcd.print("**");
        while (true) {
          choose(0, 1);
          if (digitalRead(button[4]) == 0 && now <= 0) {
            delay(3000);
            choose_mode();
            exit(0);
          } else if (digitalRead(button[4]) == 0) {
            lcd.clear();
            for (int i = 0; i < temp; i++) {
              lcd.setCursor(0, 0);
              lcd.print("                  ");
              lcd.setCursor(0, 0);
              if (sav[i].fi == 0) {
                servo.write(90);
                lcd.print("high");
              } else if (sav[i].fi == 1) {
                servo.write(90);
                servo.write(60);
                lcd.print("medium");
              } else {
                servo.write(90);
                servo.write(30);
                lcd.print("low");
              }
              count_time(sav[i].se);
            }
            delay(1000);
            lcd.clear();
            servo.write(180);
            lcd.setCursor(0, 0);
            lcd.print(" Cook Finish!!");
            while (1);
          }
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("high medium low");
        lcd.setCursor(12, 1);
        lcd.print("**");
        now = 2;
      }
    }
  }
}

void loop() {
  choose_mode();
  delay(100000);
}
