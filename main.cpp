#include "LoRaWan_APP.h"
#include <Arduino.h>
#include "driver/touch_sensor.h"
#include "driver/touch_sensor_common.h"
#include "soc/touch_sensor_channel.h"

touch_pad_t sensor1 = TOUCH_PAD_NUM2;
const uint8_t parking_id = 0x04; //sets to parking id location

uint8_t devEui[] = {0x70,0xB3,0xD5,0x7E,0xD0,0x06,0x71,0x64};  
bool overTheAirActivation = true;
uint8_t appEui[] = {0x77,0x77,0x77,0x77,0x77,0x78,0x78,0x78};  // you should set whatever your TTN generates. TTN calls this the joinEUI, they are the same thing. 
uint8_t appKey[] = {0x0F,0xFE,0xB6,0x24,0x5A,0xF7,0x28,0x59,0x13,0xE1,0xD8,0x8D,0x3A,0xEC,0x9C,0x75};  // you should set whatever your TTN generates 

//These are only used for ABP, for OTAA, these values are generated on the Nwk Server, you should not have to change these values
uint8_t nwkSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appSKey[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint32_t devAddr =  (uint32_t)0x00000000;  

/*LoraWan channelsmask*/
uint16_t userChannelsMask[6]={ 0xFF00,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;  // we define this as a user flag in the .ini file. 

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_A;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*ADR enable*/
bool loraWanAdr = true;

// uint32_t license[4] = {};

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 1;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 8;

/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{
	/*appData size is LORAWAN_APP_DATA_MAX_SIZE which is defined in "commissioning.h".
	*appDataSize max value is LORAWAN_APP_DATA_MAX_SIZE.
	*if enabled AT, don't modify LORAWAN_APP_DATA_MAX_SIZE, it may cause system hanging or failure.
	*if disabled AT, LORAWAN_APP_DATA_MAX_SIZE can be modified, the max value is reference to lorawan region and SF.
	*for example, if use REGION_CN470, 
	*the max value for different DR can be found in MaxPayloadOfDatarateCN470 refer to DataratesCN470 and BandwidthsCN470 in "RegionCN470.h".
	*/
    // This data can be changed, just make sure to change the datasize as well. 
    appDataSize = 4;
    appData[0] = 0x01;   //the format will be lora id, parking spot id and parked?(1 or 0)
    appData[1] = 0x04;
    appData[2] = 0x00;
    appData[3] = 0x00;
}

RTC_DATA_ATTR bool firstrun = true;

void setup() {
	Serial.begin(115200);
	touch_pad_init();
    touch_pad_fsm_start();
    touch_pad_io_init(sensor1);
    touch_pad_config(sensor1);
  	Mcu.begin();
  	if(firstrun)
  	{
    	LoRaWAN.displayMcuInit();
    	firstrun = false;
  	}
	deviceState = DEVICE_STATE_INIT;
}

void loop()
{
	uint32_t base_touch = 30000;
    uint8_t touch = 0;
    uint32_t touch_value;
    uint32_t threshold = base_touch*2; //maybe later set to hard coded val or like 50-75% of og val 3x
	switch( deviceState )
	{
		case DEVICE_STATE_INIT:
		{
			#if(LORAWAN_DEVEUI_AUTO)
				LoRaWAN.generateDeveuiByChipID();
			#endif
				LoRaWAN.init(loraWanClass,loraWanRegion);
			break;
		}
		case DEVICE_STATE_JOIN:
		{
      		LoRaWAN.displayJoining();
			LoRaWAN.join();
			if (deviceState == DEVICE_STATE_SEND){
			 	LoRaWAN.displayJoined();
			}
			break;
		}
		case DEVICE_STATE_SEND:
		{
			touch_pad_read_raw_data(sensor1, &touch_value);
    		Serial.printf("%4d\n", touch_value);
    		if(touch_value>threshold)
    		{
        		Serial.println("The pin is being touched");
        		touch = 1;
    		}
    		else{
        		touch = 0;
    		}
      		LoRaWAN.displaySending();
			prepareTxFrame( appPort );
			if(touch == 1){ appData[2] = 0x01;}
			else{appData[2] = 0x00; }
			LoRaWAN.send();
			deviceState = DEVICE_STATE_CYCLE;
			break;
		}
		case DEVICE_STATE_CYCLE:
		{
			// Schedule next packet transmission
			txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
			LoRaWAN.cycle(txDutyCycleTime);
			deviceState = DEVICE_STATE_SLEEP;
			break;
		}
		case DEVICE_STATE_SLEEP:
		{
      		LoRaWAN.displayAck();
			LoRaWAN.sleep(loraWanClass);
			break;
		}
		default:
		{
			deviceState = DEVICE_STATE_INIT;
			break;
		}
	}
}