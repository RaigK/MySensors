/*
 Name:		MySensors3.ino
 Created:	10/6/2020 9:22:59 PM
 Changed:   14/02/2024
 Author:	raigk
*/

/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2020 Sensnology AB
 * Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 *
 * DESCRIPTION
 * ATC mode settings and signal report functions, on RFM69 and RFM95 nodes
 *
 */

//#include <MyConfig.h>
#define val_name(v)#v

 // Enable debug prints
#include <EEPROM.h>
#define MY_DEBUG
#define MY_NODE_ID 102
//#define MY_NODE_ID 121
#define MY_SPLASH_SCREEN_DISABLED
//#define MY_DISABLED_SERIAL


// Enable signal report functionalities
#define MY_SIGNAL_REPORT_ENABLED

// Optimizations when running on 2032 Coin Cell. Also set nodeManager.setSleepBetweenSend(500) and run the board at 1Mhz
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_TRANSPORT_WAIT_READY_MS  5000
#define MY_SLEEP_TRANSPORT_RECONNECT_TIMEOUT_MS 2000
//#define MY_PARENT_NODE_ID 0
//#define MY_PARENT_NODE_IS_STATIC

// Enable and select radio type attached

// RFM69
#define MY_RADIO_RFM69
#define MY_IS_RFM69HW
#define MY_RFM69_NEW_DRIVER   // ATC on RFM69 works only with the new driver (not compatible with old=default driver)
#define MY_RFM69_ATC_TARGET_RSSI_DBM (-80)  // target RSSI -70dBm
#define MY_RFM69_TX_POWER_DBM (6)
#define MY_RFM69_MAX_POWER_LEVEL_DBM (0)

// RFM95
//#define MY_RADIO_RFM95
//#define MY_RFM95_ATC_TARGET_RSSI_DBM (-70)  // target RSSI -70dBm
//#define MY_RFM95_MAX_POWER_LEVEL_DBM (10)   // max. TX power 10dBm = 10mW

//#define SHT21
#define DALLAS
//#define HDC20x0
//#define _MS5803
#define BATTERYTEST

#ifdef _MS5803

#include <Wire.h>
#include <MS5803.h>
// Create the sensor object
MS5803 sensor;
// Create variables to store results
float temperature;
double pressure; //pressure_relative, altitude_delta, pressure_baseline;

#endif // MS5803

#include <MySensors.h>

#ifdef DALLAS
// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3 
#define ONE_WIRE_PULLUP 4
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress Water, Air;

uint16_t sensorcount;
#endif

#ifdef SHT21
#include <Wire.h>
#include <Sodaq_SHT2x.h>
#endif // 



#ifdef HDC20x0

#include <HDC2080.h>

#define ADDR 0x40
HDC2080 sensor(ADDR);

//float temperature = 0, humidity = 0;
#endif

#define BATTEST_PIN  A1  // Arduino Digital I/O pin number
int BATTERY_SENSE_PIN = A0;  // select the input pin for the battery sense point
int sensorValue;
int oldBatteryPcnt, oldBatteryVolt = 0;
uint32_t SLEEP_TIME = 120000;  // sleep time between reads (seconds * 1000 milliseconds)


// ID of the sensor child
#define CHILD_ID_UPLINK_QUALITY (10)
#define CHILD_ID_TX_LEVEL       (10)
#define CHILD_ID_TX_PERCENT     (10)
#define CHILD_ID_TX_RSSI        (10)
#define CHILD_ID_RX_RSSI        (10)
#define CHILD_ID_TX_SNR         (10)
#define CHILD_ID_RX_SNR         (10)
#define CHILD_ID_BAT			(12)
#define CHILD_ID_LOWBAT			(13)

#define CHILD_ID_TEMP			(1)
#define CHILD_ID_HUMI			(2)
#define CHILD_ID_BARO			(3)
#define CHILD_ID_DALLAS			(20)


// Initialize general message

