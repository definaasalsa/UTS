# Laporan Praktikum UTS Robotika: Eksplorasi Kit iMCLab

Nama: Defina Aulia Salsabila

NIM: 23081010280

Mata Kuliah: Robotika

---

## 1. Gambaran Umum

Dalam praktikum ini saya bekerja dengan **iMCLab** (*Programmable Internet-Based Motor Control Lab*), sebuah kit pembelajaran berbasis ESP32 yang dilengkapi driver dan motor DC. Saya mengerjakan empat tahap yang saling berkesinambungan:

1. Mengontrol motor secara langsung lewat perintah serial.
2. Mengotomatiskan pengujian dengan skrip Python.
3. Memindahkan kendali ke internet menggunakan dashboard Blynk.
4. Memperbarui firmware tanpa kabel (OTA) pada sistem yang sudah terhubung ke Blynk.

Keempat tahap berjalan sesuai tujuan. Satu hal yang perlu dicatat sejak awal: **RPM tidak berhasil diukur**. Baling-baling pada kit yang saya pakai patah, sehingga sensor putaran tidak pernah mendeteksi putaran. Karena itu, analisis kecepatan hanya bersifat kualitatif (motor bergerak atau tidak, ke arah mana).

## 2. Perangkat yang Digunakan

**Perangkat keras**
- Kit iMCLab: ESP32 DevKit, driver motor, motor DC, LED, dan sensor putaran
- Adaptor DC (jack DC1) dan kabel micro USB yang mendukung data

**Perangkat lunak**
- Arduino IDE 2.x dengan Arduino-ESP32 core 3.3.12
- Python 3.x dengan `pyserial` dan `matplotlib`
- Library Blynk v1.4.0 dan ArduinoOTA (sudah termasuk dalam core ESP32)
- Akun Blynk (blynk.cloud) serta jaringan WiFi 2.4 GHz

**Pemetaan pin ESP32**

| Pin | Dipakai untuk |
|---|---|
| GPIO 12 | Enable atau PWM kecepatan motor |
| GPIO 27 | Arah motor (IN1) |
| GPIO 26 | Arah motor (IN2) |
| GPIO 2 | LED biru onboard |
| GPIO 13 | Sensor putaran |

Pin motor saya ambil dari kode referensi dosen (IN1 = 27, IN2 = 26, Enable = 12). Pin sensor putaran tidak tercantum di referensi, sehingga saya mencarinya dengan sketch pemindai pin.

## 3. Tahap 1: Kontrol Dasar lewat Serial

### Yang ingin dicapai
Mengatur kecepatan, arah, dan LED pada iMCLab hanya dengan mengetik perintah di Serial Monitor.

### Konsep di baliknya
- **PWM** mengatur kecepatan dengan cara mengubah lebar pulsa (duty cycle) pada pin enable. Semakin lebar pulsanya, semakin tinggi tegangan rata-rata yang diterima motor.
- **H-bridge** pada driver menentukan arah putar berdasarkan kombinasi logika IN1 dan IN2.
- Perintah dikirim ke ESP32 sebagai teks melalui USB pada 115200 baud.

### Cara saya mengerjakan
1. Memilih board **ESP32 Dev Module**, port yang sesuai, dan Upload Speed **115200**.
2. Mengunggah `imclab_modul1.ino`.
3. Membuka Serial Monitor (115200 baud, line ending *Newline*), lalu mencoba perintah berikut:

| Perintah | Efek |
|---|---|
| `VER` | Menampilkan versi firmware |
| `PWM 80` | Mengatur kecepatan (0 sampai 100 persen) |
| `DIR 1` / `DIR 0` | Arah maju / mundur |
| `LED 1` / `LED 0` | LED biru menyala / mati |
| `RPM` | Membaca RPM (butuh sensor yang berfungsi) |
| `X` | Menghentikan motor |

### Dokumentasi

![Serial Monitor Modul 1](Dokumentasi%20Robotika/Modul%201.png)

*Gambar 1. Serial Monitor saat perintah diuji.*

![Rangkaian iMCLab](Dokumentasi%20Robotika/Rangkaian%20iMCLab.jpeg)

*Gambar 2. Kit iMCLab yang dipakai.*

