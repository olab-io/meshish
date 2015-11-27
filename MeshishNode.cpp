// The MIT License (MIT)
//
// Copyright (c) 2015 Brannon Dorsey <brannon@brannondorsey.com>
//               with support from the SAIC OpenLab
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "MeshishNode.h"

MeshishNode::MeshishNode():
  _isPrimary(false),
  _numNetworks(0),
  _ssidPrefix("M"),
  _ssid(""),
  _chipId(0),
  _serial(NULL),
  _debug(false),
  _connectingToAP(false),
  _creatingAP(false),
  _password(""),
  _server(ESP8266WebServer(80))
{

}

MeshishNode::~MeshishNode()
{

}

void MeshishNode::setup(String password, bool primary)
{

  WiFi.mode(WIFI_AP_STA);
  
  WiFi.disconnect();
  delay(100);

  _isPrimary = primary;
  _password = password;
  _chipId = ESP.getChipId();

  bool ssidGenerated = _generateSSID();

  if (!ssidGenerated && _debug)
  {
    _serial->println("MeshishNode::setup: _generateSSID() returned false.");
  }

  if (_isPrimary && ssidGenerated)
  {
    if (_debug)
    {
      _serial->println("MeshishNode::setup: Primary node creating SSID \"" + _ssid + "\"");
    }
    WiFi.softAP(_ssid.c_str());
    _creatingAP = true;
  }
  else
  {
    _scanAndConnect();
  }

  _server.on("/", [this](){
     _server.send(200, "text/plain", "welcome!");
  });
  
  _server.onNotFound([this](){
    _server.send(200, "text/plain", "404 file not found");
  });

  _server.begin();
}

void MeshishNode::loop()
{
 
  if (getStatus() == WL_CONNECTION_LOST)
  {
    _serial->println("MeshishNode::loop: connection lost");
  }
  else if(getStatus() == WL_DISCONNECTED)
  {
    _serial->println("MeshishNode::loop: disconnected");
  }
  
  if (_connectingToAP) 
  {
    unsigned int status = getStatus();
    if (_debug)
    {
      _serial->print("MeshishNode::loop: status ");
      _serial->println(status);  
    }
    
    // connection established
    if (status == WL_CONNECTED)
    {
      _connectingToAP = false;

      if (_debug)
      {
        _serial->print("MeshishNode::loop: connection to \"");
        _serial->print(WiFi.SSID());
        _serial->println("\" established.");
        _serial->print("MeshishNode::loop: recieved IP Address ");
        _serial->println(WiFi.localIP());
      }

      // now create a secondary AP
      bool ssidGenerated = _generateSSID();
      if (ssidGenerated)
      {
        // WiFi.softAPConfig(IPAddress(192, 168, 4, 10),
        //                   IPAddress(192, 168, 4, 10),
        //                   IPAddress(255, 255, 255, 0)); 
        WiFi.softAP(_ssid.c_str());
        _creatingAP = true;
      }

    } // connection failed
    else if (status == WL_CONNECT_FAILED)
    {
      _connectingToAP = false;

      if (_debug)
      {
        _serial->print("MeshishNode::loop: connection to ");
        _serial->print(WiFi.SSID());
        _serial->println(" failed.");
      }
    } // still connecting...
    else if (status == WL_IDLE_STATUS)
    {
      if (_debug)
      {
        _serial->println("MeshishNode::loop: connection idle...");
      }
    }
  }
  else
  {
    // not _connectingToAP
  }

  if (_creatingAP)
  {
    unsigned int status = getStatus();
    // connection established
    if (status == WL_CONNECTED)
    {
      _creatingAP = false;
      if (_debug)
      {
        _serial->println("MeshishNode::loop: Access point created.");
        WiFi.printDiag(*_serial);
      }
    }
    else if (status == WL_CONNECT_FAILED)
    {
      _creatingAP = false;
      if (_debug)
      {
      _serial->println("MeshishNode::loop: Failed to create access point.");
      }
    }
    else if (status == WL_IDLE_STATUS)
    {
      if (_debug)
      {
        _serial->println("MeshishNode::loop: access point creation idle...");
      }
    }
  }

  _server.handleClient();
}

void MeshishNode::debug(HardwareSerial* serial)
{
  _debug = true;
  _serial = serial;
}

