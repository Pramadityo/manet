#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

// --- TAMBAHAN PIN LED & BUZZER ---
#define LED_PIN 2       // Pin untuk LED
#define BUZZER_PIN 5    // Pin untuk Buzzer
#define BATAS_SUHU 30.0 // Batas suhu untuk alarm menyala

const char* ssid = "Redmi 10C";
const char* password = "RADITYA20";
const char* mqtt_server = "192.168.24.251"; 

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Menghubungkan ke: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) { // Timeout 10 detik
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal konek WiFi. Cek Hotspot!");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi MQTT...");
    if (client.connect("ESP32_Eric")) {
      Serial.println("Terhubung ke Broker!");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Waktu stabilisasi daya
  Serial.println("--- Sistem Dimulai ---");

  // --- TAMBAHAN SETUP PIN ---
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);    // Pastikan LED mati saat awal nyala
  digitalWrite(BUZZER_PIN, LOW); // Pastikan Buzzer mati saat awal nyala
  
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) { // Kirim tiap 5 detik
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      char payload[64];
      snprintf(payload, 64, "{\"temp\": %.2f, \"hum\": %.2f}", t, h);
      Serial.print("Kirim data: ");
      Serial.println(payload);
      client.publish("iot/sensor", payload);

      // --- TAMBAHAN LOGIKA ALARM ---
      if (t >= BATAS_SUHU) {
        digitalWrite(LED_PIN, HIGH);    // Nyalakan LED
        digitalWrite(BUZZER_PIN, HIGH); // Bunyikan Buzzer
        Serial.println("ALARM: Suhu Terlalu Panas!");
      } else {
        digitalWrite(LED_PIN, LOW);     // Matikan LED
        digitalWrite(BUZZER_PIN, LOW);  // Matikan Buzzer
      }

    } else {
      Serial.println("Sensor DHT error / tidak terbaca!");
    }
  }
}