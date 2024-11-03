# NodeMCUv3-MQTT-temperature
ino code for NodeMCUv3 and some python for mqtt messages handling 
# installation
##arduino and python script
install Libarary in arduino using add .zip library
install requiremtns to run python script in its dir

## EXQM - web MQTT broker
1. make free account,
2. copy CA_cert and save it for later
3. deploy deployment

## docker desktop
run influxdb server and grafana server on docker
make sure they are running on same network so grafana could ping influxDBserver
configure them and mofidy files accordingly

## build prototype
build proptoype following installtion in .png file with exceptions
Data cable from dallas termometr goes to D3
LCD_I2C SDA to D2
LCD_I2C SCL to D1
LCD_I2C VCC to VV
LCD_I2C GND to G 

## building in arduino IDE
Add config file for NodeMCUv3:
1. Enter arudino IDE, click File -> Preferences, on the bottom paste this link in input: http://arduino.esp8266.com/stable/package_esp8266com_index.json
2. select board NodeMCU 1.0 (ESP-12E Module) and the right COM