### Temuan
- Pada PWM 60 dan 70 motor diam. Motor baru berputar di sekitar **PWM 80 persen**. Rentang di bawah itu disebut **zona mati (dead zone)**, yaitu kondisi saat tegangan rata-rata belum cukup untuk melawan gesekan statis poros dan rugi tegangan di driver.
- Angka ini sejalan dengan kode referensi, yang mulai menaikkan kecepatan dari duty cycle 200 dari 255 (sekitar 78 persen).
- `DIR` bekerja dengan menukar logika IN1 dan IN2, sedangkan `X` menurunkan PWM ke 0 sehingga motor berhenti.

### Ringkasan tahap
Kecepatan dan arah motor berhasil dikendalikan lewat PWM dan logika IN1/IN2. Motor memiliki zona mati sehingga baru bergerak mulai sekitar PWM 80 persen.

---

## 4. Tahap 2: Otomatisasi dengan Python

### Yang ingin dicapai
Menjalankan uji step PWM dari Python, lalu menyimpan dan memvisualisasikan datanya.

### Konsep di baliknya
- **pySerial** memungkinkan Python mengirim dan menerima teks lewat port serial.
- **Uji step** berarti PWM dinaikkan bertahap sambil respons motor diamati terhadap waktu.
- Data disimpan dalam **CSV** dan digambarkan dengan **matplotlib**.

### Cara saya mengerjakan
1. Mengunggah `imclab_modul1.ino`, lalu menutup Serial Monitor supaya port tidak bentrok.
2. Memasang library: `pip install pyserial matplotlib`.
3. Menyesuaikan `PORT` pada skrip, kemudian menjalankan `python imclab_modul2.py`.
4. Skrip menjalankan urutan uji: PWM 60, 70, 80, 90, 100 (maju), berhenti, lalu 80 dan 100 (mundur).
5. Keluaran berupa `data_imclab.csv` dan `grafik_imclab.png`.

### Dokumentasi

![Grafik respons motor](Dokumentasi%20Robotika/Grafik%202.png)

*Gambar 3. Grafik respons motor terhadap step PWM.*

![Terminal Python](images/modul2-terminal.png)

*Gambar 4. Keluaran terminal saat skrip berjalan.*

Cuplikan data (`Modul 2/data_imclab.csv`):

```csv
waktu_s,pwm_bertanda_persen,rpm
6.038858890533447,0,0.0
8.548717975616455,0,0.0
15.076401710510254,60,0.0
17.57729697227478,60,0.0
24.111018896102905,70,0.0
26.620463132858276,70,0.0
33.14850759506299,80,0.0
35.6536386013031,80,0.0
```

### Analisis
- Pada PWM 60 dan 70 motor tidak menunjukkan gerakan, dan baru mulai berputar pada PWM 80. Hasil ini sama dengan pengamatan di tahap 1 dan memperkuat adanya zona mati.
- Kolom `rpm` di CSV bernilai 0 di seluruh baris karena sensor tidak mendeteksi putaran (baling-baling patah). Jadi data tidak bisa dipakai untuk analisis kuantitatif, dan pembacaan hanya dilakukan secara kualitatif terhadap perilaku motor pada tiap PWM dan arah.
- Keunggulan cara ini dibanding mengetik manual: pengujian bisa diulang dengan urutan yang sama, dan hasilnya tercatat sehingga dapat dianalisis ulang kapan saja.

### Ringkasan tahap
Python berhasil mengendalikan iMCLab sekaligus mencatat data percobaan secara otomatis, dan hasil uji step kembali menunjukkan zona mati pada PWM rendah.

---

## 5. Tahap 3: Kendali lewat Dashboard Blynk

### Yang ingin dicapai
Memantau dan mengendalikan motor dari dashboard Blynk melalui internet.

### Konsep di baliknya
- **Blynk IoT** adalah platform cloud yang menghubungkan perangkat dengan dashboard melalui *virtual pin*.
- Alur datanya: dashboard, cloud Blynk, WiFi, lalu ESP32.
- Tiap widget terikat ke satu *datastream* (virtual pin) yang dibaca atau ditulis oleh firmware.

