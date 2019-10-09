#include "LarfeClient.h"
#include "errno.h"
#include <map>
#include <iostream>
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "LarfeClientLog.h"

using namespace rapidjson;
//#define HOST_ADDRESS    "10.58.88.5"
#define HOST_ADDRESS    "127.0.0.1"
#define HOST_PORT       8888
#define PACKAGE_SIZE    8
#define DEVICE_ID_SIZE  24

#define URI_GET_ROOM_ID "/cgi-bin/eibcommand.cgi?tag=222"
#define URI_GET_OBJ_LIST "/cgi-bin/eibcommand.cgi?tag=2&oRoom="
#define URI_GET_MAC_ADDR "/cgi-bin/eibcommand.cgi?tag=m"
#define HTTP_CMD_GET_TO_CONSOLE "curl -s -H 'Accept: application/json' '%s'"

LarfeClient::LarfeClient()
    : m_sock(-1),
    m_stop(false),
    m_connected(false),
    m_mac(""),
    m_threadId(0),
    m_hostDid(""),
    m_callback(NULL)
{
    m_objs.clear();
    m_rooms.clear();
    m_devices.clear();
    //init the map for product key.
    m_productKeys[DEVICE_TYPE_LIGHT_PORCH] = light_porch_key;
    m_productKeys[DEVICE_TYPE_LIGHT_CHANDELIER] = light_chandelier_key;
    m_productKeys[DEVICE_TYPE_LIGHT_BELT] = light_belt_key;
    m_productKeys[DEVICE_TYPE_LIGHT_SPOT] = light_spot_key;
    m_productKeys[DEVICE_TYPE_DOOR_CONTACT] = door_contact_key;
    m_productKeys[DEVICE_TYPE_SCENE] = scene_key;
    m_productKeys[DEVICE_TYPE_AIRCONDITIONER] = airconditioner_key;
    // get info from server.
    getObjList();
}

LarfeClient::~LarfeClient()
{
}

char* LarfeClient::httpGet(const char* strURL)
{
    static const size_t nCmdSize = 1024;
    static const size_t nRspSize = 16384;
    FILE*   fp;
    char*   strCmd = NULL;
    char*   strRsp = NULL;
    size_t  nTotal = 0;

    if (NULL == strURL || '\0' == *strURL)
        return NULL;

    strCmd = (char*)calloc(nCmdSize, 1);
    strRsp = (char*)calloc(nRspSize, 1);

    if (strCmd && strRsp) {
        snprintf(strCmd, nCmdSize, HTTP_CMD_GET_TO_CONSOLE, strURL);
        while (1) {
            if (NULL != (fp = popen(strCmd, "r"))) {
                size_t nLeft = nRspSize - 1;
                size_t n;
                while (0 < (n = fread(&strRsp[nTotal], 1, nLeft, fp))) {
                    nTotal += n;
                    nLeft -= n;
                }
                pclose(fp);
                strRsp[nTotal] = '\0';
                LC_LOGA("curl result: %s\n", strRsp);
            }
            if (nTotal > 0)
                break;
            else
                sleep(5);
        }
    }
    else {
        LC_LOGE("larfe_http_get: Out of memory!");
    }

    free(strCmd);

    if (nTotal > 0) {
        return strRsp;
    }

    free(strRsp);
    return NULL;
}

