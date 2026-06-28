#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G display(128, 64, &Wire, -1);

String sendAT(String cmd, uint32_t timeout = 3000) {
  String response = "";

  while (Serial1.available())
    Serial1.read();

  Serial1.println(cmd);

  uint32_t start = millis();

  while (millis() - start < timeout) {
    while (Serial1.available()) {
      char c = Serial1.read();
      response += c;
    }
  }

  return response;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Vehicle Tracker");
  display.println("Initializing...");
  display.display();

  delay(5000);

  // Basic communication
  String atResp = sendAT("AT", 2000);

  // Network registration
  String cregResp = sendAT("AT+CREG?", 2000);

  // GPS status
  String gpsResp = sendAT("AT+CGPS?", 2000);

  // Open internet
  String netResp = sendAT("AT+NETOPEN", 5000);

  // Get IP
  String ipResp = sendAT("AT+IPADDR", 3000);

  sendAT("AT+CGPS=0", 3000);
  delay(3000);

  sendAT("AT+CGPS=1", 3000);

  String gpsState = sendAT("AT+CGPS?");
  Serial.println(gpsState);

  delay(10000);

  // GPS coordinates
  // String gpsInfo = sendAT("AT+CGPSINFO", 3000);
  String gpsInfo;

  for (int i = 0; i < 10; i++) {
    gpsInfo = sendAT("AT+CGPSINFO", 3000);

    if (gpsInfo.indexOf(",,,,") == -1)
      break;

    Serial.println("Waiting for GPS Fix...");
    delay(5000);
  }

  // Debug output
  Serial.println("===== AT =====");
  Serial.println(atResp);

  Serial.println("===== CREG =====");
  Serial.println(cregResp);

  Serial.println("===== NETOPEN =====");
  Serial.println(netResp);

  Serial.println("===== IPADDR =====");
  Serial.println(ipResp);

  Serial.println("===== GPS =====");
  Serial.println(gpsInfo);

  // Status evaluation
  String simStatus = "FAIL";
  String netStatus = "FAIL";
  String ipStatus = "FAIL";
  String gpsStatus = "NOFIX";

  if (atResp.indexOf("OK") >= 0)
    simStatus = "OK";

  if (cregResp.indexOf(",1") >= 0 || cregResp.indexOf(",5") >= 0)
    netStatus = "OK";

  if (ipResp.indexOf("+IPADDR:") >= 0)
    ipStatus = "OK";

  if (gpsInfo.indexOf(",,,,") == -1 && gpsInfo.indexOf("+CGPSINFO:") >= 0)
    gpsStatus = "FIX";

  // OLED display
  display.clearDisplay();

  display.setCursor(0, 0);
  display.println("Vehicle Tracker");

  display.setCursor(0, 16);
  display.print("SIM : ");
  display.println(simStatus);

  display.setCursor(0, 28);
  display.print("NET : ");
  display.println(netStatus);

  display.setCursor(0, 40);
  display.print("GPS : ");
  display.println(gpsStatus);

  display.setCursor(0, 52);
  display.print("IP  : ");
  display.println(ipStatus);

  display.display();
}

void loop() {
}