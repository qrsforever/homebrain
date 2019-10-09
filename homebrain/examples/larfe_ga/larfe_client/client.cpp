#include <iostream>
#include <signal.h>
#include "LarfeClient.h"
#include "LarfeClientLog.h"
#define BACK_HOME_ON 0xd0301000100017e
#define BACK_HOME_OFF 0xd0200000100017e

LarfeClient client;

void sig_exit(int s)
{
	client.deinit();
	exit(0);
}

class testCallback : public deviceCallBackHandler {
    public:
         ~testCallback() {
        } ;

        void onDeviceStatusChanged(const std::string deviceId, const std::string deviceName, const std::string productKey,
                DeviceType dtype, const std::string room, const std::string status) {
            cout << "onDeviceStatusChanged: deviceId:" << deviceId
                << ", deviceName:" << deviceName
                << ", productKey:" << productKey
                << ", room:" << room << ", status:" << status << std::endl;
        };
        void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, const std::string value) {
            cout << "onDevicePropertyChanged: deviceId:" << deviceId
                << ", propertyKey:" << propertyKey << ", value:" << value << std::endl;
        };
};

int main(int argc, char *argv[])
{
	if(argc != 1) {
		return 0;
	}
	signal(SIGINT, sig_exit);
	signal(SIGTERM, sig_exit);
    
    initLogThread();
    setLogLevel(LOG_LEVEL_TRACE);
    disableLogPool();

    testCallback* pCallback = new testCallback;
    std::string hostDid;

    client.getHostDeviceId(hostDid);
    printf("hostDid: %s\n", hostDid.c_str());

    client.setCallback(pCallback);
	client.init();

    std::map<std::string, DeviceType> devices;
    std::map<std::string, DeviceType>::iterator it;
    client.getDeviceList(devices);

    for (it = devices.begin(); it != devices.end(); it++) {
        if (it->second == DEVICE_TYPE_LIGHT_PORCH ||
                it->second == DEVICE_TYPE_LIGHT_CHANDELIER ||
                it->second == DEVICE_TYPE_LIGHT_BELT ||
                it->second == DEVICE_TYPE_LIGHT_SPOT) {
            client.setDeviceProperty(it->first, "LightSwitch", "1");
            sleep(2);
            client.setDeviceProperty(it->first, "LightSwitch", "0");
            sleep(2);
        } else if (it->second == DEVICE_TYPE_SCENE) {
            for (int i = 0; i < 5; i++) {
                char value[2];
                sprintf(value, "%d", i);
                client.setDeviceProperty(it->first, "SceneMode", value);
                sleep(2);
            }
        } else if (it->second == DEVICE_TYPE_AIRCONDITIONER) {
            client.setDeviceProperty(it->first, "PowerSwitch", "1");
            sleep(2);
            client.setDeviceProperty(it->first, "WorkMode", "1");
            sleep(2);
            client.setDeviceProperty(it->first, "WindSpeed", "2");
            sleep(2);
            client.setDeviceProperty(it->first, "TargetTemperature", "24");
            sleep(2);
        }
    }
	while(1)
	{
	}
	return 0;
}
