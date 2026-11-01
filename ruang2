#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h" // Library bawaan ESP32 untuk waktu NTP

// ================= Konfigurasi WiFi & MQTT =================
const char* ssid        = "WARJO-EXTND";
const char* password    = "miloanget";
const char* mqtt_server = "broker.hivemq.com"; // Sesuaikan dengan alamat broker yang dipakai
const int   mqtt_port   = 1883;

// ================= Konfigurasi Waktu NTP (WIB / GMT+7) =================
const char* ntpServer          = "pool.ntp.org";
const long  gmtOffset_sec      = 7 * 3600; // WIB = UTC + 7 jam
const int   daylightOffset_sec = 0;

// ================= Topic MQTT =================
const char* topic_temp_out   = "pens/klp20/ruang2/temperature";
const char* topic_hum_out    = "pens/klp20/ruang2/humidity";
const char* topic_led1_in    = "pens/klp20/ruang2/lamp1";
const char* topic_led2_in    = "pens/klp20/ruang2/lamp2";
const char* topic_device_out = "pens/klp20/ruang2/status"; // Hanya menggunakan device_status

// ================= Pin Hardware =================
const int POT_PIN  = 5;  // Pin ADC ESP32 (Potensio / NTC)
const int LED1_PIN = 2;  // LED indikator 1
const int LED2_PIN = 4;  // LED indikator 2

WiFiClient espClient;
PubSubClient client(espClient);

// ================= Variabel Timer Non-Blocking (Pengganti Delay) =================
unsigned long lastPublish             = 0;
unsigned long publishInterval         = 5000; // Kirim data setiap 5000 ms (5 detik)
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
  analogReadResolution(12); // Resolusi ADC ESP32: 0 - 4095

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

  // 2. Pengiriman Data Berkala Setiap 5 Detik (5000 ms) Tanpa Delay
  if (currentMillis - lastPublish >= publishInterval) {
    lastPublish = currentMillis;

    // Baca nilai ADC dari Potensiometer / NTC (0 - 4095)
    int potValue = analogRead(POT_PIN);

    // Petakan nilai potensio ke rentang suhu dasar (20.0°C - 45.0°C)
    float baseTemp = 20.0 + ((float)potValue / 4095.0) * (45.0 - 20.0);

    // Tambahkan variasi acak kecil pada suhu (-0.4°C sampai +0.4°C)
    float tempNoise = (random(-40, 41)) / 100.0;
    float finalTemp = baseTemp + tempNoise;

    // Hitung kelembapan berkorelasi terbalik (suhu tinggi -> kelembapan rendah)
    float baseHum = 85.0 - ((finalTemp - 20.0) / (45.0 - 20.0)) * (85.0 - 30.0);

    // Tambahkan fluktuasi acak pada kelembapan (-1.5% sampai +1.5%)
    float humNoise = (random(-150, 151)) / 100.0;
    float finalHum = constrain(baseHum + humNoise, 10.0, 99.0);

    // Format data ke 2 desimal
    char tempPayload[8];
    char humPayload[8];
    dtostrf(finalTemp, 4, 2, tempPayload);
    dtostrf(finalHum, 4, 2, humPayload);

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
    Serial.print("] Pot ADC: ");
    Serial.print(potValue);
    Serial.print(" | Temp: ");
    Serial.print(tempPayload);
    Serial.print(" °C | Hum: ");
    Serial.print(humPayload);
    Serial.println(" % | Device: ONLINE");
  }
}