MyMessage msgTxRSSI(CHILD_ID_TX_RSSI, V_VAR1);
MyMessage msgRxRSSI(CHILD_ID_RX_RSSI, V_VAR2);
MyMessage msgTxSNR(CHILD_ID_TX_SNR, V_VAR3);
MyMessage msgRxSNR(CHILD_ID_RX_SNR, V_VAR4);
MyMessage msgTxLevel(CHILD_ID_TX_LEVEL, V_VAR5);
//MyMessage msgTxPercent(CHILD_ID_TX_PERCENT, V_CUSTOM);
//MyMessage msgUplinkQuality(CHILD_ID_UPLINK_QUALITY, V_CUSTOM);
MyMessage msgBat(CHILD_ID_BAT, V_VOLTAGE);
MyMessage msgLowBat(CHILD_ID_LOWBAT, V_STATUS);

void setup()
{
	pinMode(BATTEST_PIN, OUTPUT);
	digitalWrite(BATTEST_PIN, HIGH);// enable bat divider

	Serial.begin(MY_BAUD_RATE);
	// Start up the library

#ifdef DALLAS

	// locate devices on the bus
	Serial.print("Locating devices...");
	Serial.print("Found ");
	Serial.print(sensorcount, DEC);
	Serial.println(" devices.");

	// report parasite power requirements
	Serial.print("Parasite power is: ");
	if (sensors.isParasitePowerMode()) Serial.println("ON");
	else Serial.println("OFF");

	// Search for devices on the bus and assign based on an index. Ideally,
	// you would do this to initially discover addresses on the bus and then
	// use those addresses and manually assign them (see above) once you know
	// the devices on your bus (and assuming they don't change).
	//
	// method 1: by index
	if (!sensors.getAddress(Water, 0)) Serial.println("Unable to find address for Device 0");
	if (!sensors.getAddress(Air, 1)) Serial.println("Unable to find address for Device 1");
//	if (!sensors.getAddress(Misc, 2)) Serial.println("Unable to find address for Device 2");
#endif

#ifdef _MS5803
	// Begin class with selected I2C address of sensor and max pressure range
	// available addresses (selected by jumper on board)
	//  ADDRESS_HIGH = 0x76
	//  ADDRESS_LOW  = 0x77
	sensor.begin(0x76, 14);
	//Retrieve calibration constants for conversion math.
	sensor.reset();
	sensor.getPressure(ADC_4096);
#endif

#ifdef SHT21
	Wire.begin();
#endif
#ifdef HDC20x0
	// Initialize I2C communication
	sensor.begin();

	// Begin with a device reset
	sensor.reset();
	// Configure Measurements
	sensor.setMeasurementMode(TEMP_AND_HUMID);  // Set measurements to temperature and humidity
	sensor.setRate(MANUAL);                     // Set measurement frequency to 1 Hz
	sensor.setTempRes(FOURTEEN_BIT);
	sensor.setHumidRes(FOURTEEN_BIT);
	sensor.triggerMeasurement();

	// use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
	analogReference(INTERNAL1V1);
#else
	analogReference(INTERNAL);
//	analogReference(DEFAULT);
	sensorValue = analogRead(BATTERY_SENSE_PIN);

#endif


#endif

	analogReference(INTERNAL);
	sensorValue = analogRead(BATTERY_SENSE_PIN);
}

#ifdef DALLAS
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		// zero pad the address if necessary
		if (deviceAddress[i] < 16) Serial.print("0");
		Serial.print(deviceAddress[i], HEX);
	}
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
	float tempC = sensors.getTempC(deviceAddress);
	if (tempC == DEVICE_DISCONNECTED_C)
	{
		Serial.println("Error: Could not read temperature data");
		return;
	}
	Serial.print("Temp C: ");
	Serial.print(tempC);
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
	Serial.print("Resolution: ");
	Serial.print(sensors.getResolution(deviceAddress));
	Serial.println();
}

// main function to print information about a device
void printData(DeviceAddress deviceAddress)
{
	Serial.print("Device Address: ");
	printAddress(deviceAddress);
	Serial.print(" ");
	printTemperature(deviceAddress);
	Serial.println();
}

#endif // !Dallas



