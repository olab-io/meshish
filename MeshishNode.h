#ifndef __MESHISH_H__
#define __MESHISH_H__

#define MESHISH_HTTP_PORT 80
#define MESHISH_PRIMARY 1
#define MESHISH_PRIMARY_IP "192.168.4.1"
#define MESHISH_MAX_AP_SCAN 100

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

class MeshishNode {

public:

  struct AccessPoint {
    String ssid;
    long rssi;
    byte encrypt;
    unsigned int nodeType;
  };

  enum nodeType{
    NODE_NONE,
    NODE_PRIMARY,
    NODE_SECONDARY
  };

  MeshishNode();
  ~MeshishNode();

  void setup(String password, bool primary=false);
  void loop();
  void makePrimary(bool primary=true);
  void debug(HardwareSerial* serial);
  bool isPrimary();
  bool isConnectedToPrimary();
  bool enableDefaultRoutes(bool b=true);
  unsigned int getStatus();

protected:

  void _setSSID();
  void _scanAndConnect();
  bool _generateSSID();
  unsigned int _getNodeType(const AccessPoint& ap);
  String _ipToString(const IPAddress& ip);

  byte _encrypt;
  bool _isPrimary;
  bool _debug;
  bool _connectingToAP; // true when STA is connecting to AP
  bool _creatingAP; // true when AP is being created
  unsigned int _numNetworks;
  uint32_t _chipId;
  String _ssid;
  String _ssidPrefix;
  String _password;
  
  ESP8266WebServer _server;
  AccessPoint _apList[MESHISH_MAX_AP_SCAN];
  HardwareSerial* _serial;
};

#endif /* __MESHISH_H__ */

