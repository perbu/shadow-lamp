# shadow-lamp
Demo project showcasing the use of shadow devices in a simple MCU-app.



## Handy stuff:

Build locally (requires Docker):
```
mos build --local
```

Enable OTA:
```
mos config-set dash.enable=true dash.token=DEADBEEF
```

Connect to AWS IoT Core - requires 
```
mos aws-iot-setup
```