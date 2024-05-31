#include "WifiCam.hpp"
#include <WiFi.h>

#include <HTTPClient.h>

//static const char* WIFI_SSID = "VIETTEL_2.4G";
//static const char* WIFI_PASS = "mainambangtung";

static const char* WIFI_SSID = "Hope Diyoza";
static const char* WIFI_PASS = "khuvuon69";

//static const char* WIFI_SSID = "Giangvien";
//static const char* WIFI_PASS = "dhbk@2024";

//static const char* WIFI_SSID = "mine.";
//static const char* WIFI_PASS = "01020304";

String localIP;
const int buttonPin = 2;
#define LED_BUILTIN 4

esp32cam::Resolution initialResolution;

WebServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode (LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.println();
  delay(2000);

  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.printf("WiFi failure %d\n", WiFi.status());
    Serial.println("WIFIFAILED");
    delay(2000);
    ESP.restart();
  }
  Serial.println("--------------");
  Serial.println("WiFi connected");
  
  {
    using namespace esp32cam;

    initialResolution = Resolution::find(1024, 768);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("Camera Initialize Failure");
      Serial.println("CAMFAILURE");
      delay(2000);
      ESP.restart();
    }
    Serial.println("Camera Initialize Success");
  }

  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("--------------");
  
  localIP = "http://" + WiFi.localIP().toString() + "/takepicture";

  addRequestHandlers();
  server.begin();
  Serial.println("READYTOUSE");
}

void loop() {
  server.handleClient();
  
  if (digitalRead(buttonPin) == 0) {
    Serial.println("PROCESSIMAGE");
    delay(100);
    takePictureCode();
  }

}
