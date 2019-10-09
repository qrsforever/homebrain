#ifndef LARFE_CLIENT_H
#define LARFE_CLIENT_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h> 
#include <vector>
#include <pthread.h>
#include "LarfeDevice.h"

using namespace std;

class deviceCallBackHandler {
public:
		virtual ~deviceCallBackHandler() {} ;

		virtual void onDeviceStatusChanged(const std::string deviceId, const std::string deviceName, const std::string productKey,
                DeviceType dtype, const std::string room, const std::string status) = 0;
		virtual void onDevicePropertyChanged(const std::string deviceId, const std::string propertyKey, const std::string value) = 0;
};

class LarfeClient
{
public:
    LarfeClient();
    ~LarfeClient();

    int parseCmd(const unsigned char* cmd, const size_t size);
    /**
     * API to get host deviceId, can be called before init().
     *
     * @param[out] std::string deviceId: host deviceId.
     * @return 0:success -1:fail.
     */
    int getHostDeviceId(std::string& hostDeviceId);

    /**
     * API to Init, must be invoked after setCallback.
     *
     * @return 0:success -1:fail.
     */
    int init();
    int deinit();

    /**
     * API to get the list of devices
     *
     * @param[out] std::vector<std::string, DeviceType>& deviceList. key is device id, value is DeviceType.
     * @return 0:success -1:fail.
     */
    int getDeviceList(std::map<std::string, DeviceType>& deviceList);

    /**
     * API to set property of device
     *
     * @param[in] deviceId: device ID
     * @param[in] propertyKey: the property name
     * @param[in] value: the property value
     * @return 0:success -1:fail.
     */
    int setDeviceProperty(const std::string deviceId, const std::string propertyKey, const std::string value);

    /**
     * API to get property value of device
     *
     * @param[in] deviceId: device ID
     * @param[in] propertyKey: the property name
     * @param[out] value: the property value
     * @return 0:success -1:fail.
     */
    int getDeviceProperty(const std::string deviceId, const std::string propertyKey, std::string& value, VarType& type);

    /**
     * API to set callback function
     *
     * @param[in] callback
     * @return 0:success -1:fail.
     * notes: it must be called before init().
     */
    int setCallback(deviceCallBackHandler* callback);
    bool tcpSend(unsigned char* data, size_t size);

public:
    pthread_mutex_t m_mutex;
    struct sockaddr_in m_server;
    int m_sock;
    bool m_connected;
    bool m_stop;

private:
    char* httpGet(const char* strURL);
    int getObjList();
    void dumpObjList();
    int createDeviceList();
    int getDeviceInfoByObj(LarfeObj* obj, std::string& deviceId, DeviceType& deviceType, std::string& roomId);
    int generateCmd(LarfeObj* obj, unsigned char* &cmd, size_t &size);
    LarfeDevice* getDeviceById(std::string deviceId);

private:
    std::string m_mac;
    std::vector<LarfeObj*> m_objs;
    std::vector<LarfeRoom> m_rooms;
    std::map<std::string, LarfeDevice*> m_devices;
    std::map<DeviceType, std::string> m_productKeys;
    pthread_t m_threadId;
    std::string m_hostDid;
    deviceCallBackHandler* m_callback;
};

#endif
