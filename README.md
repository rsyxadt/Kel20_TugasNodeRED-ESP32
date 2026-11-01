# Kel20_TugasNodeRED-ESP32
Evaluasi Jaringan Sensor dan IoT - Kelompok 20 (Monitoring &amp; Kontrol 4 Ruangan dengan ESP32, MQTT, dan Node-RED)

## 👥 Anggota Kelompok 20

| Ruangan | Nama Anggota | NRP |
| :--- | :--- | :--- |
| **Ruang 1** | Muh Adzril Aziz | 2124600095 |
| **Ruang 2** | Rassya Aditia | 2124600097 |
| **Ruang 3** | Ferrisca Maylanazwa Santoso | 2124600117 |
| **Ruang 4** | Bima Syahputra | 2124600115 |

## Link Video Demonstrasi: 
    (https://drive.google.com/file/d/12jOS07XiDOtgNVn_NuHebHhn_qD1p9H3/view?usp=sharing)

## Link Video Presentasi:

## Arsitektur sistem:

## Perangkat keras:
    1. Daftar Komponen
  
   <img width="872" height="490" alt="image" src="https://github.com/user-attachments/assets/265941d7-6a11-4c48-ac21-27b6edfdf853" />

    2. Tabel Pin

  <img width="842" height="442" alt="image" src="https://github.com/user-attachments/assets/4e425037-ee30-471a-9bed-ad7c581e57b5" />
  <img width="852" height="391" alt="image" src="https://github.com/user-attachments/assets/25c80359-4374-4fdf-b249-546c549ab72f" />

    3. Gambar Rangkaian
![Rangkaian ESP32 dan Sensor](docs/images/rangkaian.jpg)

## Tabel topic final:

topic_temp_out   = "pens/klp20/ruangx(1-4)/temperature"

topic_hum_out    = "pens/klp20/ruang(1-4)/humidity"

topic_led1_in    = "pens/klp20/ruang(1-4)/lamp1"

topic_led2_in    = "pens/klp20/ruang(1-4)/lamp2"

topic_status = "pens/klp20/ruang(1-4)/status"

##  cara Menjalankan Kode ESP32

### Opsi A: Menggunakan PlatformIO (VS Code)
1. Buka folder `esp32/esp32_ruang/` di Visual Studio Code.
2. Salin `include/secrets.h.example` menjadi `include/secrets.h`, lalu isi SSID dan Password Wi-Fi Anda.
3. Pilih Environment board yang sesuai di status bar bawah PlatformIO:
   * **`env:esp32_ruang12`** untuk mengunggah ke **ESP32 #1** (Ruang 1 & 2).
   * **`env:esp32_ruang34`** untuk mengunggah ke **ESP32 #2** (Ruang 3 & 4).
4. Klik tombol **Build (✓)** lalu **Upload (→)**.

### Opsi B: Menggunakan Arduino IDE
1. Buka berkas yang sesuai di Arduino IDE:
   * **`esp32_ruang12.ino`** untuk board ESP32 #1.
   * **`esp32_ruang34.ino`** untuk board ESP32 #2.
2. Pasang library yang dibutuhkan via **Library Manager**:
   * `PubSubClient` oleh Nick O'Leary
   * `DHT sensor library` oleh Adafruit
   * `Adafruit Unified Sensor` oleh Adafruit
3. Pilih board **DOIT ESP32 DEVKIT V1** dan upload.
   
1. Konfigurasi dan Upload Program ESP32

* **Library Arduino yang Dibutuhkan:**
  * `PubSubClient` (untuk menangani protokol MQTT).
  * `DHT sensor library` (untuk membaca sensor suhu dan kelembaban).

* **Langkah-langkah Upload:**
  1. Buka file kode di direktori `esp32/esp32_ruang/esp32_ruang.ino`[cite: 1] menggunakan Arduino IDE.
  2. Cari dan sesuaikan bagian kode berikut sebelum melakukan *upload*:
     * **Wi-Fi:** Ganti variabel SSID dan Password dengan jaringan Wi-Fi yang digunakan:
       ```cpp
       const char* ssid = "SSID_WIFI_KAMU";
       const char* password = "PASSWORD_WIFI_KAMU";
       ```
     * **Broker MQTT (HiveMQ):** Ganti alamat server broker dengan HiveMQ:
       ```cpp
       const char* mqtt_server = "broker.hivemq.com"; // atau IP broker HiveMQ lokal/publik
       ```
     * **ROOM (Ruangan):** Sesuaikan string identifikasi ruangan pada payload atau topik MQTT:
       ```cpp
       String ROOM_ID = "ruang1"; // Ubah sesuai ruangan yang dipegang
       ```
  3. Hubungkan ESP32 ke komputer, pilih board **ESP32 Dev Module** di Arduino IDE, lalu klik **Upload**.

---

###  Konfigurasi Node-RED

* **Paket Palette yang Harus Dipasang:**
  Masuk ke menu *Manage Palette* di Node-RED, lalu pasang *node* tambahan berikut:
  * `node-red-dashboard`

* **Cara Import `flows.json`:**
  1. Buka Node-RED di browser.
  2. Klik ikon **Menu (garis tiga)** di pojok kanan atas, lalu pilih **Import**.
  3. Pilih atau *drag & drop* file `flows.json` yang ada di dalam folder `node-red/` repository ini[cite: 1].
  4. Klik tombol **Import**, lalu klik tombol merah **Deploy** di pojok kanan atas.

---

###  Alamat Dashboard
Setelah *flow* berhasil di-*deploy*, antarmuka visualisasi data ruangan dapat diakses melalui browser pada alamat berikut:

```text
[http://10.32.81.35:1880/ui](http://10.32.81.35:1880/ui)

## Screenshot Flow Node-RED and Dashboard:

   <img width="532" height="558" alt="image" src="https://github.com/user-attachments/assets/18b97a59-ce71-4dc9-b598-cdb2169efd93" />

## Hasil pengujian T1-T12:
  
    <img width="497" height="516" alt="image" src="https://github.com/user-attachments/assets/f23a4334-5af9-48a7-9a7d-899c6f2c6a81" />

## Ruang yang disimulasikan:

    ruang 1-4
    
## kendala dan solusi:

## Pembagian tugas anggota
