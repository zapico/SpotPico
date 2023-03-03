# SpotPico
jorge@zapi.co 2023

Energy use and price screen using RPi Pico + TFT
It connects to MQTT to access electricity data from a connected smart meter using https://energyintelligence.se/ 

Instructions:
- Install Smart meter device and configure to your MQTT server.
- Change wifi and mqtt credentials in conf.h
- Change MQTT server and topics.
- Install and use Pico LittleFS data upload to upload font to memory.

Note about color on tft:
- Background colors are in 4 digit hex, but not RGB > Ox GRB T
For new colors use this and reverse RG https://dev.to/grahamthedev/hold-on-there-are-4-and-8-digit-hex-codes-for-colours-261
