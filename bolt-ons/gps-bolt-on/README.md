# GPS bolt on

## GPS Module
The GPS bolt on uses the Skytraq SUP500F. See the datasheet within this repository for programming information

## Pin configuration

```
* Rx to GPS module (tx from ESP32) - pin 17 of SV1 - GPIO40 of ESP32
* Tx from GPS module (rx to ESP32) - pin 18 of SV1 - GPIO41 of ESP32
* P1PPS from GPS module            - pin 16 of SV1 - GPIO39 of ESP32
* PSE_SEL from GPS moduel          - pin 11 of SV1 - GPIO45 of ESP32
```

