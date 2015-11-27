# Meshish

Meshish is an experimental Mesh-like library for use with the ESP8266 WiFi SoC. 
It is built on top of the existing Arduino library for ESP8266. At this time the
library __does not work__ as described. In short, this library was
created with the incorrect assumption that the ESP8266, acting in AP Mode, has
the ability to route traffic between two or more clients connected to it
([more here](http://www.esp8266.com/viewtopic.php?f=32&t=4686)). This is not the
case. This repository now lives as a historical document a proof-of-concept as
to one way that cheap WiFi nodes could be configured to communicate in a decentralized
mesh-like way. The rest of this README will outline the initial goals and desires of
the library design, as conceived before the author came to the disheartening realization
that the ESP8266 doesn't have routing/switching capabilities. 
