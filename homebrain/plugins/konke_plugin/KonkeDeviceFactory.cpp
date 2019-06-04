#include <sstream>
#include "KonkeDevice.h"
#include "KonkeSensor.h"
#include "KonkeSOSAlarm.h"
#include "KonkeSwitch.h"
#include "KonkeDoorContact.h"
#include "KonkeCurtain.h"
#include "KonkeSmartplug.h"
#include "KonkeIlluminometer.h"
#include "KonkeDeviceFactory.h"

KonkeDeviceFactory::KonkeDeviceFactory()
{

}
KonkeDevice::Ptr KonkeDeviceFactory::getInstance(const char* type)
{
	KonkeDevice::Ptr pDevice = NULL;
	printf("KonkeDeviceFactory::getInstance 1\n");
	if(strcmp(type, KONKE_SWITCH_ID) == 0) {
		pDevice = std::make_shared<KonkeSwitch>();
	}else if(strcmp(type, KONKE_SOSPANEL_ID) == 0) {
		pDevice = std::make_shared<KonkeSOSAlarm>();
	}else if(strcmp(type, KONKE_SCENEPANEL_ID) == 0) {
		pDevice = std::make_shared<KonkeSwitch>();
	}else if(strcmp(type, KONKE_ILLUMINATION_ID) == 0) {
		pDevice = std::make_shared<KonkeIlluminometer>();
	}else if(strcmp(type, KONKE_DOORCONTACT_ID) == 0) {
		pDevice = std::make_shared<KonkeDoorContact>();
	}else if(strcmp(type, KONKE_CURTAIN_ID) == 0) {
		pDevice = std::make_shared<KonkeCurtain>();
	}else if(strcmp(type, KONKE_SMARTPLUG_ID) == 0) {
		pDevice = std::make_shared<KonkeSmartplug>();
	}else if(strcmp(type, KONKE_TEMP_SENSOR_ID) == 0) {
		pDevice = std::make_shared<KonkeSensor>();
	}else if(strcmp(type, KONKE_HUMIDITY_SENSOR_ID) == 0) {
		pDevice = std::make_shared<KonkeSensor>();
	}
	printf("KonkeDeviceFactory::getInstance end\n");
	return pDevice;
}