void MeshishNode::makePrimary(bool primary)
{
  if (_isPrimary != primary) {

    _isPrimary = primary;
    WiFi.disconnect();

    if (_generateSSID() && _isPrimary)
    {
      WiFi.softAP(_ssid.c_str());
      _creatingAP = true;
    }
    else if (_debug)
    {
      _serial->println("MeshishNode::makePrimary: _generateSSID() returned false.");
    }

  }
}

bool MeshishNode::isPrimary()
{
  return _isPrimary;
}

// bool MeshishNode::isConnectedToPrimary()
// {

// }

// bool MeshishNode::enableDefaultRoutes(bool enable)
// {

// }

unsigned int MeshishNode::getStatus()
{
  return WiFi.status();
}


void MeshishNode::_scanAndConnect()
{
  if (_debug)
  {
    _serial->println("scan start...");
  }

  _numNetworks = min(WiFi.scanNetworks(), MESHISH_MAX_AP_SCAN);

  if (_debug)
  {
    _serial->print(_numNetworks);
    _serial->println(" networks found in scan");
  }

  // the index of the node with the highest rssi
  int maxDBmPrimary = -1;

  for (int i = 0; i < _numNetworks; i++)
  {
    _apList[i] = AccessPoint();
    _apList[i].ssid     = WiFi.SSID(i);
    _apList[i].rssi     = WiFi.RSSI(i);
    _apList[i].encrypt  = WiFi.encryptionType(i);
    _apList[i].nodeType = _getNodeType(_apList[i]);

    if (_apList[i].nodeType == NODE_PRIMARY)
    {
      if (maxDBmPrimary == -1)
      {
        maxDBmPrimary = i;
      }
      else
      {
        if (_apList[i].rssi > _apList[maxDBmPrimary].rssi)
        {
          maxDBmPrimary = i;
        }
      }
    }

    if (_debug)
    {
      _serial->print(_apList[i].ssid);
      _serial->print("\t");
      _serial->print(_apList[i].rssi);
      _serial->print("dBm\t");
      _serial->print("Encryption: ");
      _serial->print(_apList[i].encrypt);
      _serial->print("\tnodeType: ");
      _serial->println(_apList[i].nodeType);
    }
  }

  if (maxDBmPrimary != -1)
  {
    if (_debug)
    {
      _serial->print("MeshishNode::_scanAndConnect: connecting to ");
      _serial->println(_apList[maxDBmPrimary].ssid.c_str());
    } 

    if (_password.equals("")) WiFi.begin(_apList[maxDBmPrimary].ssid.c_str());
    else WiFi.begin(_apList[maxDBmPrimary].ssid.c_str(), _password.c_str());

    _connectingToAP = true;
  }
}

bool MeshishNode::_generateSSID()
{
  if (_isPrimary)
  { 
    _ssid = String(_ssidPrefix + "_1_" + String(MESHISH_PRIMARY_IP) + "_" + String(_chipId));
    if (_debug)
    {
      _serial->print("MeshishNode::_generateSSID: _isPrimary, ssid:");
      _serial->println(_ssid);
    }
    return true;
  }
  else
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      IPAddress ip = WiFi.localIP();
      _ssid = String(_ssidPrefix + "_0_" + _ipToString(ip) + "_" + String(_chipId));
      if (_debug)
      {
      _serial->print("MeshishNode::_generateSSID: !_isPrimary, ssid:");
      _serial->println(_ssid);
      }
      return true;
    }
    else
    {
      if (_debug)
      {
      _serial->println("MeshishNode::_generateSSID: !_isPrimary but status != WL_CONNECTED");
      }
    }

  }

  return false;
}


unsigned int MeshishNode::_getNodeType(const AccessPoint& ap)
{
  String prefix = ap.ssid.substring(0, _ssidPrefix.length());

  if (prefix.equals(_ssidPrefix))
  {
    if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_1_"))
    {
      return NODE_PRIMARY;
    }
    else if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_0_"))
    {
      return NODE_SECONDARY;
    }
  }

  return NODE_NONE;
}

String MeshishNode::_ipToString(const IPAddress& ip)
{
  return String(ip[0]) + "." + 
         String(ip[1]) + "." + 
         String(ip[2]) + "." + 
         String(ip[3]);
}