### Cara saya mengerjakan
1. Memasang library **Blynk** (v1.3.5 ke atas, kompatibel dengan ESP32 core 3.x).
2. Membuat template **iMCLab** (ESP32, WiFi) dengan empat datastream:

| Pin | Kegunaan | Tipe |
|---|---|---|
| V0 | RPM | Double (0-5000) |
| V1 | PWM | Integer (0-100) |
| V2 | Arah | Integer (0-1) |
| V3 | LED | Integer (0-1) |

3. Menyusun Web Dashboard: Gauge dan Chart untuk V0, Slider untuk V1, serta Switch untuk V2 dan V3.
4. Membuat device dari template, menyalin `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, dan `BLYNK_AUTH_TOKEN` ke sketch, lalu mengisi `ssid` dan `pass` (WiFi 2.4 GHz).
5. Mengunggah `imclab_modul3_blynk.ino`, kemudian memastikan Serial Monitor menampilkan `Ready` dan status device **Online**.

### Dokumentasi

![Template dan datastream](images/modul3-datastream.png)

*Gambar 5. Template dan datastream di Blynk.*

![Dashboard Online](images/modul3-dashboard.png)

*Gambar 6. Dashboard berstatus Online (gauge, slider, switch).*

![Serial Monitor Blynk](images/modul3-serial.png)

*Gambar 7. Serial Monitor: WiFi tersambung dan Blynk siap.*

### Temuan
- Slider PWM, switch arah, dan switch LED di dashboard berhasil menggerakkan motor dan LED dari jarak jauh.
- Ada jeda antara aksi di dashboard dan respons motor. Ping ke server Blynk tercatat sekitar 527 ms. `[isi: jeda yang kamu amati secara langsung]`
- Kontrol internet lebih fleksibel karena bisa dilakukan dari mana saja, tetapi punya latensi dan bergantung pada kualitas jaringan. Ini berbeda dengan kontrol berkabel di tahap 1 dan 2 yang responsnya langsung.
- Slider di bawah sekitar 80 persen tidak menggerakkan motor, lagi-lagi karena zona mati.
- Gauge RPM tidak menampilkan nilai yang bermakna karena sensor tidak mendeteksi putaran. Sebagai pengganti, saya mengujinya dengan nilai tetap, dan gauge serta chart pada datastream V0 terbukti bekerja dengan benar.

### Ringkasan tahap
Dashboard Blynk berhasil memantau dan mengendalikan iMCLab melalui WiFi dan internet, dengan catatan latensi harus diperhitungkan.

---

## 6. Tahap 4: Pembaruan Firmware Tanpa Kabel (OTA)

### Yang ingin dicapai
Mengganti firmware ESP32 lewat jaringan, tanpa mencolokkan kabel USB, pada sistem yang tetap terhubung ke Blynk.

### Konsep di baliknya
- **OTA** mengirim firmware baru melalui jaringan sehingga akses fisik ke board tidak lagi diperlukan.
- Firmware diberi nomor versi lewat `BLYNK_FIRMWARE_VERSION` agar tiap pembaruan bisa dibedakan.
- Di percobaan ini saya memakai **ArduinoOTA** pada WiFi lokal. Jalur resmi Blynk.Air (OTA dari Blynk Console) membutuhkan kerangka Blynk.Edgent, jadi tidak saya gunakan.

### Cara saya mengerjakan
1. Menambah datastream **V5** (String) dan widget **Label** untuk menampilkan versi firmware.
2. Mengunggah `imclab_modul4_final.ino` lewat kabel dengan versi **1.0.0** dan `PWM_MAX = 100`. Label V5 menampilkan 1.0.0.
3. Mengubah dua baris pada sketch:
   ```cpp
   #define BLYNK_FIRMWARE_VERSION "1.0.1"
   const int PWM_MAX = 90;
   ```
4. Memastikan laptop dan ESP32 berada di WiFi yang sama, memilih port jaringan **imclab** di **Tools > Port**, lalu menekan Upload.
5. Setelah selesai, board restart sendiri dan tersambung kembali ke Blynk.

### Dokumentasi

![Sebelum update](images/modul4-sebelum.png)

*Gambar 8. Sebelum update: V5 = 1.0.0 dan slider bisa mencapai 100.*

![Log OTA](images/modul4-log.png)

*Gambar 9. Log upload via jaringan (`Sending invitation to 10.182.6.9` sampai `100% Done`).*

![Sesudah update](images/modul4-sesudah.png)

*Gambar 10. Sesudah update: V5 = 1.0.1 dan slider tertahan di 90.*

### Temuan
- Firmware terkirim lewat WiFi hingga 100 persen tanpa kabel USB. Di ujung proses muncul peringatan `Unexpected response from device`, tetapi board tetap menjalankan firmware baru.
- Keberhasilan update terlihat dari dua perubahan: label V5 berubah dari 1.0.0 menjadi 1.0.1, dan slider yang digeser ke 100 otomatis kembali ke 90.
- Keuntungan OTA: tidak perlu akses fisik, cocok untuk perangkat yang sulit dijangkau, dan bisa dilakukan berulang kali.
- Hal yang perlu dijaga: laptop dan board harus berada di jaringan yang sama, versi perlu dinaikkan agar perubahan mudah dilacak, dan motor dihentikan selama update (lewat `ArduinoOTA.onStart`).

### Ringkasan tahap
Firmware iMCLab berhasil diperbarui dari 1.0.0 ke 1.0.1 tanpa kabel, dan perubahannya terbukti dari dashboard Blynk.

---

## 7. Masalah yang Dihadapi dan Cara Mengatasinya

- **Port serial tidak muncul.** Penyebabnya driver USB-serial dan kabel. Saya memasang driver CP210x dan beralih ke kabel yang mendukung data.
- **Upload gagal setelah pesan `Changing baud rate to 921600`.** Kecepatan upload terlalu tinggi, jadi saya menurunkan Upload Speed ke 115200.
- **`Failed to connect to ESP32: No serial data received`.** Board tidak masuk mode upload. Solusinya menahan tombol BOOT saat proses upload dan mencabut adaptor DC.
- **Library "Blynk" tidak ditemukan.** Saya keliru memilih paket di Library Manager, lalu memasang library **Blynk** (v1.3.5 ke atas).
- **Proses berhenti di `Connecting to WiFi`.** Jaringan tidak sesuai. Saya beralih ke WiFi 2.4 GHz dan memeriksa ulang SSID serta password.
- **Motor diam pada PWM rendah.** Ini akibat zona mati, jadi PWM dipakai mulai 80 persen ke atas.
- **RPM selalu 0.** Baling-baling kit patah sehingga sensor tidak mendeteksi putaran. Logika pembacaan sensor tetap saya implementasikan, dan pengukuran RPM menunggu baling-baling diperbaiki.

## 8. Kesimpulan

Rangkaian percobaan ini membawa saya dari kontrol lokal lewat serial, ke otomatisasi dengan Python, kemudian ke kendali jarak jauh via Blynk, dan terakhir ke pembaruan firmware tanpa kabel. Pengendalian kecepatan dan arah motor berjalan baik di semua tahap. Satu-satunya yang belum tercapai adalah pengukuran kecepatan putar, dan itu murni karena keterbatasan perangkat keras (baling-baling patah), bukan kendala pada program.

## 9. Menjalankan Ulang Proyek

1. Clone repositori ini.
2. Buka sketch yang diinginkan di Arduino IDE (board **ESP32 Dev Module**, Upload Speed **115200**).
3. Untuk tahap 3 dan 4, isi `BLYNK_TEMPLATE_ID`, `BLYNK_AUTH_TOKEN`, `ssid`, dan `pass` dengan data milik sendiri.
4. Untuk tahap 2, jalankan `pip install pyserial matplotlib`, atur `PORT` di skrip, lalu `python imclab_modul2.py`.

## 10. Daftar Pustaka

- Blynk Documentation, OTA firmware updates (Blynk.Air): https://docs.blynk.io/en/blynk.edgent/updating-devices-firmwares-ota
- Blynk Library: https://github.com/Blynk-Technologies/blynk-library
- Arduino-ESP32 (Espressif): https://github.com/espressif/arduino-esp32
- Kode referensi kontrol motor dari dosen (IN1 = 27, IN2 = 26, Enable = 12)