int LarfeClient::getObjList() {
    const size_t strURLSize = 1024;
    const size_t macSize = 17, keySize = 6;
    char*   strURL = NULL;
    char* strResponse;
    char mac[macSize+1], key[keySize+1];
    size_t i = 0;

    strURL = (char*)calloc(strURLSize, 1);
    // parse mac addr.
    snprintf(strURL, strURLSize, "http://%s%s", HOST_ADDRESS, URI_GET_MAC_ADDR);
    strResponse = httpGet(strURL);

    if (strlen(strResponse) != macSize + keySize) {
        LC_LOGE("wrong response size for get mac addr request.");
        free(strResponse);
        return -1;
    }

    if (strResponse) {
        for (i = 0; i < strlen(strResponse); i++) {
            if (i < keySize)
                key[i] = strResponse[i];         
            else    
                mac[i-keySize] = strResponse[i];
        }
        free(strResponse);
    }

    key[keySize] = '\0';
    if (strcmp(key, "[MAC] ") != 0) {
        LC_LOGE("wrong response format for get mac addr request. expected: [MAC] xx");
        return -1;
    }

    mac[macSize] = '\0';
    m_mac.assign(mac);
    LC_LOGA("mac addr: %s\n", m_mac.c_str());
    // host deviceId
    char id[DEVICE_ID_SIZE];
    snprintf(id, DEVICE_ID_SIZE, "%s-%02d-%02d", m_mac.c_str(), DEVICE_TYPE_GATEWAY, 0);
    m_hostDid.assign(id);

    // parse larfe room info
    // Example response:
    //[{
    //"nID":"4","nName":"客餐厅","pID":"0","Icon":""},{
    //"nID":"8","nName":"安防","pID":"0","Icon":""},{
    //"nID":"11","nName":"主卧","pID":"0","Icon":""},{
    //"nID":"12","nName":"南次卧","pID":"0","Icon":""},{
    //"nID":"13","nName":"北次卧","pID":"0","Icon":""}]
    snprintf(strURL, strURLSize, "http://%s%s", HOST_ADDRESS, URI_GET_ROOM_ID);
    strResponse = httpGet(strURL);
    LarfeRoom room;

    if (strResponse) {
        Document doc;
        doc.Parse<0>(strResponse);
        if (doc.HasParseError()) {
            LC_LOGE("room info doc parse error!");
            return -1;
        }
        Value &roomList = doc;
        if (roomList.IsArray() && !roomList.Empty()) {
            for (SizeType i = 0; i < roomList.Size(); i++) {
                Value& roomInfo = roomList[i];
                if (roomInfo.IsObject()) {
                    if (roomInfo.HasMember("nID") && roomInfo.HasMember("nName")) {
                        room.m_id = roomInfo["nID"].GetString();
                        room.m_name = roomInfo["nName"].GetString();
                        m_rooms.push_back(room);
                    }
                }
            }
        }
        free(strResponse);
    }
    // add roomId 0: scenes
    room.m_id = "0";
    room.m_name = "场景";
    m_rooms.push_back(room);

    // parse larfe obj list
    // [{
    // "oID":"13","EIS":"1","val":"0","oName":"负载1","room":"4","type":"1","addrSet":"0/0/1","Icon":"","addrStatus":"0/1/1"},{
    // "oID":"14","EIS":"1","val":"0","oName":"负载2","room":"4","type":"1","addrSet":"0/0/2","Icon":"","addrStatus":"0/1/2"},{
    // "oID":"15","EIS":"1","val":"1","oName":"负载3","room":"4","type":"1","addrSet":"0/0/3","Icon":"","addrStatus":"0/1/3"}]
    std::vector<LarfeRoom>::iterator it;
    for (it = m_rooms.begin(); it != m_rooms.end(); it++) {
        snprintf(strURL, strURLSize, "http://%s%s%s", HOST_ADDRESS, URI_GET_OBJ_LIST, (it->m_id).c_str());
        strResponse = httpGet(strURL);
        if (strResponse) {
            const char s[2] = "/";
            char *token;
            char str[20];
            size_t str_size;

            Document doc;
            doc.Parse<0>(strResponse);
            if (doc.HasParseError()) {
                LC_LOGE("obj info doc parse error!");
                return -1;
            }
            Value &objList = doc;
            if (objList.IsArray() && !objList.Empty()) {
                for (SizeType i = 0; i < objList.Size(); i++) {
                    Value& objInfo = objList[i];
                    if (objInfo.IsObject()) {
                        if (objInfo.HasMember("oID") && objInfo.HasMember("type") &&
                                objInfo.HasMember("EIS") && objInfo.HasMember("room") &&
                                objInfo.HasMember("oName") && objInfo.HasMember("val") &&
                                objInfo.HasMember("addrSet") && objInfo.HasMember("addrStatus")) {
                            LarfeObj* obj = new LarfeObj;
                            obj->m_id = objInfo["oID"].GetString();
                            obj->m_type = (ObjectType)atoi(objInfo["type"].GetString());
                            obj->m_eis = (unsigned char)atoi(objInfo["EIS"].GetString());
                            obj->m_roomId = objInfo["room"].GetString();
                            obj->m_name = objInfo["oName"].GetString();
                            obj->m_value = objInfo["val"].GetString();
                            obj->m_addrSet = objInfo["addrSet"].GetString();
                            if (!obj->m_addrSet.empty()) {
                                str_size = obj->m_addrSet.size();
                                memcpy(str, obj->m_addrSet.c_str(), str_size);
                                str[str_size] = '\0';
                                token = strtok(str, s);
                                obj->m_addrValueSet[0] = atoi(token);
                                token = strtok(NULL, s);
                                obj->m_addrValueSet[1] = atoi(token);
                                token = strtok(NULL, s);
                                obj->m_addrValueSet[2] = atoi(token);
                            }
                            obj->m_addrStatus = objInfo["addrStatus"].GetString();
                            if (!obj->m_addrStatus.empty()) {
                                str_size = obj->m_addrStatus.size();
                                memcpy(str, obj->m_addrStatus.c_str(), str_size);
                                str[str_size] = '\0';
                                token = strtok(str, s);
                                obj->m_addrValueStatus[0] = atoi(token);
                                token = strtok(NULL, s);
                                obj->m_addrValueStatus[1] = atoi(token);
                                token = strtok(NULL, s);
                                obj->m_addrValueStatus[2] = atoi(token);
                            }
                            m_objs.push_back(obj);
                        }
                    }
                }
            }
            free(strResponse);
        }
    }

    free(strURL);
    // dump info
    dumpObjList();
    return 0;
}

