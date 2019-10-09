#include <stdio.h>
#include "GatewayAgent.h"

#define ELINK_EMQ_HOST       "10.112.36.194"
#define ELINK_CLIENTID       "el.62be978faf876991c152dba094231cc0"
#define ELINK_DEVICE_SECRET  "ds.d638608eb4cd835d3fd5"
#define ELINK_PRODUCT_KEY    "pk.33ef1b64b64"

using namespace HB;

class IOTEventCallback:public RemoteEventCallback {
public:
    /* sub device */
     int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId) {
         printf("(%s %s %s)\n", deviceId.c_str(), deviceSecret.c_str(), elinkId.c_str());
         return -1;
     }
     int doTopoAddReply(std::string elinkId, int code, std::string message) {
         printf("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
         return -1;
     }
     int doTopoDelReply(std::string elinkId, int code, std::string message) {
         printf("(%s %d %s)\n", elinkId.c_str(), code, message.c_str());
         return -1;
     }

    /* device property */
     int doPropertySet(std::string rid, std::string elinkId, std::vector<std::pair<std::string, std::string>>&) {
         return -1;
     }
     int doPropertyGet(std::string rid, std::string elinkId, std::vector<std::string>&) {
         return -1;
     }

    /* ota */
     int doUpgradeReply(std::string newVersion, int code, std::string message) {
         return -1;
     }
     int doUpgradeCheck(std::string text) {
         return -1;
     }

    /* log */
     int doLogReport(std::string elinkId, std::string key, std::string logId) {
         return -1;
     }

    /* system cmd */
     int doSystemCmd(std::string cmd, std::string params) {
         return -1;
     }
};

int main(int argc, char *argv[])
{
    int ret = -1;
    // 1. 初始化连接Elink云的参数, 以及Elink控制Callback的HAL对象
    // 参数依次为: 处理Elink云下行接口的HAL对象, EMQ主机地址, MQTT的clientID, 用户名, 密码, 主机的Product Key
    ret = EIOT_Init(new IOTEventCallback(), ELINK_EMQ_HOST, ELINK_CLIENTID, ELINK_CLIENTID, ELINK_DEVICE_SECRET, ELINK_PRODUCT_KEY);
    printf("ret = %d\n", ret);
    // 2. 连接EMQ
    ret = EIOT_Connect();
    printf("ret = %d\n", ret);
    // 3. 注册子设备, 如果已经注册过, 无需再次注册
    ret = EIOT_RegisterSubdev("D0:9B:05:08:FF:41-15-03", "空调", "pk.2f0f787e2d5");
    printf("ret = %d\n", ret);
    EIOT_Yield(1000);
    // 4. 将子设备添加到Topo, 如果已经添加过, 无需再次添加
    ret = EIOT_TopoAdd("el.a46d206f028c5f1f39ef957aca787a15", "ds.b50ce45d9910994813fa", "pk.2f0f787e2d5");
    printf("ret = %d\n", ret);
    EIOT_Yield(1000);
    // 5. 每次调用EIOT_Yield, 都会读取EMQ发过来的消息, 如果有消息会回调到RemoteEventCallback实例的对应的接口上
    // 如果多次返回-1, 检查网络问题, 并再次EIOT_Connect();
    int count = 0;
    while (true) {
        ret = EIOT_Yield(300);
        if (ret < 0) {
            printf("mqtt error!\n");
            count++;
            sleep(1);
        }
        // if (count == 10) {
            // EIOT_Connect();
            // count = 0;
        // }
    }
    return 0;
}
