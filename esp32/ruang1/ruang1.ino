#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h> 
#include "time.h"           // Library bawaan ESP32 untuk waktu NTP

// ================= Konfigurasi WiFi & MQTT =================
const char* ssid        = "SSID_LAB";
const char* password    = "PASSWORD_LAB";
const char* mqtt_server = "broker.hivemq.com"; 
const int   mqtt_port   = 1883;

// ================= Konfigurasi Waktu NTP (WIB / GMT+7) =================
const char* ntpServer          = "pool.ntp.org";
const long  gmtOffset_sec      = 7 * 3600; // WIB = UTC + 7 jam
const int   daylightOffset_sec = 0;

// ================= Topic MQTT =================
const char* topic_temp_out   = "pens/klp20/ruang1/temperature";
const char* topic_hum_out    = "pens/klp20/ruang1/humidity";
const char* topic_led1_in    = "pens/klp20/ruang1/lamp1";
const char* topic_led2_in    = "pens/klp20/ruang1/lamp2";
const char* topic_device_out = "pens/klp20/ruang1/device_status"; // Hanya menggunakan device_status

// ================= Pin Hardware =================
const int LED1_PIN = 2;  // LED indikator 1
const int LED2_PIN = 4;  // LED indikator 2
// Pin I2C AHT10 di ESP32:
// SDA -> GPIO 21
// SCL -> GPIO 22
// VIN -> 3.3V
// GND -> GND

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_AHTX0 aht; // Objek sensor AHT10

// ================= Variabel Timer Non-Blocking (Pengganti Delay) =================
unsigned long lastPublish             = 0;
const unsigned long publishInterval   = 5000; // Kirim data setiap 5000 ms (5 detik)
unsigned long lastReconnectTry        = 0;
const unsigned long reconnectInterval = 5000; // Interval coba konek ulang MQTT (5 detik)
unsigned long lastWiFiDot             = 0;

// Callback saat ada pesan masuk pada topik subscribe (lamp1 & lamp2)
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Kontrol Lampu 1
  if (String(topic) == topic_led1_in) {
    if (message == "ON" || message == "1") {
      digitalWrite(LED1_PIN, HIGH);
    } else if (message == "OFF" || message == "0") {
      digitalWrite(LED1_PIN, LOW);
    }
  }

  // Kontrol Lampu 2
  if (String(topic) == topic_led2_in) {
    if (message == "ON" || message == "1") {
      digitalWrite(LED2_PIN, HIGH);
    } else if (message == "OFF" || message == "0") {
      digitalWrite(LED2_PIN, LOW);
    }
  }
}

// Fungsi Reconnect Non-Blocking dengan Last Will untuk device_status
bool reconnect() {
  String clientId = "ESP32_Klp20_" + String(random(0xffff), HEX);
  
  // Set Last Will: Jika ESP32 putus/mati, broker otomatis mengirim "OFFLINE" ke topic_device_out
  if (client.connect(clientId.c_str(), topic_device_out, 1, true, "OFFLINE")) {
    Serial.println("MQTT Terhubung!");
    
    // Kirim status bahwa perangkat sedang ONLINE (Retained message)
    client.publish(topic_device_out, "ONLINE", true);

    // Subscribe kembali ke kontrol lampu
    client.subscribe(topic_led1_in);
    client.subscribe(topic_led2_in);
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Inisialisasi I2C dan Sensor AHT10
  Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22
  if (!aht.begin()) {
    Serial.println("Gagal menemukan sensor AHT10! Cek kabel I2C.");
    while (1) {
      // Berkedip cepat tanpa delay() jika sensor AHT10 tidak terdeteksi
      if (millis() - lastWiFiDot >= 200) {
        lastWiFiDot = millis();
        digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
      }
    }
  }
  Serial.println("Sensor AHT10 berhasil ditemukan!");

  // Koneksi WiFi menggunakan non-blocking timer
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWiFiDot >= 500) {
      lastWiFiDot = millis();
      Serial.print(".");
    }
  }
  Serial.println("\nWiFi Terhubung");

  // Sinkronisasi Waktu dengan Server NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Cek Koneksi MQTT Secara Non-Blocking
  if (!client.connected()) {
    if (currentMillis - lastReconnectTry >= reconnectInterval) {
      lastReconnectTry = currentMillis;
      reconnect();
    }
  } else {
    client.loop();
  }

  // 2. Pembacaan AHT10 & Pengiriman Data Setiap 5 Detik (5000 ms) Tanpa Delay
  if (currentMillis - lastPublish >= publishInterval) {
    lastPublish = currentMillis;

    // Variabel event untuk membaca suhu dan kelembapan dari AHT10
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);

    float currentTemp = temp.temperature;
    float currentHum  = humidity.relative_humidity;

    // Format data ke 2 angka di belakang koma
    char tempPayload[8];
    char humPayload[8];
    dtostrf(currentTemp, 4, 2, tempPayload);
    dtostrf(currentHum, 4, 2, humPayload);

    // Kirim data suhu, kelembapan, dan device_status ke broker MQTT jika terhubung
    if (client.connected()) {
      client.publish(topic_temp_out, tempPayload);
      client.publish(topic_hum_out, humPayload);
      client.publish(topic_device_out, "ONLINE", true); // Kirim status perangkat ONLINE
    }

    // Ambil waktu real-time dari NTP untuk ditampilkan di Serial Monitor
    struct tm timeinfo;
    char timeString[12] = "00:00:00";
    if (getLocalTime(&timeinfo, 10)) {
      strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
    }

    // Debug serial monitor
    Serial.print("[");
    Serial.print(timeString);
    Serial.print("] AHT10 -> Temp: ");
    Serial.print(tempPayload);
    Serial.print(" °C | Hum: ");
    Serial.print(humPayload);
    Serial.println(" % | Device: ONLINE");
  }
}
