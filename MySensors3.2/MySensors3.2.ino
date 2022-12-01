/*
 Name:		MySensors3.ino
 Created:	10/6/2020 9:22:59 PM
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
 * REVISION HISTORY
 * Version 1.1 - tekka
 *
 * DESCRIPTION
 * ATC mode settings and signal report functions, on RFM69 and RFM95 nodes
 *
 */

 // Enable debug prints
#define MY_DEBUG
#define MY_NODE_ID 103
#define MY_SPLASH_SCREEN_DISABLED


// Enable signal report functionalities
#define MY_SIGNAL_REPORT_ENABLED

// Optimizations when running on 2032 Coin Cell. Also set nodeManager.setSleepBetweenSend(500) and run the board at 1Mhz
//#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
//#define MY_TRANSPORT_WAIT_READY_MS  5000
//#define MY_SLEEP_TRANSPORT_RECONNECT_TIMEOUT_MS 2000
//#define MY_PARENT_NODE_ID 0
//#define MY_PARENT_NODE_IS_STATIC

// Enable and select radio type attached

// RFM69
#define MY_RADIO_RFM69
#define MY_IS_RFM69HW
#define MY_RFM69_NEW_DRIVER   // ATC on RFM69 works only with the new driver (not compatible with old=default driver)
#define MY_RFM69_ATC_TARGET_RSSI_DBM (-70)  // target RSSI -70dBm
#define MY_RFM69_TX_POWER_DBM (0)
#define MY_RFM69_MAX_POWER_LEVEL_DBM (0)

// RFM95
//#define MY_RADIO_RFM95
//#define MY_RFM95_ATC_TARGET_RSSI_DBM (-70)  // target RSSI -70dBm
//#define MY_RFM95_MAX_POWER_LEVEL_DBM (10)   // max. TX power 10dBm = 10mW

#ifdef SHT21
#include <Wire.h>
#include <Sodaq_SHT2x.h>

#endif // 

#include <MySensors.h>

#define HDC20x0
#ifdef HDC20x0

#include <HDC2080.h>

#define ADDR 0x40
HDC2080 sensor(ADDR);

//float temperature = 0, humidity = 0;
#endif

// ID of the sensor child
#define CHILD_ID_UPLINK_QUALITY (0)
#define CHILD_ID_TX_LEVEL       (1)
#define CHILD_ID_TX_PERCENT     (2)
#define CHILD_ID_TX_RSSI        (3)
#define CHILD_ID_RX_RSSI        (4)
#define CHILD_ID_TX_SNR         (5)
#define CHILD_ID_RX_SNR         (6)
#define CHILD_ID_HUMI  (10)
#define CHILD_ID_TEMP  (11)


// Initialize general message
MyMessage msgTxRSSI(CHILD_ID_TX_RSSI, V_CUSTOM);
MyMessage msgRxRSSI(CHILD_ID_RX_RSSI, V_CUSTOM);
//MyMessage msgTxSNR(CHILD_ID_TX_SNR, V_CUSTOM);
//MyMessage msgRxSNR(CHILD_ID_RX_SNR, V_CUSTOM);
//MyMessage msgTxLevel(CHILD_ID_TX_LEVEL, V_CUSTOM);
//MyMessage msgTxPercent(CHILD_ID_TX_PERCENT, V_CUSTOM);
//MyMessage msgUplinkQuality(CHILD_ID_UPLINK_QUALITY, V_CUSTOM);

void setup()
{
	Serial.begin(MY_BAUD_RATE);
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

#endif

}


void presentation()
{
	// Send the sketch version information to the gateway and controller
#ifdef SHT21
	sendSketchInfo("SHT21+ATC", "1.1");
#endif // SHT21

#ifdef HDC20x0
	sendSketchInfo("HDC20x0+ATC", "1.1");
#endif
	// Register all sensors to gw (they will be created as child devices)
//	present(CHILD_ID_UPLINK_QUALITY, S_CUSTOM, "UPLINK QUALITY RSSI");
//	present(CHILD_ID_TX_LEVEL, S_CUSTOM, "TX LEVEL DBM");
//	present(CHILD_ID_TX_PERCENT, S_CUSTOM, "TX LEVEL PERCENT");
	present(CHILD_ID_TX_RSSI, S_CUSTOM, "TX RSSI");
	present(CHILD_ID_RX_RSSI, S_CUSTOM, "RX RSSI");
//	present(CHILD_ID_TX_SNR, S_CUSTOM, "TX SNR");
//	present(CHILD_ID_RX_SNR, S_CUSTOM, "RX SNR");
	present(CHILD_ID_HUMI, S_HUM, "Humidity");
	present(CHILD_ID_TEMP, S_TEMP, "Temperature");

}

void loop()
{
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
	// send messages to GW
//	send(msgUplinkQuality.set(transportGetSignalReport(SR_UPLINK_QUALITY)));
//	send(msgTxLevel.set(transportGetSignalReport(SR_TX_POWER_LEVEL)));
//	send(msgTxPercent.set(transportGetSignalReport(SR_TX_POWER_PERCENT)));
	// retrieve RSSI / SNR reports from incoming ACK
	send(msgTxRSSI.set(transportGetSignalReport(SR_TX_RSSI)));
	send(msgRxRSSI.set(transportGetSignalReport(SR_RX_RSSI)));
//	send(msgTxSNR.set(transportGetSignalReport(SR_TX_SNR)));
//	send(msgRxSNR.set(transportGetSignalReport(SR_RX_SNR)));
	// wait a bit
	smartSleep(10000);
}