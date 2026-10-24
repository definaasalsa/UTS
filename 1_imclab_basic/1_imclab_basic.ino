
/*
  Modul 1 - iMCLab Basic
  Baca suhu T1/T2, kontrol heater Q1/Q2 dan LED lewat Serial Monitor.

  Perintah Serial (115200 baud, Newline):
    T1       -> baca suhu sensor 1
    T2       -> baca suhu sensor 2
    Q1 50    -> atur heater 1 (0-100%)
    Q2 50    -> atur heater 2 (0-100%)
    LED 100  -> atur LED (0-100%)
    VER      -> versi firmware
    X        -> matikan semua heater

  Kompatibel dengan ESP32 Arduino Core versi 3.x.
  Pastikan pin sesuai dengan skematik kit iMCLab.
*/

const int pinT1   = 34;  // Sensor suhu 1 (ADC)
const int pinT2   = 35;  // Sensor suhu 2 (ADC)
const int pinQ1   = 32;  // Heater 1 (PWM)
const int pinQ2   = 33;  // Heater 2 (PWM)
const int pinLED1 = 26;  // LED indikator (PWM)
const int pinLED2 = 27;  // LED indikator digital

const float TEMP_MAX = 80.0;  // Batas suhu maksimum (°C)

const int PWM_FREQ = 5000;
const int PWM_RES  = 8;       // Resolusi 8-bit: 0-255

// Membaca suhu sensor
float readTemp(int pin) {
  long sum = 0;

  for (int i = 0; i < 20; i++) {
    sum += analogRead(pin);
  }

  float raw  = sum / 20.0;
  float volt = raw / 4095.0 * 3.3;

  // Asumsi sensor LM35: 10 mV per °C
  return volt * 100.0;
}

// Mengatur output PWM dalam persen (0-100%)
void setPct(int pin, int pct) {
  pct = constrain(pct, 0, 100);
  int duty = pct * 255 / 100;

  ledcWrite(pin, duty);
}

// Mematikan semua heater
void allOff() {
  setPct(pinQ1, 0);
  setPct(pinQ2, 0);
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1000);

  analogReadResolution(12);

  // Konfigurasi PWM untuk ESP32 Core 3.x
  bool pwmQ1   = ledcAttach(pinQ1, PWM_FREQ, PWM_RES);
  bool pwmQ2   = ledcAttach(pinQ2, PWM_FREQ, PWM_RES);
  bool pwmLED1 = ledcAttach(pinLED1, PWM_FREQ, PWM_RES);

  pinMode(pinLED2, OUTPUT);
  digitalWrite(pinLED2, LOW);

  // Pastikan heater mati saat awal
  allOff();

  if (!pwmQ1 || !pwmQ2 || !pwmLED1) {
    Serial.println("ERROR: Gagal menginisialisasi PWM!");
    return;
  }

  Serial.println("iMCLab Basic 1.0 siap.");
  Serial.println("Perintah: VER, T1, T2, Q1 50, Q2 50, LED 100, X");
}

void loop() {
  // Baca suhu terbaru
  float t1 = readTemp(pinT1);
  float t2 = readTemp(pinT2);

  // Pengaman suhu: matikan heater jika batas terlampaui
  if (t1 >= TEMP_MAX || t2 >= TEMP_MAX) {
    allOff();
  }

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    line.toUpperCase();

    if (line == "VER") {
      Serial.println("iMCLab Basic 1.0");

    } else if (line == "T1") {
      Serial.println(t1, 2);

    } else if (line == "T2") {
      Serial.println(t2, 2);

    } else if (line == "X") {
      allOff();
      Serial.println("Stop");

    } else if (line.startsWith("Q1 ")) {
      int v = line.substring(3).toInt();

      if (t1 >= TEMP_MAX || t2 >= TEMP_MAX) {
        allOff();
        Serial.println("ERROR: Suhu maksimum tercapai!");
      } else {
        setPct(pinQ1, v);
        Serial.println(constrain(v, 0, 100));
      }

    } else if (line.startsWith("Q2 ")) {
      int v = line.substring(3).toInt();

      if (t1 >= TEMP_MAX || t2 >= TEMP_MAX) {
        allOff();
        Serial.println("ERROR: Suhu maksimum tercapai!");
      } else {
        setPct(pinQ2, v);
        Serial.println(constrain(v, 0, 100));
      }

    } else if (line.startsWith("LED ")) {
      int v = line.substring(4).toInt();
      setPct(pinLED1, v);
      Serial.println(constrain(v, 0, 100));

    } else if (line.length() > 0) {
      Serial.println("Perintah tidak dikenal");
    }
  }
}
