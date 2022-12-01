/*
* The MySensors Arduino library handles the wireless radio link and protocol
* between your home built sensors/actuators and HA controller of choice.
* The sensors forms a self healing radio network with optional repeaters. Each
* repeater and gateway builds a routing tables in EEPROM which keeps track of the
* network topology allowing messages to be routed to nodes.
*
* Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
* Copyright (C) 2013-2017 Sensnology AB
* Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
*
* Documentation: http://www.mysensors.org
* Support Forum: http://forum.mysensors.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*/
#ifndef SensorSHT21_Dewpoint_h
#define SensorSHT21_Dewpoint_h

/*
SensorSHT21: temperature and humidity sensor
*/

#include <Wire.h>
#include <Sodaq_SHT2x.h>

class SensorSHT21_Dewpoint : public Sensor {
public:
	SensorSHT21_Dewpoint(uint8_t child_id = 0) : Sensor(-1) {
#if MeasureSHT21_Dewpoint  == ON
		children.allocateBlocks(3);
#else		
		children.allocateBlocks(2);
#endif		

		new Child(this, FLOAT, nodeManager.getAvailableChildId(child_id), S_TEMP, V_TEMP, "Temperature_SHT");
		new Child(this, FLOAT, child_id > 0 ? nodeManager.getAvailableChildId(child_id + 1) : nodeManager.getAvailableChildId(child_id), S_HUM, V_HUM, "Humidity_SHT");
#if MeasureSHT21_Dewpoint  == ON
		new Child(this, FLOAT, child_id > 0 ? nodeManager.getAvailableChildId(child_id + 2) : nodeManager.getAvailableChildId(child_id), S_TEMP, V_TEMP, "Dewpoint_SHT");
#endif		
	};

	// what to do during setup
	void onSetup() {
		// initialize the library
		Wire.begin();
	};

	// what to do during loop
	void onLoop(Child* child) {
		// temperature sensor
		if (child->getChildId() == 1) {
			if (child->getType() == V_TEMP) {
				// read the temperature
				float temperature = SHT2x.GetTemperature();
				// convert it
				//temperature = nodeManager.celsiusToFahrenheit(temperature);
				// store the value
				child->setValue(temperature);
			}
		}
			// Humidity Sensor
		if (child->getChildId() == 2) {
			if (child->getType() == V_HUM) {
				// read humidity
				float humidity = SHT2x.GetHumidity();
				// store the value
				child->setValue(humidity);
			}
		}
		// dewpoint
#if MeasureSHT21_Dewpoint  == ON
		if (child->getChildId() == 3) {
			// Humidity Sensor
			if (child->getType() == V_TEMP) {
				// read humidity
				float humidity = SHT2x.GetDewPoint();
				// store the value
				child->setValue(humidity);
			}
		}
#endif		
	};
};
#endif