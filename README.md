# Tugas Mata Kuliah Jaringan Sensor dan IoT — Kelompok 20

**Materi:** Node-RED dan ESP32 

---

## 👥 Anggota Kelompok 20 & Pembagian Ruangan

Setiap anggota bertanggung jawab merangkai dan memprogram 1 unit node ESP32 untuk masing-masing ruangan:

| No | Nama Anggota | NRP | Tanggung Jawab Node | Sensor yang Digunakan | Client ID MQTT |
| :-: | :--- | :---: | :---: | :---: | :--- |
| 1 | *Moh Adzril Aziz* | *2124600095* | **Ruang 1** (`ruang1`) | Modul Sensor **AHT10** (I2C) | `esp32-klp20-ruang1` |
| 2 | *Rassya Aditia* | *2124600097* | **Ruang 2** (`ruang2`) | **Potensiometer** (`POT_R2`) | `esp32-klp20-ruang2` |
| 3 | *Bima Syahputra* | *2124600115* | **Ruang 3** (`ruang3`) | **Potensiometer** (`POT_R3`) | `esp32-klp20-ruang3` |
| 4 | *Ferrisca Maylanazwa* | *2124600117* | **Ruang 4** (`ruang4`) | **Potensiometer** (`POT_R4`) | `esp32-klp20-ruang4` |

---

## 1. Deskripsi & Arsitektur Sistem

Sistem ini memantau kondisi suhu dan kelembapan serta mengendalikan 2 lampu (LED) di **4 ruangan (`ruang1` s/d `ruang4`)** secara *real-time* menggunakan protokol **MQTT** dan **Node-RED Dashboard**.

```text
                           ┌────────────────────┐
  ESP32 ruang1 (AHT10) ────┤                    │
  ESP32 ruang2 (Potensio) ─┤  Broker MQTT       ├──────── Node-RED ──── Browser
  ESP32 ruang3 (Potensio) ─┤  (Mosquitto Lokal  │         (client)      /ui
  ESP32 ruang4 (Potensio) ─┤   Port 1883)       │
                           └────────────────────┘
  publish  : temperature, humidity, status      subscribe: pens/klp20/+/temperature
  subscribe: lamp1, lamp2                                  pens/klp20/+/humidity
                                                           pens/klp20/+/status
                                                publish  : pens/klp20/ruangN/lamp1, lamp2
```

### Konfigurasi Broker MQTT (Mosquitto Lokal)
Sesuai ketentuan tugas, kelompok kami menggunakan **Mosquitto Broker Lokal** yang dijalankan pada laptop server (satu jaringan Wi-Fi/Hotspot dengan keempat ESP32):
- **Host / IP Broker (untuk ESP32):** `10.32.81.35` *(IPv4 WLAN laptop pada jaringan Hotspot)*
- **Host Broker (untuk Node-RED):** `localhost` / `10.32.81.35`
- **Port TCP:** `1883`
- **Konfigurasi `mosquitto.conf`:**
  ```conf
  listener 1883 0.0.0.0
  allow_anonymous true
  ```

---

## 2. Skematik & Konfigurasi Perangkat Keras (Pinout ESP32)

Berdasarkan penyesuaian perangkat keras yang digunakan oleh kelompok kami, **Ruang 1** menggunakan sensor suhu & kelembapan digital **AHT10 (I2C)**, sedangkan **Ruang 2, Ruang 3, dan Ruang 4** menggunakan **Potensiometer** sebagai input suhu dan kelembapan. Seluruh ruangan menggunakan pin **GPIO 2 (`D2`)** untuk Lampu 1 dan **GPIO 4 (`D4`)** untuk Lampu 2.

### 2.1 Tabel Pemetaan Pin Ruang 1 (ESP32 DevKit + Sensor AHT10)
| Komponen | Pin ESP32 | Label Skematik | Keterangan Rangkaian |
| :--- | :---: | :---: | :--- |
| **AHT10 – VIN** | `3V3` | `3V3` | Suplai tegangan 3.3V |
| **AHT10 – GND** | `GND` | `GND` | Ground |
| **AHT10 – SCL** | `GPIO 22` (`D22`) | `SCL_R1` | Jalur I2C Serial Clock |
| **AHT10 – SDA** | `GPIO 21` (`D21`) | `SDA_R1` | Jalur I2C Serial Data |
| **LED Lampu 1 + Resistor 220 Ω** | `GPIO 2` (`D2`) | `LAMP1_R1` | Pin D2 → Resistor R2 (220 Ω) → Anoda LED 1 → Katoda ke GND |
| **LED Lampu 2 + Resistor 220 Ω** | `GPIO 4` (`D4`) | `LAMP2_R1` | Pin D4 → Resistor R1 (220 Ω) → Anoda LED 2 → Katoda ke GND |