void presentation()
{
#ifdef DALLAS
	pinMode(ONE_WIRE_PULLUP, OUTPUT);
	digitalWrite(ONE_WIRE_PULLUP, HIGH);
		// Start up the library
	Serial.print("Present Dallas begin");
	sensors.begin();
	sensorcount = sensors.getDeviceCount();
	Serial.println(sensorcount);
	for (uint16_t i = 0; i < sensorcount; i++) {
		present(CHILD_ID_DALLAS + i, S_TEMP, "Temperature");
		delay(500);
	}
	sendSketchInfo("Dallas", "2.0");
	Serial.print("Present Dallas end");
#endif // Dallas

#ifdef _MS5803
	sendSketchInfo("MS5803+ATC", "2.0");
	present(CHILD_ID_BARO, S_BARO, "Air Pressure");
	present(CHILD_ID_TEMP, S_TEMP, "Temperature");

#endif // _MS5803


	// Send the sketch version information to the gateway and controller
#ifdef SHT21
	sendSketchInfo("SHT21+ATC", "1.2");
#endif // SHT21

#ifdef HDC20x0
	sendSketchInfo("HDC20x0+ATC", "1.2");
#endif
#ifdef SHT21 || HDC2080
	present(CHILD_ID_TEMP, S_TEMP, "Temperature");
	present(CHILD_ID_HUMI, S_HUM, "Humidity");
#endif
	present(CHILD_ID_BAT, S_MULTIMETER, "Voltage");
	present(CHILD_ID_LOWBAT, S_BINARY, "Low Bat");
	// Register all sensors to gw (they will be created as child devices)
//	present(CHILD_ID_UPLINK_QUALITY, S_CUSTOM, "UPLINK QUALITY RSSI");
	present(CHILD_ID_TX_LEVEL, S_CUSTOM, "TX LEVEL DBM");
//	present(CHILD_ID_TX_PERCENT, S_CUSTOM, "TX LEVEL PERCENT");
	present(CHILD_ID_TX_RSSI, S_CUSTOM, "TX RSSI");
	present(CHILD_ID_RX_RSSI, S_CUSTOM, "RX RSSI");
//	present(CHILD_ID_TX_SNR, S_CUSTOM, "TX SNR");
//	present(CHILD_ID_RX_SNR, S_CUSTOM, "RX SNR");
	send(msgLowBat.set(false));

}

