/*
  Modul 4 - Pemrograman OTA berbasis Blynk (Blynk IoT, core 3.x), dengan sensor putaran

  Datastream di Blynk Template:
    V0 = RPM   (Double,  0-5000)   -> Gauge / Chart
    V1 = PWM   (Integer, 0-100)    -> Slider
    V2 = Arah  (Integer, 0-1)      -> Switch (1 = maju, 0 = mundur)
    V3 = LED   (Integer, 0-1)      -> Switch (LED biru onboard)
    V5 = Versi firmware (String)   -> Label
*/

// ====== ISI DARI BLYNK CONSOLE ======
#define BLYNK_TEMPLATE_ID "TMPL6XVTRM6Gj"
#define BLYNK_TEMPLATE_NAME "iMCLab"
#define BLYNK_AUTH_TOKEN "wDvwScT6pALgb4WVxw63MNUFeKo61h1C"
// ====================================

// ====== OTA (harus di atas #include Blynk) ======
#define BLYNK_FIRMWARE_TYPE    "iMCLab"
#define BLYNK_FIRMWARE_VERSION "1.0.1"     // NAIKKAN setiap membuat update (1.0.1, 1.0.2, ...)
// ================================================

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoOTA.h>        // cadangan OTA lewat WiFi lokal

char ssid[] = "Miaw";      // harus WiFi 2.4 GHz
char pass[] = "miawmiaw";

// ===== Batas PWM: ubah di versi 1.0.1 sebagai bukti update =====
const int PWM_MAX = 90;

// ===== Pin motor =====
const int PIN_EN  = 12;   // enable / PWM kecepatan
const int PIN_IN1 = 27;   // arah 1
const int PIN_IN2 = 26;   // arah 2
const int PIN_LED = 2;    // LED biru onboard

// ===== Sensor putaran =====
const int   PIN_ENC = 13;
const float PULSES_PER_REV = 1.0;        // jumlah pulsa per putaran cakram/baling-baling
const unsigned long MIN_PULSE_US = 200;  // abaikan pulsa yang lebih rapat dari ini (penyaring derau)

// ===== PWM =====
const int PWM_FREQ = 30000;
const int PWM_RES  = 8;

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
  digitalWrite(PIN_IN1, dirNow ? LOW : HIGH);
  digitalWrite(PIN_IN2, dirNow ? HIGH : LOW);
  ledcWrite(PIN_EN, pwmPct * 255 / 100);
}

BLYNK_WRITE(V1) {
  int req = param.asInt();
  pwmPct = constrain(req, 0, PWM_MAX);
  applyMotor();
  if (pwmPct != req) Blynk.virtualWrite(V1, pwmPct);   // slider kembali ke batas
}
BLYNK_WRITE(V2) { dirNow = param.asInt() ? 1 : 0; applyMotor(); }
BLYNK_WRITE(V3) { digitalWrite(PIN_LED, param.asInt() ? HIGH : LOW); }

void kirimRPM() {
  unsigned long now = millis();
  noInterrupts();
  unsigned long p = pulses;
  pulses = 0;
  interrupts();
  float rpm = (p / PULSES_PER_REV) * 60000.0 / (now - lastCalc);
  lastCalc = now;

  Serial.printf("v%s  pulsa=%lu  RPM=%.1f  PWM=%d%%  arah=%d\n",
                BLYNK_FIRMWARE_VERSION, p, rpm, pwmPct, dirNow);
  Blynk.virtualWrite(V0, rpm);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1, V2, V3);
  Blynk.virtualWrite(V5, BLYNK_FIRMWARE_VERSION);
}

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware versi ");
  Serial.println(BLYNK_FIRMWARE_VERSION);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  ledcAttach(PIN_EN, PWM_FREQ, PWM_RES);   // sintaks core 3.x
  applyMotor();

  pinMode(PIN_ENC, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC), onPulse, RISING);

  lastCalc = millis();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  ArduinoOTA.setHostname("imclab");        // cadangan OTA lewat WiFi lokal
  ArduinoOTA.onStart([]() { pwmPct = 0; applyMotor(); });   // motor berhenti saat update
  ArduinoOTA.begin();

  timer.setInterval(1000L, kirimRPM);
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle();
  timer.run();
}
