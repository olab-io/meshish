#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "MeshishNode.h"


MeshishNode node;

void setup() {

  Serial.begin(9600);
  node.debug(&Serial);
  node.setup("", false);
}

void loop() {
  // put your main code here, to run repeatedly:
  node.loop();
}
