# Vibration pattern sensor
Using Wemos D1 mini (esp8266) + SW-420 to detect vibration patterns and publish them to MQTT

<img src="http://irishelectronics.ie/WebRoot/Register365/Shops/950018241/55E3/2665/B630/EA5E/DC8F/C0A8/190C/C347/sensor1.JPG" width="128">

This can be used everytime you want to track the vibration/movement of something (washing machine/dryer/children jumping on the bed :D)

## Notes
```
#define N_SAMPLES 50
#define SAMPLE_INTERVAL 100
```
increase `N_SAMPLES` to increase the interval after which you consider the new_state valid