### 2.2 Tabel Pemetaan Pin Ruang 2, Ruang 3, dan Ruang 4 (ESP32 DevKit + Potensiometer)
| Komponen | Pin ESP32 | Label Skematik | Keterangan Rangkaian |
| :--- | :---: | :---: | :--- |
| **Potensiometer – VCC** | `3V3` | `3V3` | Suplai tegangan 3.3V |
| **Potensiometer – GND** | `GND` | `GND` | Ground |
| **Potensiometer – Wiper (Data)** | `GPIO 5` (`D5`) | `POT_R2` / `POT_R3` / `POT_R4` | Input pembacaan nilai suhu & kelembapan |
| **LED Lampu 1 + Resistor 220 Ω** | `GPIO 2` (`D2`) | `LAMP1_R2..R4` | Pin D2 → Resistor 220 Ω → Anoda LED 1 → Katoda ke GND |
| **LED Lampu 2 + Resistor 220 Ω** | `GPIO 4` (`D4`) | `LAMP2_R2..R4` | Pin D4 → Resistor 220 Ω → Anoda LED 2 → Katoda ke GND |

**Kebutuhan Library & Paket:**
- **Arduino IDE:** `EspMQTTClient` (Patrick Lapointe), `Adafruit AHTX0` (untuk sensor AHT10 pada Ruang 1).
- **Node-RED:** `node-red-dashboard`.

---

## 3. Rancangan Topic MQTT (`klp20`)

Seluruh topic diawali dengan prefix kelompok **`pens/klp20/<nama_ruang>/<fungsi>`** dengan detail spesifikasi sebagai berikut:

| Topic | Arah Komunikasi | Contoh Payload | QoS | Retain |
| :--- | :---: | :--- | :-: | :-: |
| `pens/klp20/ruangN/temperature` | ESP32 → Node-RED | `28.5` (°C, 1 angka desimal) | 0 | Tidak (`false`) |
| `pens/klp20/ruangN/humidity` | ESP32 → Node-RED | `65.0` (%, 1 angka desimal) | 0 | Tidak (`false`) |
| `pens/klp20/ruangN/lamp1` | Node-RED → ESP32 | `1` (Nyala) atau `0` (Mati) | 1 | Tidak (`false`) |
| `pens/klp20/ruangN/lamp2` | Node-RED → ESP32 | `1` (Nyala) atau `0` (Mati) | 1 | Tidak (`false`) |
| `pens/klp20/ruangN/status` | ESP32 / LWT | `online` atau `offline` | 0 | Ya (`true`) |

*(Catatan: `ruangN` mewakili `ruang1`, `ruang2`, `ruang3`, dan `ruang4`).*

### Penggunaan Wildcard pada Node-RED (`mqtt in`)
| Tujuan Subscribe | Topic Wildcard |
| :--- | :--- |
| Suhu seluruh ruangan | `pens/klp20/+/temperature` |
| Kelembapan seluruh ruangan | `pens/klp20/+/humidity` |
| Status koneksi seluruh ruangan | `pens/klp20/+/status` |
| Pemantauan seluruh trafik kelompok (CLI) | `pens/klp20/#` |

---

## 4. Pemenuhan Spesifikasi Sistem

### 4.1 Spesifikasi Node ESP32 (E1 – E7)
- **E1 (Auto-Reconnect & Non-Blocking):** Menggunakan `EspMQTTClient` (`client.loop()`) untuk menangani koneksi Wi-Fi dan MQTT secara otomatis tanpa penggunaan `delay()` panjang di dalam `loop()`.
- **E2 (Unique Client ID):** Setiap ESP32 memiliki ID unik `esp32-klp20-ruang1` hingga `esp32-klp20-ruang4`.
- **E3 (Last Will and Testament / LWT):** Mendaftarkan LWT pada `setup()` melalui `client.enableLastWillMessage(T_STATUS, "offline", true)` dengan *Keep Alive* 15 detik.
- **E4 (On Connection Established):** Saat tersambung ke broker, ESP32 melakukan *subscribe* ke topic `lamp1` dan `lamp2` masing-masing ruangan (QoS 1) dan mem-*publish* pesan `"online"` (*retain* = `true`) ke topic `status`.
- **E5 (Pembacaan Sensor Periodik 5 Detik):** Menggunakan pewaktu `millis()` setiap `5000 ms` untuk membaca suhu dan kelembapan (dari AHT10 pada Ruang 1 serta Potensiometer pada Ruang 2–4) dalam format 1 desimal (`String(val, 1)`).
- **E6 (Validasi Payload Aktuator):** Fungsi kendali lampu hanya mengeksekusi payload `"1"` (`HIGH` pada GPIO 2 / GPIO 4) dan `"0"` (`LOW`), serta mengabaikan payload lainnya.
- **E7 (Modularitas Kode):** Pembentukan seluruh topic dan Client ID otomatis mengikuti makro `#define KELOMPOK "klp20"` dan `#define ROOM "ruangN"`.