void LarfeClient::dumpObjList()
{
    for(uint32_t i = 0; i < m_objs.size(); i++) {
        LC_LOGD("objId: %s, objName: %s, type: %d, EIS: %d, room: %s, value: %s, \
                addrSet: %s, addrStatus: %s, addrValueSet: %d, %d, %d, addrValueStatus: %d, %d, %d\n",
                m_objs[i]->m_id.c_str(), m_objs[i]->m_name.c_str(), m_objs[i]->m_type,
                (int)m_objs[i]->m_eis, m_objs[i]->m_roomId.c_str(), m_objs[i]->m_value.c_str(),
                m_objs[i]->m_addrSet.c_str(), m_objs[i]->m_addrStatus.c_str(),
                m_objs[i]->m_addrValueSet[0], m_objs[i]->m_addrValueSet[1], m_objs[i]->m_addrValueSet[2],
                m_objs[i]->m_addrValueStatus[0], m_objs[i]->m_addrValueStatus[1], m_objs[i]->m_addrValueStatus[2]);
    }
}

LarfeDevice* LarfeClient::getDeviceById(std::string deviceId)
{
    if (m_devices.find(deviceId) == m_devices.end())
        return NULL;

    return m_devices[deviceId];
}

int LarfeClient::createDeviceList()
{
    std::string deviceId, roomId;
    DeviceType deviceType;
    LarfeDevice* device;
    for (uint32_t i = 0; i < m_objs.size(); i++) {
        getDeviceInfoByObj(m_objs[i], deviceId, deviceType, roomId);
        if (m_devices.find(deviceId) == m_devices.end()) {
            device = new LarfeDevice(deviceId, deviceType, roomId);
            m_devices[deviceId] = device;
        } else {
            device = m_devices[deviceId];
        }
        device->m_objs.push_back(m_objs[i]);
    }

    std::map<std::string, LarfeDevice*>::iterator it;
    for (it = m_devices.begin(); it != m_devices.end(); it++) {
        it->second->updateAttributeList();
        //invoke callback to notify device online.
        std::string roomName;
        for (size_t i = 0; i < m_rooms.size(); i++) {
            if (m_rooms[i].m_id == it->second->m_roomId)
                roomName = m_rooms[i].m_name;
        }
        if (m_callback != NULL)
            m_callback->onDeviceStatusChanged(it->second->m_deviceId, it->second->m_deviceName,
                m_productKeys[it->second->m_deviceType], it->second->m_deviceType, roomName, "online");
    }

    return 0;
}

int LarfeClient::getDeviceInfoByObj(
        LarfeObj* obj,
        std::string& deviceId,
        DeviceType& deviceType,
        std::string& roomId)
{
    // deviceId : device type + index
    roomId = obj->m_roomId;
    int index = 0;
    if (obj->m_type == OBJ_AIRCONDITION) {
        deviceType = DEVICE_TYPE_AIRCONDITIONER;
    } else if (obj->m_type == OBJ_DOORCONTACT) {
        deviceType = DEVICE_TYPE_DOOR_CONTACT;
    } else if (obj->m_type == OBJ_SCENE) {
        deviceType = DEVICE_TYPE_SCENE;
    } else if (obj->m_type == OBJ_SWITCH) {
        // addrSet0 indicate detailed light type.
        deviceType = (DeviceType)(obj->m_addrValueStatus[0]);
    }

    if (obj->m_addrValueStatus[0] != 0)
        index = obj->m_addrValueStatus[2] / 10;
    else if (obj->m_addrValueSet[0] != 0)
        index = obj->m_addrValueSet[2] / 10;

    char id[DEVICE_ID_SIZE];
    snprintf(id, DEVICE_ID_SIZE, "%s-%02d-%02d", m_mac.c_str(), deviceType, index);
    deviceId.assign(id);

    return 0;
}

