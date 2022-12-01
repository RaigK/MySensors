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
#ifndef Sensor_DigitalInput_h
#define Sensor_DigitalInput_h

#define  Sensor_DigitalInput_Debug  OFF
/*
SensorDigitalInput: read the digital input of the configured pin
*/
class Sensor_DigitalInput: public Sensor {
protected:
	bool _invert_value_to_report = false;
	int _initial_value = -1;
	int _last_value = 0;
	int _pinvalue = 0;
	
public:
	Sensor_DigitalInput(int8_t pin, uint8_t child_id = 0): Sensor(pin) {
		_name = "DIGI_I";
		children.allocateBlocks(1);
		new Child(this,INT,nodeManager.getAvailableChildId(child_id),S_DOOR,V_ARMED,_name);
	};
	
	// Invert the value to report. E.g. report 1 if value is LOW, report 0 if HIGH (default: false) 
	void setInvertValueToReport(bool value) {
		_invert_value_to_report = value;
	};
	
	// Set optional internal pull up/down
	void setInitialValue(int value) {
		_initial_value = value;
	};

	// define what to do during setup
	void onSetup() {
		// set the pin for input
		pinMode(_pin, INPUT);
		// set internal pull up/down
		if (_initial_value > -1) digitalWrite(_pin,_initial_value);
#if Sensor_DigitalInput_Debug == ON
		MY_SERIALDEVICE.print("Setup Pin ");
		MY_SERIALDEVICE.println(_pin);
#endif
	};

	// define what to do during loop
	void onLoop(Child* child) {
		// read the value
		_pinvalue = digitalRead(_pin);
		if (_first_run) {
			_last_value = _pinvalue;
			child->setType(V_TRIPPED);
			child->setValue(_pinvalue);

#if Sensor_DigitalInput_Debug == ON
			MY_SERIALDEVICE.print("Type  ");
			MY_SERIALDEVICE.println(child->getType());
#endif			

		}
#if Sensor_DigitalInput_Debug == ON
			MY_SERIALDEVICE.print("ChilId ");
			MY_SERIALDEVICE.println(child->getChildId());
			MY_SERIALDEVICE.print(" Pin=");
			MY_SERIALDEVICE.println(_pinvalue);
#endif
		if (_pinvalue != _last_value){
			_last_value = _pinvalue;	
			// invert the value if needed
			if (_invert_value_to_report) _pinvalue = !_pinvalue;
			// store the value
			child->setType(V_TRIPPED);
#if Sensor_DigitalInput_Debug == ON
			MY_SERIALDEVICE.print(" Read Pin ");
			MY_SERIALDEVICE.println(_pinvalue);
#endif
			child->setValue(_pinvalue);
		}
	
	};
};
#endif