### 4.2 Spesifikasi Flow & Dashboard Node-RED (N1 – N7)
- **N1:** Seluruh node `mqtt in` dan `mqtt out` terhubung ke 1 konfigurasi `mqtt-broker` yang sama (`Mosquitto Lokal`).
- **N2:** Grafik suhu menggunakan 1 node `ui_chart` (tipe *Line*) yang menampilkan 4 garis sekaligus (`ruang1` s/d `ruang4`) melalui pemrosesan `msg.topic` di node `function`.
- **N3:** Menampilkan 4 buah `ui_gauge` kelembapan (rentang 0–100%) untuk masing-masing ruangan.
- **N4:** Menyediakan 8 saklar (`ui_switch`) untuk mengendalikan `lamp1` dan `lamp2` di keempat ruangan melalui `mqtt out` dengan **QoS 1**.
- **N5:** Menampilkan 4 indikator teks (`ui_text`) untuk memantau status `online`/`offline` setiap ruangan secara *real-time*.
- **N6:** Setiap jalur `mqtt in` dilengkapi node `debug` (`Debug Suhu`, `Debug Kelembapan`, `Debug Status`).
- **N7:** Tata letak dashboard terorganisir dalam 1 Tab (`Gedung`) yang memuat grup `Suhu` serta grup `Ruang 1`, `Ruang 2`, `Ruang 3`, dan `Ruang 4`.

---

## 5. Struktur Repositori

```text
├── README.md                       # Dokumentasi dan laporan proyek Kelompok 20
├── esp32/
│   ├── ruang1/                     # Source code ESP32 Ruang 1 (Program with AHT10.ino)
│   ├── ruang2/                     # Source code ESP32 Ruang 2 (Program with Potensio.ino)
│   ├── ruang3/                     # Source code ESP32 Ruang 3 (Program with Potensio.ino)
│   └── ruang4/                     # Source code ESP32 Ruang 4 (Program with Potensio.ino)
├── flows_klp20.json                # File export flow Node-RED Kelompok 20
└── dokumentasi/
    ├── skematik_rangkaian.jpg      # Gambar skematik rangkaian Ruang 1 - Ruang 4
    ├── Node-Red.png                # Screenshot flow Node-RED Kelompok 20
    └── dashboard_ui.png            # Screenshot halaman dashboard Node-RED (/ui)
```

---

## 6. Hasil Pengujian & Dokumentasi

### Daftar Pengujian Sistem
| No | Skenario Pengujian | Prosedur Uji | Hasil yang Diharapkan | Status |
| :-: | :--- | :--- | :--- | :-: |
| 1 | Koneksi Awal & Status Online | Menyalakan keempat ESP32 | Status `ruang1`–`ruang4` di dashboard berubah menjadi `online` | ✅ Berhasil |
| 2 | Pengiriman Data Sensor | Memantau grafik suhu & gauge kelembapan setiap 5 detik | Grafik menampilkan 4 garis ruangan & 4 gauge menampilkan nilai % kelembapan | ✅ Berhasil |
| 3 | Kontrol 8 Saklar Lampu | Mengubah saklar Lampu 1 (`D2`) & Lampu 2 (`D4`) tiap ruangan di dashboard | LED fisik pada ESP32 ruangan terkait menyala (`1`) dan mati (`0`) seketika | ✅ Berhasil |
| 4 | Uji Last Will (LWT) | Mencabut catu daya salah satu ESP32 secara tiba-tiba | Setelah ~15 detik, status ruangan tersebut otomatis berubah menjadi `offline` | ✅ Berhasil |
| 5 | Uji Retained Message | Me-*refresh* halaman browser dashboard Node-RED | Status terakhir (`online`/`offline`) langsung tampil berkat fitur *retain* | ✅ Berhasil |

### Tangkapan Layar (Screenshot) & Skematik
- **Link Video Demo:** *[Masukkan Link YouTube / Google Drive Video Demo di sini]*
- **Skematik Rangkaian 4 Ruangan:**
  ![Skematik Rangkaian](dokumentasi/skematik_rangkaian.jpg)
- **Flow Node-RED (`Node-Red.png`):**
  ![Flow Node-RED](dokumentasi/Node-Red.png)
- **Dashboard Node-RED (`/ui`):**
  ![Dashboard UI](dokumentasi/dashboard_ui.png)
