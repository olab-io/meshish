# Meshish

Meshish is an experimental Mesh-like library for use with the ESP8266 WiFiSoC. It is built on top of the existing Arduino library for ESP8266. At this time the library __does not work__ as described. In short, this library was created with the incorrect assumption that the ESP8266, acting in AP Mode, has
the ability to route traffic between two or more clients connected to it ([more here](http://www.esp8266.com/viewtopic.php?f=32&t=4686)). This is not the case. This repository now lives as a historical document and proof-of- concept as to one way that cheap WiFi nodes could be configured to communicate in a decentralized mesh-like way. The rest of this document will outline the initial goals and desires of the library design, as conceived before the author came to the disheartening realization that the ESP8266 doesn't have routing/switching capabilities.

Meshish and [related ESP8266 research](https://github.com/olab-io/ESP8266-Research)  was  conducted in summer of 2015 by Brannon Dorsey with support from the SAIC OpenLab, SAIC  Shapiro Center, and a grant from the Motorola Corporation.

## Research Goals

1. Provide a general understanding of the ESP8266 hardware/software capabilities and limitations to be leveraged by the School of the Art Institute of Chicago (SAIC) community (and the online community at large), culminating in a one day ESP8266 related workshop at SAIC.
2. Gain familiarity with a range of ESP8266 supported SDKs and build environments (including [NodeMCU Lua](http://www.electrodragon.com/w/ESP8266_NodeMCU_Lua), [Arduino](https://github.com/esp8266/Arduino), [Open SDK](https://github.com/pfalcon/esp-open-sdk), [AT Commands](http://www.electrodragon.com/w/index.php)), learning and articulating the strengths and weaknesses of each "platform" so as to be able to best choose the appropriate means of development when starting new WiFi related projects. 
3. Outline and construct a prototype __low cost and low power__ mesh network that fulfills the definition as expressed in the [Defining a Mesh Network](https://github.com/olab-io/ESP8266-Research/blob/master/defining_mesh_network_for_esp8266.md) document.
4. Provide insight into future development and techniques that can be employed by ESP8266 and other WiFi enabled microcontroller chips.

## 1. Understanding and Presenting ESP8266

The allure and popularity of the ESP8266 System on a Chip (SoC) lies primarily in its low cost (~$3 a board) and high-level capabilities to behave as an 802.11 WiFi Access Point/Station. The ESP8266 is the first product of its type to provide these WiFi features at such a low cost ($15+ dollars cheaper than similar featured devices) and on such a small and embeddable platform. 

The ESP8266 is the first SoC I have developed with besides the Arduino platform, and it presents several challenges that make it difficult to use. In understanding and presenting my research I am also challenged by the target audience: creative artists working at the intersection of the arts and technology with varying levels of skill and comfortability when it comes to programming and networking practices.

### Technical 

I have outlined some of the challenges I faced when using the ESP8266 chip in [this brief troubleshooting guide](https://github.com/olab-io/ESP8266-Research/blob/master/troubleshooting.md). They can be summarized as follows:

- Programming Interface: The ESP8266 SoC is made available in many different breakouts ([ESP-01](http://l0l.org.uk/wp-content/uploads/2014/12/ESP01v0.jpg) and [ESP-12](http://esp8266.ru/wp-content/images/esp8266-modules/ESP8266_esp-12.jpg) being perhaps the most common) and development boards ([NodeMCU](http://hackaday.com/2015/01/01/a-dev-board-for-the-esp-lua-interpreter/), [Adafruit HUZZAH](https://www.adafruit.com/products/2471), [SparkFun ESP8266 Thing](https://www.sparkfun.com/products/13231)). Unless you opt to purchase a development board, which significantly raises the cost of each ESP8266, you will have to program the devices using a USB->FTDI programmer attached to the Rx and Tx pins on your SoC. It is also possible to use an Arduino instead of an FTDI programmer. More on that [here](http://www.instructables.com/id/Arduino-Examples-2-Use-an-Arduino-as-a-FTDI-Progr/).
- Power issues: The ESP8266 requires 3.3v and 250mA to behave correctly. However, when flashing new firmware to the device, it may require as much as 350mA of current. See the [Power Issues](https://github.com/olab-io/ESP8266-Research/blob/master/troubleshooting.md#power-issues) section of the troubleshooting document for more info.
- Aggressive Watchdog Timer: A [watchdog timer](https://en.wikipedia.org/wiki/Watchdog_timer) is a common occurrence in SoCs but may be a new concept for people that haven't done much work with them. The watchdog in the ESP8266 is a hardware timer that resets the device if execution of custom code takes longer than ~1 sec without releasing control to the WiFi stack managed by the ESP8266 itself. If you find that your device keeps resetting, you may need to pause your custom code execution, handing off execution to the ESP8266's internal WiFi code. With Arduino, this is done using the `delay(...)` function, however some method for this exists in each SDK/Environment.

### Workshop

In November 2015, the I co-organized _Closer Than Your Skin: A Networked Wearables Workshop_ with Christopher Baker and Sasha De Koninick at the School of the Art Institute of Chicago. The event lasted one day and was attended by 20 students from the school, who formed 4 groups that each developed an project using the ESP8266 throughout the course of workshop. The participants backgrounds ranged from art and technology to fashion and fibers, however most all atendees had no experience with the ESP8266 before the day of the event. In preperation for the workshop we fabricated 16 custom printed circuit boards that provided breakouts to easily add two buttons and two haptic vibration motors to each group's project. My summer research provided a comforatibility with the ESP8266 Arduino library allowing me to help individual groups throughout the day, providing group members an oppertunity to learn about the ESP8266 software development process while also focusing on the presentation and design of their wearable garments.

Some of the projects built during the workshop include:

- A bike helmet that uses LEDs to display the rider's heart rate
- A broach that vibrates when it is within the proximity of certain WiFi networks
- Custom embedded hats that augment the traditional roundtable game of "Assassin"

## 2. ESP8266 SDKs and Build Environments

A large part of my ESP8266 related research was devoted to using and familiarizing myself with the various tools and languages that can be used to program the devices. Below include a short list of the strength and weaknesses of some of the most popular SDKs and environments that I experimented with.

#### [AT Commands](http://www.electrodragon.com/w/index.php?title=Category:ESP8266_Firmware_and_SDK&redirect=no)
The AT Command firmware (aka `Ai-Thinker`) is the default firmware that is developed by ESPRESSIF and shipped with the chips. It uses a [list of commands](http://www.pridopia.co.uk/pi-doc/ESP8266ATCommandsSet.pdf) that are sent serially to the devices to configure different modes and behaviors supported by the chips.

__Pros:__ Easy to use as a first interface with the chip, as it only requires a serial port. Can provide a good understanding of some of the common uses and capabilities of the ESP8266.

__Cons:__ Limited programatic control without some sort of "controller" device that is connected to the ESP8266 via serial. 

#### [Open SDK](https://github.com/pfalcon/esp-open-sdk)
The Open SDK is an effort to provide open source (as much as possible) tools to build custom ESP8266 firmware using the official ESPRESSIF SDK.

__Pros:__ The Open SDK provides the most possible control available with the SDK. New releases of the ESPRESSIF firmware also incorporate features that may not yet be provided in other environments.

__Cons:__ Documentation is somewhat sparse, and programming in architecture specific C or C++ can be daunting to many people.

#### [ESP8266 Arduino](https://github.com/esp8266/Arduino)
The Arduino core and IDE is made available for ESP8266 development via the Arduino board manager with Arduino v1.6.5.

__Pros:__ Arduino is perhaps the most used DIY/Hobbiest microcontroller platform. This environment is by far the most approachable for users already familiar with Arduino or just getting started with working with microcontrollers. It also for easily portable code between traditional Arduino boards and the ESP8266.

__Cons:__ Because ESP8266 Arduino must be Arduino compatible I've found the library to be somewhat limiting in terms of some of the native capabilities possible with the ESP8266.

#### [NodeMCU w/ ESPlorer](http://www.electrodragon.com/w/ESP8266_NodeMCU_Lua)
NodeMCU is a Lua binding providing high-level access to the ESPRESSIF SDK via a scripting language. ESPlorer is a full-featured IDE that even provides a small file system on the ESP8266 allowing you to save, run, and include multiple `.lua` files.

__Pros:__ ESPlorer is hands down the most powerful tool that I have come across to author software that runs on the ESP8266. The benifits provided by a scripting language, not having to compile and flash with each change, also significanly increase development speed.

__Cons:__ Lua is a somewhat unfamiliar programming language for most people. If you know Lua, this really is the best development environment unless you need the kind of control that the Open SDK provides.

## 3. Mesh Network

My initial interest in the ESP8266 was brought on by a desire to implement a scalable and distributed mesh network using the devices. For my purposes, I define a mesh network, as intended to be implemented entirely with ESP8266s as network nodes/repeaters, to have the following qualities:

An ESP8266 Mesh Network __is__ a network:

1. Where any node in the network can communicate with any other node in the network
2. Where the range is determined and increased by each node in the network

An ESP8266 Mesh Network __may__:

1. Be a hidden network that pre-programmed ESP8266 devices automatically connect to without SSID advertisement.
2. Be a visible network that any device may connect to like a traditional wifi Access Point. 
3. Have the functionality to piggy-back off of an existing public wifi network to boost connection availability and speed.

Once I had outlined these criteria, and had become familiar enough with basic programming of the ESP8266, I made the decision that I wanted to build the mesh network on top of the [Arduino WiFi library](https://www.arduino.cc/en/Reference/WiFi) as it provided enough basic functionality to build an informal network protocol/system so as not to have to implement a custom low-level ESPRESSIF code.If you are familiar with the [OSI Network Model](https://en.wikipedia.org/wiki/OSI_model), my Mesh Network implementation would sit at the highest level, the Application layer.

### Meshish

Meshish was proposed as a solution. The premise of Meshish is that it creates a network using multiple ESP8266 nodes each opperating as both an access point and a station/client simultaneously (or AP/STA Mode). In Meshish, there are two types of nodes, primary and secondary. Primary nodes serve as the main 'routing' nodes that facilitate communication between the multiple ESP8266 secondary clients that may be connected to it. Each primary node then connects to another primary node to act as a gateway between their "subnets", forwarding communications allowing a secondary node attatched to one primary to talk to a secondary node attatched to a different primary.

#### How it works

Because the process of [leader election](https://en.wikipedia.org/wiki/Leader_election) can be a somewhat difficult task, and I was hoping to build a proof of concept implementation of library, I chose to predetermine which ESP8266 nodes would be primary and which ones would act as secondary nodes. When any Meshish node powers on, it imediately places itself in STA/AP Mode, and does one of two things, regardless if it's a secondary or primary node:

- It advertises its network and services by broadcasting an SSID that notifies nearby nodes:
	1. That is itself a Meshish node, remember the world is full of many WiFi networks
	2. Wether it is a primary or secondary Meshish node
	3. Its IP address on the network that it currently belongs
- Attempts to connect to a primary node that it "sees".

An example SSID of a Meshish node might look like this:

```
Mesh_1_192.168.4.1_28946321
```

Where `Mesh` is the preffix used to indicate this SSID belongs to a Meshish node, `1` indicates the node is primary (`0` if secondary), `192.168.4.1` the node's IP address, and `28946321`, the node's chip id.

implied heirarchy so its only "Mesh-ish"

## 4. Future Research
