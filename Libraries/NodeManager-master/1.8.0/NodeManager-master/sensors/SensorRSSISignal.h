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
#ifndef SensorRSSISignal_h
#define SensorRSSISignal_h

/*
SensorSignal: report RSSI signal strength from the radio
*/

#define MY_SIGNAL_REPORT_ENABLED
#define SIGNAL_CHILD_ID 202

class SensorRSSISignal: public Sensor {
protected:
	int _signal_command = SR_RX_RSSI;
	int i = 0;
/*	enum RSSI_Type {
		SR_RX_RSSI = 0,            //!< SR_RX_RSSI
		SR_TX_RSSI = 1,            //!< SR_TX_RSSI
		SR_RX_SNR  = 2,             //!< SR_RX_SNR
		SR_TX_SNR  = 3,             //!< SR_TX_SNR
		SR_TX_POWER_LEVEL = 4,     //!< SR_TX_POWER_LEVEL
		SR_TX_POWER_PERCENT = 5,   //!< SR_TX_POWER_PERCENT
		SR_UPLINK_QUALITY= 6,     //!< SR_UPLINK_QUALITY
		SR_NOT_DEFINED  = 7       //!< SR_NOT_DEFINED
		} _rssi_t;
*/
public:
	SensorRSSISignal(uint8_t child_id = SIGNAL_CHILD_ID): Sensor(-1) {
		children.allocateBlocks(5);
		new Child(this, INT, child_id, S_CUSTOM, V_VAR1, "SIGNAL_QUALITY");

		// report signal level every 60 minutes by default
		setReportIntervalMinutes(60);
	};
	
	// [101] define which signal report to send. Possible values are SR_UPLINK_QUALITY, SR_TX_POWER_LEVEL, SR_TX_POWER_PERCENT, SR_TX_RSSI, SR_RX_RSSI, SR_TX_SNR, SR_RX_SNR (default: SR_RX_RSSI)
	void setSignalCommand(int value) {
//		_signal_command = value;
	};
	
	// define what to do during loop
	void onLoop(Child* child) {
		MY_SERIALDEVICE.print("RSSI_Typ->");
		MY_SERIALDEVICE.println(_signal_command);
		
		if(_signal_command > SR_UPLINK_QUALITY) _signal_command = SR_RX_RSSI;
		
		switch ( _signal_command )
		{
			case SR_RX_RSSI:
			{
				child->setType(V_VAR1);
				child->setValue(transportGetSignalReport((signalReport_t) _signal_command));
				MY_SERIALDEVICE.print("SR_RX_RSSI  ");
				MY_SERIALDEVICE.println(child->getValueInt());
			}
			break;
			case SR_TX_RSSI:
			{
				child->setType(V_VAR2);
				child->setValue(transportGetSignalReport((signalReport_t) _signal_command));
				MY_SERIALDEVICE.print("SR_TX_RSSI  ");
				MY_SERIALDEVICE.println(child->getValueInt());
			}
			break;
			case SR_TX_POWER_LEVEL:
			{
				child->setType(V_VAR3);
				child->setValue(transportGetSignalReport((signalReport_t) _signal_command));
				MY_SERIALDEVICE.print("SR_TX_Power_Level  ");
				MY_SERIALDEVICE.println(child->getValueInt());
			}
			break;
			case SR_TX_POWER_PERCENT:
			{
				child->setType(V_VAR4);
				child->setValue(transportGetSignalReport((signalReport_t) _signal_command));
				MY_SERIALDEVICE.print("SR_TX_Power_Percent ");
				MY_SERIALDEVICE.println(child->getValueInt());
			}
			break;
			case SR_UPLINK_QUALITY:
			{
				child->setType(V_VAR5);
				child->setValue(transportGetSignalReport((signalReport_t) _signal_command));
				MY_SERIALDEVICE.print("SR_Uplink_Quality ");
				MY_SERIALDEVICE.println(child->getValueInt());
			}
			break;
			default:
			MY_SERIALDEVICE.println("SR_Err  ");
			break;
		}
		_signal_command++;
	};
	
#if NODEMANAGER_OTA_CONFIGURATION == ON
	// define what to do when receiving an OTA configuration request
	void onOTAConfiguration(ConfigurationRequest* request) {
		switch(request->getFunction()) {
		case 101: setSignalCommand(request->getValueInt()); break;
		default: return;
		}
	};
#endif
};
#endif