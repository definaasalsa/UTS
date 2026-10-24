"""
Modul 2 - Pemrograman iMCLab dengan Python
Mengirim perintah ke iMCLab lewat serial (sketch Modul 1 harus sudah ter-upload),
menjalankan uji step PWM, mencatat data ke CSV, lalu membuat grafik.

Install : pip install pyserial matplotlib
Jalankan: python imclab_modul2.py
TUTUP Serial Monitor Arduino dulu, karena satu port tidak bisa dipakai dua program.
"""
import csv
import time

import matplotlib.pyplot as plt
import serial

PORT = "COM5"
BAUD = 115200

# profil uji: (PWM persen, arah, lama detik)
PROFIL = [(0, 1, 3), (60, 1, 5), (70, 1, 5), (80, 1, 8), (90, 1, 8), (100, 1, 8),
          (0, 1, 3), (80, 0, 8), (100, 0, 8), (0, 0, 3)]
# catatan: 60 dan 70 sengaja disertakan untuk menunjukkan zona mati motor (RPM = 0)
INTERVAL = 0.5   # detik


def buka_port():
    ser = serial.Serial()
    ser.port = PORT
    ser.baudrate = BAUD
    ser.timeout = 1
    ser.dtr = False      # cegah ESP32 ter-reset / tertahan saat port dibuka
    ser.rts = False
    ser.open()
    time.sleep(1)
    return ser


def kirim(ser, perintah):
    ser.reset_input_buffer()          # buang sisa data lama
    ser.write((perintah + "\n").encode())
    return ser.readline().decode(errors="ignore").strip()


def main():
    ser = buka_port()
    ver = ""
    for _ in range(5):
        ver = kirim(ser, "VER")
        if ver.startswith("iMCLab"):
            break
    else:
        print("Board TIDAK menjawab. Cek: (1) Serial Monitor Arduino sudah ditutup,")
        print("(2) board berisi imclab_modul1.ino, (3) PORT di script benar.")
        ser.close()
        return
    print("Firmware:", ver)

    waktu, pwm_list, rpm_list = [], [], []
    start = time.time()

    try:
        for pwm, arah, lama in PROFIL:
            kirim(ser, f"DIR {arah}")
            kirim(ser, f"PWM {pwm}")
            nama = "maju" if arah else "mundur"
            print(f"--- PWM {pwm}% ({nama}) selama {lama} detik ---")
            t_akhir = time.time() + lama
            while time.time() < t_akhir:
                jawab = kirim(ser, "RPM")
                try:
                    rpm = float(jawab)
                except ValueError:
                    print(f"   (jawaban tidak valid dari board: '{jawab}')")
                    rpm = 0.0
                t = time.time() - start
                waktu.append(t)
                pwm_list.append(pwm if arah else -pwm)   # negatif = mundur
                rpm_list.append(rpm)
                print(f"t={t:6.1f}s  PWM={pwm:3d}% {nama:6s}  RPM={rpm:7.1f}")
                time.sleep(INTERVAL)
    except KeyboardInterrupt:
        print("Dihentikan manual")
    finally:
        kirim(ser, "X")
        ser.close()

    # CSV: bisa dipakai lagi sebagai dataset UAS
    with open("data_imclab.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["waktu_s", "pwm_bertanda_persen", "rpm"])
        w.writerows(zip(waktu, pwm_list, rpm_list))

    # grafik
    fig, ax1 = plt.subplots(figsize=(9, 5))
    ax1.step(waktu, pwm_list, color="tab:red", where="post", label="PWM (%)")
    ax1.set_xlabel("Waktu (s)")
    ax1.set_ylabel("PWM (%), negatif = mundur", color="tab:red")
    if any(r != 0 for r in rpm_list):
        ax2 = ax1.twinx()
        ax2.plot(waktu, rpm_list, color="tab:blue", label="RPM")
        ax2.set_ylabel("RPM", color="tab:blue")
    plt.title("Respons motor iMCLab terhadap step PWM")
    ax1.grid(True)
    fig.tight_layout()
    plt.savefig("grafik_imclab.png", dpi=150)
    plt.show()


if __name__ == "__main__":
    main()