int LarfeClient::generateCmd(LarfeObj* obj, unsigned char* &cmd, size_t &size)
{
    cmd = (unsigned char*)calloc(PACKAGE_SIZE, 1);
    if (cmd == NULL) {
        LC_LOGE("generateCmd: falied to alloc cmd.");
        return -1;
    }
    cmd[0] = (unsigned char)OBJ_CONTROL_BYTE;
    cmd[1] = obj->m_eis;
    cmd[2] = (unsigned char)(obj->m_addrValueSet[0] << OBJ_ADDR0_SHIFT | obj->m_addrValueSet[1]);
    cmd[3] = obj->m_addrValueSet[2];
    cmd[4] = (atoi(obj->m_value.c_str()) >> 8) & 0xff;
    cmd[5] = atoi(obj->m_value.c_str()) & 0xff;
    cmd[6] = (cmd[1] + cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
    cmd[7] = (unsigned char)OBJ_END_BYTE;
    size = PACKAGE_SIZE;
    return 0;
}

int LarfeClient::parseCmd(const unsigned char* cmd, const size_t size)
{
    if (size != PACKAGE_SIZE) {
        LC_LOGE("parseCmd: package size invalid.");
        return -1;
    }

    unsigned char checksum = (cmd[1] +cmd[2] + cmd[3] + cmd[4] + cmd[5]) & 0xff;
    if (checksum != cmd[6] || cmd[7] != OBJ_END_BYTE) {
        LC_LOGE("parseCmd: checksum or ending byte invalid.");
        return -1;
    }

    if (cmd[0] != OBJ_STATUS_BYTE) {
        LC_LOGE("parseCmd: starting byte invalid.");
        return -1;
    }

    int addrValueStatus[3];
    char value[5];
    //unsigned char objEIS = cmd[1];
    std::string deviceId, roomId;
    DeviceType deviceType;
    addrValueStatus[0] = cmd[2] >> OBJ_ADDR0_SHIFT;
    addrValueStatus[1] = cmd[2] & OBJ_ADDR1_MASK;
    addrValueStatus[2] = cmd[3];
    int objValue = cmd[4] << 8 | cmd[5];
    sprintf(value, "%d", objValue);
    for (size_t i = 0; i < m_objs.size(); i++) {
        if (m_objs[i]->m_addrValueStatus[0] == addrValueStatus[0] &&
                m_objs[i]->m_addrValueStatus[1] == addrValueStatus[1] &&
                m_objs[i]->m_addrValueStatus[2] == addrValueStatus[2]) {
            /*if (m_objs[i]->m_eis != objEIS) {
                WARNING_PRINT("parseCmd: obj EIS doesn't match. actual:"
                        << (int)objEIS<<", expected:"<<(int)m_objs[i]->m_eis);
            }*/
            getDeviceInfoByObj(m_objs[i], deviceId, deviceType, roomId);
            LarfeDevice* device = getDeviceById(deviceId);
            //update both value in obj and attr.
            m_objs[i]->m_value.assign(value);
            std::string propertyName = device->getAttributeName(m_objs[i]);
            device->setAttributeValue(propertyName, m_objs[i]->m_value);
            if (m_callback != NULL)
                m_callback->onDevicePropertyChanged(deviceId, propertyName, m_objs[i]->m_value);
            break;
        }
    }
    return 0;
}

bool LarfeClient::tcpSend(unsigned char* data, size_t size)
{
    if(m_sock != -1 && m_connected) {
		if( send(m_sock , data, size , 0) < 0) {
			LC_LOGE("failed. data: 0x%llx, errno: %d\n", *(long long int*)data, strerror(errno));
			return false;
		}
	} else {
        LC_LOGE("not connected to tcp server.\n");
		return false;
    }

    LC_LOGA("0x%llx\n", *(long long int*)data);
	return true;
}

void* tcpReceiveThread(void* param)
{
    LarfeClient* client = (LarfeClient*) param;
  	unsigned char buffer[PACKAGE_SIZE];
    int retry = 0;

    // establish tcp connection with server.
    if(client->m_sock == -1) {
        client->m_sock = socket(AF_INET , SOCK_STREAM , 0);
        if (client->m_sock == -1) {
            LC_LOGE("Could not create socket");
            return NULL;
        }
    }

    if((signed)inet_addr(HOST_ADDRESS) == -1) {
        struct hostent *he;
        struct in_addr **addr_list;
        if ( (he = gethostbyname( HOST_ADDRESS ) ) == NULL) {
            herror("gethostbyname");
            LC_LOGE("Failed to resolve hostname\n");
            return NULL;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
        for(uint32_t i = 0; addr_list[i] != NULL; i++) {
            client->m_server.sin_addr = *addr_list[i];
            break;
        }
    } else {
        client->m_server.sin_addr.s_addr = inet_addr( HOST_ADDRESS );
    }
    client->m_server.sin_family = AF_INET;
    client->m_server.sin_port = htons( HOST_PORT );

    //dead wait for server ready.
    while (connect(client->m_sock , (struct sockaddr *)&client->m_server , sizeof(client->m_server)) != 0) {
        LC_LOGD("Waiting for server. retry %d\n", ++retry);
        sleep(1);
    }

    client->m_connected = true;
    //pthread_mutex_lock(&client->m_mutex);
    while(!client->m_stop) {
        int numOfBytesReceived = recv(client->m_sock, buffer, PACKAGE_SIZE, 0);
        if(numOfBytesReceived < 1) {
            client->m_stop = true;
            LC_LOGE("failed. errno: %s\n", strerror(errno));
            client->deinit();
            return NULL;
        }
        LC_LOGA("0x%llx\n", *(long long int*)buffer);
        client->parseCmd(buffer, PACKAGE_SIZE);
    }
    return NULL;
}

int LarfeClient::getHostDeviceId(std::string& hostDeviceId)
{
   hostDeviceId = m_hostDid;
   return 0;
}

int LarfeClient::init()
{
    // create device list from obj list
    createDeviceList();

    // start receive thread for receiving msg from server
    pthread_attr_t attr;
    pthread_mutex_init(&m_mutex, 0);

    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&m_threadId, &attr, &tcpReceiveThread, this);
    return 0;
}

int LarfeClient::deinit()
{
    for (uint32_t i = 0; i < m_objs.size(); i++) {
        delete m_objs[i];
    }
    m_objs.clear();

    m_stop = true;
    if (m_threadId) {
        pthread_join(m_threadId, 0);
    }

    if (close(m_sock) == -1) {
        // close failed
        return -1;
    }
    return 0;
}

int LarfeClient::getDeviceList(std::map<std::string, DeviceType>& deviceList)
{
    std::map<std::string, LarfeDevice*>::iterator it;
    for (it = m_devices.begin(); it != m_devices.end(); it++) {
        deviceList[it->first] = it->second->m_deviceType;
    }
    return 0;
}

int LarfeClient::setDeviceProperty(
        const std::string deviceId,
        const std::string propertyKey,
        const std::string value)
{
    LarfeDevice* device = getDeviceById(deviceId);    
    unsigned char* cmd = NULL;
    size_t cmd_size = 0;

    LarfeObj* obj = device->getAttributeObj(propertyKey);
    if (obj == NULL) {
        LC_LOGE("failed to get obj for property: %s\n", propertyKey.c_str());
        return -1;
    }

    obj->m_value = value;
    generateCmd(obj, cmd, cmd_size);

    if (cmd != NULL && cmd_size != 0) {
        tcpSend(cmd, cmd_size);
        free(cmd);
    }
    LC_LOGA("deviceId: %s, propertyKey: %s, value %s\n", deviceId.c_str(), propertyKey.c_str(), value.c_str());
    return 0;
}

int LarfeClient::getDeviceProperty(
        const std::string deviceId,
        const std::string propertyKey,
        std::string& value, VarType& type)
{
    LarfeDevice* device = getDeviceById(deviceId);    
    int ret = device->getAttributeValue(propertyKey, value, type);
    LC_LOGA("deviceId: %s, propertyKey: %s, value %s\n", deviceId.c_str(), propertyKey.c_str(), value.c_str());
    return ret;
}

int LarfeClient::setCallback(deviceCallBackHandler* callback)
{
    m_callback = callback;
    return 0;
}
