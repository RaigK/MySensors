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
#ifndef SensorHDC10080_Dewpoint_h
#define SensorHDC10080_Dewpoint_h

/*
SensorHDC1080: temperature and humidity sensor
*/

#include <Wire.h>
//#include <Sodaq_SHT2x.h>
#include <ClosedCube_HDC1080.h>
#include <math.h>

ClosedCube_HDC1080 hdc1080;

// Specify the constants for water vapor and barometric pressure.
#define WATER_VAPOR 17.62f
#define BAROMETRIC_PRESSURE 243.5f


class SensorHDC1080_Dewpoint : public Sensor {
float temperature, humidity, dewpoint;
public:
	SensorHDC1080_Dewpoint(uint8_t child_id = 0) : Sensor(-1) {

#if MeasureHDC1080_Dewpoint  == ON
		children.allocateBlocks(3);
#else		
		children.allocateBlocks(2);
#endif		

		new Child(this, FLOAT, nodeManager.getAvailableChildId(child_id), S_TEMP, V_TEMP, "Temperature_HDC1080");
		new Child(this, FLOAT, child_id > 0 ? nodeManager.getAvailableChildId(child_id + 1) : nodeManager.getAvailableChildId(child_id), S_HUM, V_HUM, "Humidity_HDC1080");
#if MeasureHDC1080_Dewpoint == ON
		new Child(this, FLOAT, child_id > 0 ? nodeManager.getAvailableChildId(child_id + 2) : nodeManager.getAvailableChildId(child_id), S_TEMP, V_TEMP, "Dewpoint_HDC1080");
#endif		
	};
	
	// what to do during setup
	void onSetup() {
		// initialize the library
		//Wire.begin();
		hdc1080.begin(0x40);
		MY_SERIALDEVICE.print("Manufacturer ID=0x");
		MY_SERIALDEVICE.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
		MY_SERIALDEVICE.print("Device ID=0x");
		MY_SERIALDEVICE.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device

		//hdc1080.setResolution(HDC1080_RESOLUTION_14BIT, HDC1080_RESOLUTION_14BIT);
	};

	// what to do during loop
	void onLoop(Child* child) {
		// temperature sensor
		if (child->getChildId() == 1) {
			if (child->getType() == V_TEMP) {
				// read the temperature
				//float temperature = SHT2x.GetTemperature();
				temperature = hdc1080.readTemperature();
				// store the value
				child->setValue(temperature);
			}
		}
			// Humidity Sensor
		if (child->getChildId() == 2) {
			if (child->getType() == V_HUM) {
				// read humidity
				//float humidity = SHT2x.GetHumidity();
				humidity = hdc1080.readHumidity();
				// store the value
				child->setValue(humidity);
			}
		}
		
#if MeasureHDC1080_Dewpoint  == ON // dewpoint
		if (child->getChildId() == 3) {
			// Humidity Sensor
			if (child->getType() == V_TEMP) {
				// read humidity
				dewpoint = GetDewPoint();
				// store the value
				child->setValue(dewpoint);
			}
		}
#endif		
		MY_SERIALDEVICE.print("Temp ");
		MY_SERIALDEVICE.println(temperature);
		MY_SERIALDEVICE.print("Humi ");
		MY_SERIALDEVICE.println(humidity);
		MY_SERIALDEVICE.print("Dewp ");
		MY_SERIALDEVICE.println(dewpoint);

	};

#if MeasureHDC1080_Dewpoint  == ON
/**********************************************************
 * GetDewPoint
 *  Gets the current dew point based on the current humidity and temperature
 *
 * @return float - The dew point in Deg C
 **********************************************************/
float GetDewPoint(void)
{
//  float humidity = GetHumidity();
 // float temperature = GetTemperature();

  // Calculate the intermediate value 'gamma'
  float gamma = log(humidity / 100) + WATER_VAPOR * temperature / (BAROMETRIC_PRESSURE + temperature);
  // Calculate dew point in Celsius
  float dewPoint = BAROMETRIC_PRESSURE * gamma / (WATER_VAPOR - gamma);

  return dewPoint;
}
#endif
};
#endif