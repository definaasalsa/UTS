/*
  Modul 3 - Pemrograman iMCLab berbasis Blynk (Blynk IoT, core 3.x)

  Datastream di Blynk Template:
    V0 = RPM   (Double,  0-5000)   -> Gauge / Chart
    V1 = PWM   (Integer, 0-100)    -> Slider
    V2 = Arah  (Integer, 0-1)      -> Switch (1 = maju, 0 = mundur)
    V3 = LED   (Integer, 0-1)      -> Switch (LED biru onboard)

  Library: "Blynk" by Volodymyr Shymanskyy | Board: ESP32 Dev Module
*/


//

// ====== ISI DARI BLYNK CONSOLE ======
#define BLYNK_TEMPLATE_ID "TMPL6XVTRM6Gj"
#define BLYNK_TEMPLATE_NAME "iMCLab"
#define BLYNK_AUTH_TOKEN "wDvwScT6pALgb4WVxw63MNUFeKo61h1C"
// ====================================

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Miaw";      // harus WiFi 2.4 GHz
char pass[] = "miawmiaw";

// ===== Pin motor =====
const int PIN_EN  = 12;   // enable / PWM kecepatan
const int PIN_IN1 = 27;   // arah 1
const int PIN_IN2 = 26;   // arah 2
const int PIN_LED = 2;    // LED biru onboard

// ===== Sensor putaran =====
const int   PIN_ENC = 13;
const float PULSES_PER_REV = 1.0;        // jumlah pulsa per putaran cakram
const unsigned long MIN_PULSE_US = 0






;  // abaikan pulsa yang lebih rapat dari ini (penyaring derau)

// ===== PWM =====
const int PWM_FREQ  = 30000;
const int PWM_RES   = 8;
const int PWM_START = 0;   // isi 80 agar slider 1-100 dipetakan ke 80-100 (melewati zona mati)

volatile unsigned long pulses = 0;
volatile unsigned long lastPulseUs = 0;

void IRAM_ATTR onPulse() {
  unsigned long t = micros();
  if (t - lastPulseUs >= MIN_PULSE_US) {
    pulses++;
    lastPulseUs = t;
  }
}

int pwmPct = 0;
int dirNow = 1;
unsigned long lastCalc = 0;
BlynkTimer timer;

void applyMotor() {
  int efektif = 0;
  if (pwmPct > 0) {
    efektif = PWM_START + pwmPct * (100 - PWM_START) / 100;
  }
  digitalWrite(PIN_IN1, dirNow ? LOW : HIGH);
  digitalWrite(PIN_IN2, dirNow ? HIGH : LOW);
  ledcWrite(PIN_EN, efektif * 255 / 100);
}

BLYNK_WRITE(V1) { pwmPct = constrain(param.asInt(), 0, 100); applyMotor(); }
BLYNK_WRITE(V2) { dirNow = param.asInt() ? 1 : 0; applyMotor(); }
BLYNK_WRITE(V3) { digitalWrite(PIN_LED, param.asInt() ? HIGH : LOW); }

void kirimRPM() {
  float rpm = 0;
  unsigned long p = 0;
  unsigned long now = millis();
  if (PIN_ENC >= 0) {
    noInterrupts();
    p = pulses;
    pulses = 0;
    interrupts();
    rpm = (p / PULSES_PER_REV) * 60000.0 / (now - lastCalc);
  }
  lastCalc = now;
  Serial.printf("pulsa=%lu  RPM=%.1f  PWM=%d%%  arah=%d\n", p, rpm, pwmPct, dirNow);
  Blynk.virtualWrite(V0, rpm);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1, V2, V3);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  ledcAttach(PIN_EN, PWM_FREQ, PWM_RES);   // sintaks core 3.x
  applyMotor();

  if (PIN_ENC >= 0) {
    pinMode(PIN_ENC, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC), onPulse, RISING);
  }

  lastCalc = millis();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, kirimRPM);
}

void loop() {
  Blynk.run();
  timer.run();
}