void loop()
{
	Serial.println("wake up");
#ifdef BATTERYTEST  
	pinMode(BATTEST_PIN, OUTPUT);
	digitalWrite(BATTEST_PIN, HIGH);// enable bat divider
	wait(10);
#endif

#ifdef _MS5803
	static MyMessage msgBaro(CHILD_ID_BARO, V_PRESSURE);
	static MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
	temperature = sensor.getTemperature(CELSIUS, ADC_4096);
	pressure    = sensor.getPressure(ADC_4096);
	send(msgTemp.set(temperature, 2));
	send(msgBaro.set(pressure, 2));

#endif // _MS5803

#ifdef DALLAS
	float tempC;
	pinMode(ONE_WIRE_PULLUP, OUTPUT);
	digitalWrite(ONE_WIRE_PULLUP, HIGH);
	// call sensors.requestTemperatures() to issue a global temperature
// request to all devices on the bus
	sensors.requestTemperatures();
#ifdef My_DEBUG
	Serial.print("Requesting temperatures...");
//	sensors.requestTemperatures();
	Serial.println("DONE");
	// print the device information
#endif //My_Debug
	
	static MyMessage msgTemp1(CHILD_ID_DALLAS, V_TEMP);
	static MyMessage msgID1(CHILD_ID_DALLAS, V_ID);
	tempC = sensors.getTempC(Air);
	send(msgTemp1.set(tempC, 2));
	send(msgID1.set("Thermo_Air"));

	static MyMessage msgTemp2(CHILD_ID_DALLAS+1, V_TEMP);
	static MyMessage msgID2(CHILD_ID_DALLAS+1, V_ID);
	tempC = sensors.getTempC(Water);
	send(msgTemp2.set(tempC, 2));
	send(msgID2.set("Thermo_Water"));
/*
	static MyMessage msgTemp3(CHILD_ID_DALLAS+2, V_TEMP);
	static MyMessage msgID3(CHILD_ID_DALLAS+2, V_ID);
	tempC = sensors.getTempC(Misc);
	send(msgTemp3.set(tempC, 2));
	send(msgID3.set("Thermo_Misc"));
*/	
#ifdef My_DEBUG
	printData(Air);
	printData(Water);
	printData(Misc);
#endif // My_DEBUG

#endif // Dallas



#ifdef SHT21
	// Read temperature & humidity from sensor.
	const float temperature = float(SHT2x.GetTemperature());
	const float humidity = float(SHT2x.GetHumidity());
#endif
#ifdef HDC20x0
	//begin measuring
	sensor.triggerMeasurement();

	const float temperature = sensor.readTemp();
	const float humidity    = sensor.readHumidity();

#endif // HDC2080
#ifdef SHT21 || HDC2080
	static MyMessage msgHumi(CHILD_ID_HUMI, V_HUM);
	static MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
	send(msgTemp.set(temperature, 2));
	send(msgHumi.set(humidity, 2));

#ifdef MY_DEBUG
	Serial.print(F("Temp "));
	Serial.print(temperature);
	Serial.print('C');
	Serial.print(F("\tHum "));
	Serial.println(humidity);
#endif
#endif
	// get the battery Voltage
	sensorValue = analogRead(BATTERY_SENSE_PIN);
	int batteryPcnt = sensorValue / 10;
	float batteryV = sensorValue * 0.005057;//calib factor ref 1.1
//	float batteryV = sensorValue * 0.01379 * 0.9999;//calib factor ref 3.0
//	float batteryV = sensorValue * 0.01287 * 1.0;//calib factor ref 2.8

	if (batteryV <= 3.0) { 
		Serial.print("Battey is empty");
		send(msgLowBat.set(true));
	}
  else {send(msgLowBat.set(false));}

// send messages to GW
//	send(msgUplinkQuality.set(transportGetSignalReport(SR_UPLINK_QUALITY)));
	send(msgTxLevel.set(transportGetSignalReport(SR_TX_POWER_LEVEL)));
//	send(msgTxPercent.set(transportGetSignalReport(SR_TX_POWER_PERCENT)));
	// retrieve RSSI / SNR reports from incoming ACK
	send(msgTxRSSI.set(transportGetSignalReport(SR_TX_RSSI)));
	send(msgRxRSSI.set(transportGetSignalReport(SR_RX_RSSI)));
//	send(msgTxSNR.set(transportGetSignalReport(SR_TX_SNR)));
//	send(msgRxSNR.set(transportGetSignalReport(SR_RX_SNR)));
	// wait a bit
#ifdef BATTERYTEST  
	digitalWrite(BATTEST_PIN, LOW);// disable bat divider
#endif
	// 1M, 270K divider across battery and using internal ADC ref of 1.1V
	// Sense point is bypassed with 0.1 uF cap to reduce noise at that point
	// ((1e6+270e3)/270e3)*1.1 = Vmax = 5.174 Volts
	// 5.174/1023 = Volts per bit = 0.005057


#ifdef MY_DEBUG
	Serial.print("Battery Voltage: ");
	Serial.print(batteryV);
	Serial.println(" V");

	Serial.print("Battery percent: ");
	Serial.print(batteryPcnt);
	Serial.println(" %");
#endif
//	send(msgBat.set(batteryV, 2));
	if (oldBatteryVolt != batteryV) {
		// Power up radio after sleep
		send(msgBat.set(batteryV, 2));
		oldBatteryPcnt = batteryV;
	}
	if (oldBatteryPcnt != batteryPcnt) {
		// Power up radio after sleep
		sendBatteryLevel(batteryPcnt);
		oldBatteryPcnt = batteryPcnt;
	}
#ifdef DALLAS
	digitalWrite(ONE_WIRE_PULLUP, LOW);
#endif DALLAS
	Serial.println("go sleep");
	sleep(SLEEP_TIME, true);
}
