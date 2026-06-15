#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- SETUP SENSOR DHT11 ---
#define DHTPIN 4
#define DHTTYPE DHT11
#define BATAS_SUHU 30.0 

// --- SETUP LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- KONFIGURASI JARINGAN & MQTT ---
const char* ssid = "RELAX";
const char* password = "mbappe1234";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
unsigned long lastAnim = 0;  // Variabel timer untuk animasi heartbeat
bool heartState = false;     // Status kedip heartbeat

// --- CUSTOM CHARACTERS FOR ANIMATION ---
byte customBlock[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}; // Kotak Loading
byte happyFace[8]  = {0x00, 0x0A, 0x00, 0x04, 0x11, 0x0E, 0x00, 0x00}; // Emoji Smile 😊
byte sadFace[8]    = {0x00, 0x0A, 0x00, 0x04, 0x0E, 0x11, 0x00, 0x00}; // Emoji Sad/Hot ☹
byte heartIcon[8]  = {0x0A, 0x1F, 0x1F, 0x1E, 0x0C, 0x04, 0x00, 0x00}; // Icon Heart

void setup_wifi() {
  delay(100);
  Serial.println("\nMenghubungkan ke WiFi...");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Konek WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true); 
  delay(1000); 

  WiFi.begin(ssid, password);

  int timeout = 0;
  // Animasi Loading Bar di baris ke-2 LCD (Maksimal 16 Kotak)
  while (WiFi.status() != WL_CONNECTED && timeout < 16) { 
    delay(500);
    Serial.print(".");
    lcd.setCursor(timeout, 1);
    lcd.write(0); // Cetak custom kotak loading
    timeout++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Terhubung!");
    lcd.setCursor(0, 0);
    lcd.print("WiFi Sukses!    ");
  } else {
    Serial.println("\nGagal WiFi!");
    lcd.setCursor(0, 0);
    lcd.print("WiFi Gagal!     ");
  }
  delay(2000);
  lcd.clear();
}

void reconnect() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi terputus! Batal sambung MQTT.");
      break; 
    }

    Serial.print("Mencoba koneksi MQTT...");
    lcd.setCursor(0, 0);
    lcd.print("Konek Server... ");
    
    if (client.connect("ESP32Maggot123")) { 
      Serial.println("Terhubung ke Broker!");
      lcd.setCursor(0, 0);
      lcd.print("Server Sukses!  ");
      delay(1500);
      lcd.clear();
    } else {
      Serial.print("Gagal, rc=");
      Serial.println(client.state());
      lcd.setCursor(0, 1);
      lcd.print("Gagal: rc=");
      lcd.print(client.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  
  // --- MULAI LCD & DAFTARKAN KARAKTER ANIMASI ---
  lcd.init();       
  lcd.backlight();  
  lcd.createChar(0, customBlock);
  lcd.createChar(1, happyFace);
  lcd.createChar(2, sadFace);
  lcd.createChar(3, heartIcon);

  lcd.setCursor(0, 0);
  lcd.print("Sistem Maggot");
  lcd.setCursor(0, 1);
  lcd.print("Memulai...     ");
  delay(2000);

  dht.begin();
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Cek Koneksi WiFi
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
    if (WiFi.status() != WL_CONNECTED) {
      return; 
    }
  }
  
  // Cek Koneksi MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- ANIMASI 1: HEARTBEAT (Blinking tiap 500ms tanpa delay) ---
  unsigned long currentMillis = millis();
  if (currentMillis - lastAnim >= 500) {
    lastAnim = currentMillis;
    heartState = !heartState;
    
    lcd.setCursor(15, 1); // Pojok kanan bawah
    if (heartState) {
      lcd.write(3); // Cetak Icon Heart
    } else {
      lcd.print(" "); // Hapus icon (efek kedip)
    }
  }

  // --- LOGIKA UTAMA SENSOR & DATA (Tiap 5 Detik) ---
  if (currentMillis - lastMsg > 5000) { 
    lastMsg = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      char payload[64];
      snprintf(payload, 64, "{\"temp\": %.2f, \"hum\": %.2f}", t, h);
      
      Serial.print("Kirim data ke MQTT: ");
      Serial.println(payload);
      client.publish("iot/sensor", payload);

      // --- TAMPILKAN DATA KE LCD (Maks 15 Karakter agar Kolom 16 Bebas) ---
      char lcdBuf0[16];
      char lcdBuf1[16];
      snprintf(lcdBuf0, 16, "Suhu: %.1f C   ", t);
      snprintf(lcdBuf1, 16, "Lembap: %.1f %% ", h);

      lcd.setCursor(0, 0);
      lcd.print(lcdBuf0);
      lcd.setCursor(0, 1);
      lcd.print(lcdBuf1);

      // --- ANIMASI 2: EMOJI STATUS BERDASARKAN SUHU ---
      lcd.setCursor(15, 0); // Pojok kanan atas
      if (t >= BATAS_SUHU) {
        lcd.write(2); // Cetak Emoji Cemberut/Bahaya ☹
        Serial.println("WARNING: Suhu Terlalu Panas untuk Maggot!");
      } else {
        lcd.write(1); // Cetak Emoji Senyum/Aman 😊
      }

    } else {
      Serial.println("Sensor DHT error!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor DHT Error");
    }
  }
}