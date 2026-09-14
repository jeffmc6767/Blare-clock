#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#define TFT_SCLK 0
#define TFT_MOSI 1
#define TFT_RST 2
#define TFT_DC 3
#define TFT_CS 4
#define TFT_BL 5

#define BTN_MODE 6
#define BTN_UP 7
#define BTN_DOWN 8
#define BTN_ALARM 9
#define BUZZER_PIN 10

class MyST7789 : public Adafruit_ST7789 {
public:
  MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
    : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
  void setOffsets(uint8_t col, uint8_t row) {
    _colstart = _colstart2 = col;
    _rowstart = _rowstart2 = row;
  }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int hours = 12;
int minutes = 0;
int seconds = 0;

int alarmHours = 7;
int alarmMinutes = 0;
bool alarmEnabled = false;
bool alarmTriggered = false;

enum EditMode { RUNNING, SET_HOURS, SET_MINUTES, SET_ALARM_HOURS, SET_ALARM_MINUTES };
EditMode currentMode = RUNNING;

unsigned long lastTick = 0;
unsigned long lastBlink = 0;
bool blinkState = true;

bool lastModeBtn = HIGH;
bool lastUpBtn = HIGH;
bool lastDownBtn = HIGH;
bool lastAlarmBtn = HIGH;

void setup() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_ALARM, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  tft.init(76, 284);
  tft.setOffsets(82, 18);
  tft.invertDisplay(false);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
}

void loop() {
  bool modeBtn = digitalRead(BTN_MODE);
  bool upBtn = digitalRead(BTN_UP);
  bool downBtn = digitalRead(BTN_DOWN);
  bool alarmBtn = digitalRead(BTN_ALARM);

  if (alarmTriggered) {
    if (modeBtn == LOW || upBtn == LOW || downBtn == LOW || alarmBtn == LOW) {
      alarmTriggered = false;
      noTone(BUZZER_PIN);
      delay(200);
    }
  }

  if (modeBtn == LOW && lastModeBtn == HIGH) {
    currentMode = (EditMode)((currentMode + 1) % 5);
    delay(50);
  }

  if (alarmBtn == LOW && lastAlarmBtn == HIGH) {
    alarmEnabled = !alarmEnabled;
    delay(50);
  }

  if (upBtn == LOW && lastUpBtn == HIGH) {
    if (currentMode == SET_HOURS) hours = (hours % 24) + 1;
    if (currentMode == SET_MINUTES) minutes = (minutes + 1) % 60;
    if (currentMode == SET_ALARM_HOURS) alarmHours = (alarmHours % 24) + 1;
    if (currentMode == SET_ALARM_MINUTES) alarmMinutes = (alarmMinutes + 1) % 60;
    delay(50);
  }

  if (downBtn == LOW && lastDownBtn == HIGH) {
    if (currentMode == SET_HOURS) hours = (hours == 1) ? 24 : hours - 1;
    if (currentMode == SET_MINUTES) minutes = (minutes == 0) ? 59 : minutes - 1;
    if (currentMode == SET_ALARM_HOURS) alarmHours = (alarmHours == 1) ? 24 : alarmHours - 1;
    if (currentMode == SET_ALARM_MINUTES) alarmMinutes = (alarmMinutes == 0) ? 59 : alarmMinutes - 1;
    delay(50);
  }

  lastModeBtn = modeBtn;
  lastUpBtn = upBtn;
  lastDownBtn = downBtn;
  lastAlarmBtn = alarmBtn;

  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    seconds++;
    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours++;
        if (hours > 24) hours = 1;
      }
    }

    if (alarmEnabled && hours == alarmHours && minutes == alarmMinutes && seconds == 0) {
      alarmTriggered = true;
    }
  }

  if (alarmTriggered) {
    if ((millis() / 250) % 2 == 0) {
      tone(BUZZER_PIN, 1000);
    } else {
      noTone(BUZZER_PIN);
    }
  }

  if (millis() - lastBlink >= 300) {
    lastBlink = millis();
    blinkState = !blinkState;
  }

  char timeBuf[9];
  if (currentMode == SET_HOURS && !blinkState) {
    snprintf(timeBuf, sizeof(timeBuf), "  :%02d:%02d", minutes, seconds);
  } else if (currentMode == SET_MINUTES && !blinkState) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:  :%02d", hours, seconds);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", hours, minutes, seconds);
  }

  char alarmBuf[9];
  if (currentMode == SET_ALARM_HOURS && !blinkState) {
    snprintf(alarmBuf, sizeof(alarmBuf), "  :%02d", alarmMinutes);
  } else if (currentMode == SET_ALARM_MINUTES && !blinkState) {
    snprintf(alarmBuf, sizeof(alarmBuf), "%02d:  ", alarmHours);
  } else {
    snprintf(alarmBuf, sizeof(alarmBuf), "%02d:%02d", alarmHours, alarmMinutes);
  }

  tft.fillRect(10, 15, 260, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(10, 15);
  tft.print(timeBuf);

  tft.fillRect(10, 50, 260, 20, ST77XX_BLACK);
  tft.setTextSize(2);
  if (alarmEnabled) {
    tft.setTextColor(ST77XX_YELLOW);
  } else {
    tft.setTextColor(0x7BEF);
  }
  tft.setCursor(10, 50);
  tft.print(alarmBuf);
}
