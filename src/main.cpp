#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Deneyap_SicaklikNemOlcer.h>
#include <Deneyap_Role.h>

// Wi-Fi ayarları
const char* ssid = "wifiadiniz";
const char* password = "wifisifeniz";

// Nesne tanımlamaları
TempHum TempHum;
Relay role;
WebServer server(80);
int role2 = 0;

float istenenSicaklik = 24.0; // Varsayılan istenen sıcaklık

// Forward declarations
void handleRoot();
void handleSetTemp();

void setup() {

  delay(3000);
  Serial.begin(9600);
  Wire.begin();
  
  // Wi-Fi bağlantısı
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi'ye bağlanılıyor...");
  }
  Serial.println("WiFi bağlantısı başarılı");

  // IP adresini yazdırma
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());

  // Röle başlatma
  if (!role.begin(0x0C)) {
    delay(3000);
    Serial.println("role I2C bağlantısı başarısız");
    while (1);
  }

  if (!TempHum.begin(0x70)) {
    delay(3000);
    Serial.println("sıcaklık nem I2C bağlantısı başarısız");
  }

  uint16_t READ_ID = TempHum.getReadID();
  Serial.print("READ_ID = ");
  Serial.println(READ_ID);

  // Web sunucusu ayarları
  server.on("/", handleRoot);
  server.on("/setTemp", handleSetTemp);
  server.begin();
  Serial.println("HTTP sunucusu başlatıldı");
}

void loop() {
  server.handleClient();

  float Tempvalue = TempHum.getTempValue();
  float Humvalue = TempHum.getHumValue();

  if (Tempvalue > istenenSicaklik) {
    role.RelayDrive(0);
    role2 = 0;
  } else {
    role.RelayDrive(1);
    role2 = 1;
  }

  delay(100);
}

void handleRoot() {
  float Tempvalue = TempHum.getTempValue();
  float Humvalue = TempHum.getHumValue();
  bool isRelayOn = role2 == 1; // Röle durumu kontrolü

  String html = "<html><body>";
  html += "<h1>Deneyap Kart Kombi Kontrol</h1>";
  html += "<p>Mevcut Sicaklik: " + String(Tempvalue) + "°C</p>";
  html += "<p>Mevcut Nem: %" + String(Humvalue) + "</p>";
  html += "<p>Kombi Durumu: " + String(isRelayOn ? "Calisiyor" : "Bosta") + "</p>";
  html += "<form action='/setTemp' method='get'>";
  html += "<label for='temp'>Istenen Sicaklik:</label>";
  html += "<input type='number' id='temp' name='temp' step='0.1'>";
  html += "<input type='submit' value='Ayarla'>";
  html += "</form>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleSetTemp() {
  if (server.hasArg("temp")) {
    istenenSicaklik = server.arg("temp").toFloat();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}