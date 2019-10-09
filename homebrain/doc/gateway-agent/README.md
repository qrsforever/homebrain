---

title: HB网关代理接口使用说明

date: 2019-05-24 10:42:26
tags: [mqtt, api, elink]
categories: [local]

---

# doxygen docs

    doxygen Doxyfile


# 接口定义

   src/gateway-agent/src/GatewayAgent.h

## PUB接口

### 注册子网关

    // -----------------------------
    /// @brief registerSubdev :
    ///
    /// @param deviceId: HB设备唯一标示,HB内部
    /// @param deviceName: HB设备名字
    /// @param productKey: HB设备产品Key
    ///
    /// @returns 0: ok -1: error
    // -----------------------------
    int registerSubdev(std::string deviceId, std::string deviceName, std::string productKey);

调用实例:

    GAgent().registerSubdev("deviceId1", "deviceName1", "productKey1");

### 网关Topo添加

    // -----------------------------
    /// @brief topoAdd :
    ///
    /// @param elinkId: 云端返回的子设备云端统一标示
    /// @param deviceSecret: 子设备秘钥
    /// @param productKey: 子设备产品Key
    ///
    /// @returns 0: ok -1: error
    // -----------------------------
    int topoAdd(std::string elinkId, std::string deviceSecret, std::string productKey);

调用实例:

    GAgent().topoAdd("elinkId1", "deviceScret1", "productKey1");


### 网关Topo删除

    // -----------------------------
    /// @brief topoAdd :
    ///
    /// @param elinkId: 云端返回的子设备云端统一标示
    /// @param deviceSecret: 子设备秘钥
    /// @param productKey: 子设备产品Key
    ///
    /// @returns 0: ok -1: error
    // -----------------------------
    int topoDel(std::string elinkId, std::string deviceSecret, std::string productKey);

调用实例:

    GAgent().topoDel("elinkId1", "deviceScret1", "productKey1");


### 属性设置结果返回

    // -----------------------------
    /// @brief propertySetResult :
    ///
    /// @param elinkId:
    /// @param code: 结果返回码, 200是OK
    /// @param message: 返回结果信息
    ///
    /// @returns 0: ok -1: error
    // -----------------------------
    int propertySetResult(std::string elinkId, int code, std::string message);

调用实例:

    GAgent().propertySetResult("elinkId1", 200, "Set Property Ok");


### 属性获取结果返回


    // -----------------------------
    /// @brief propertyGetResult :
    ///
    /// @param elinkId:
    /// @param code:
    /// @param std::vector: 设备属性DeviceProperty数组
    ///
    /// @returns
    // -----------------------------
    int propertyGetResult(std::string elinkId, int code, std::vector<struct DeviceProperty>&);

其中[DeviceProperty](#m1)设置设备属性信息, 设备属性:name, value, type信息

调用实例:

    std::vector<struct DeviceProperty> props;
    struct DeviceProperty prop1("Power", "1", "int");
    struct DeviceProperty prop2("Level", "1", "string");
    props.push_back(prop1);
    props.push_back(prop2);
    GAgent().propertyGetResult("elinkId1", 200, props);

### 事件上报

    // -----------------------------
    /// @brief eventReport :
    ///
    /// @param events: 事件类型接口构体数组
    ///
    /// @returns
    // -----------------------------
    int eventReport(std::vector<struct EventInfo> &events);

其中[EventInfo](#m2)事件属性信息, event, type, message等信息

调用实例:

    std::vector<struct EventInfo> events;
    struct EventInfo event1("SmokeCheckAlarm", "alarm", "fire your home");
    struct EventInfo event2("ImmersionCheckAlarm", "alarm", "water your home");
    events.push_back(event1);
    events.push_back(event2);
    GAgent().eventReport(events);

## SUB接口

采用回调接口类实现, 虚接口参考[SubElinkCallback](#m3)定义, 继承实现接口.

Demo参考`GatewayAgentHandler.cpp`

接口介绍略


## 其他

### 接口单元测试方法

1. PUB主动发消息,将`GatewayAgentHandler.h`中开关放开`#define UNITTEST 1`

HB启动后, 检测Network正常后(这块暂没有开发, 默认是OK的), 发网络成功消息, GatewayAgentHandler
接收到此消息后,首先启动GatewayAgent初始化, 初始化后应该设置SUB的回调接口, 方可接受消息.
如果初始化失败(服务器连不上),默认3s会尝试继续链接, 连接成功后会逐个测试PUB的每个接口,
时间间隔为1s.

2. SUB被动消息, 采用设置SUB回调接口 + Python脚本发消息`unittest_pub.py`

    '1': 'test_registerReply',
    '2': 'test_addReply',
    '3': 'test_delReply',
    '4': 'test_set',
    '5': 'test_get',

执行`./unittest_pub num`, num是上面对应的test api的序列数字


### DeviceProperty定义

<span id="m1">定义:</span>

    struct DeviceProperty {
        DeviceProperty(): name(""), value(""), type("") {}
        DeviceProperty(std::string n, std::string v, std::string t)
            : name(n), value(v), type(t) {}
        std::string name;
        std::string value;
        std::string type; // int, string, float
    };


### EventInfo定义

<span id="m2">定义:</span>

    struct EventInfo {
        EventInfo(): event(""), type(""), message(""), value("") {}
        EventInfo(std::string e, std::string t, std::string m, std::string v="")
            : event(e), type(t), message(m), value(v) {}
        std::string event;
        std::string type;
        std::string message;
        std::string value;
    };


### SubElinkCallback定义

<span id="m3"></span>

    class SubElinkCallback {
    public:
        virtual ~SubElinkCallback() {} ;

        virtual int doRegisterReply(std::string deviceId, std::string deviceSecret, std::string elinkId) = 0;
        virtual int doTopoAddReply(std::string elinkId, int code, std::string message) = 0;
        virtual int doTopoDelReply(std::string elinkId, int code, std::string message) = 0;
        virtual int doPropertySet(std::string elinkId, std::vector<struct DeviceProperty>&) = 0;
        virtual int doPropertyGet(std::string elinkId, std::vector<std::string>&) = 0;

    };

