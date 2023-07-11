#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

static const int RX= 16, TX= 17;
static const uint32_t GPSBaud = 9600;

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

const char* serverName = "https://api.thingspeak.com/update.json";
const char* writeAPIKey = "your_API_KEY";
const int channelID = your_CHANNEL_ID;

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

void setup() {
  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   IOT BASED       ");
  lcd.setCursor(0, 1);
  lcd.print("VEHICLE TRACKING   ");
  delay(2000);
  lcd.clear(); 
  
  Serial.begin(115200);
  GPSSerial.begin(GPSBaud, SERIAL_8N1, RX, TX);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("WiFi connecting...          ");
  }

  lcd.setCursor(0, 0);
  lcd.print("WiFi connected          ");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

}

void loop() {
  while (GPSSerial.available() > 0) {
    if (gps.encode(GPSSerial.read())) {
      displaydata();
      displaywebpage();
    }
  }
}

void displaydata() {
  if (gps.location.isValid()) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    
    HTTPClient http;
    String serverPath = serverName;
    serverPath += "?api_key=";
    serverPath += writeAPIKey;
    serverPath += "&field1=";
    serverPath += String(latitude, 6);
    serverPath += "&field2=";
    serverPath += String(longitude, 6);
    
    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();
    http.end();
    delay(20000);
  }
  else {
    Serial.println("GPS Error");
  }
}

void displaywebpage() {
  WiFiClient client;
  if (!client.connect("api.thingspeak.com", 80)) {
    Serial.println("Connection Failed");
    return;
  }
  
  String page = "<html><center><p><h1>Real Time Vehicle Tracking using IoT</h1><a style='color:RED;font-size:125%;' href='http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
  page += gps.location.lat();
  page += "+";
  page += gps.location.lng();
  page += "'>Click here For Live Location</a> </p></center></html>";

  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: text/html\r\n");
  client.print("Connection: close\r\n");
  client.print("\r\n");
  client.print(page);

  delay(100);
}
