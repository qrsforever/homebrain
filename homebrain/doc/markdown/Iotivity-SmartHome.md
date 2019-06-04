---

title: Iotivity之SmartHome
date: 2018-05-16 14:09:38
tags: [ IOT, DrawIt ]
categories: [ Note ]

---

Discovery Payload
=================

```
device ---
          \
           v  /oic/res
 +------------------------------------------+                          +--------+     +--------------------+
 |           OCDiscoveryPayload             |                     +--> |oic.wk.d| --> |oic.d.airconditioner|
 |------------------------------------------|                     |    +--------+     +--------------------+
 |sid : f9f0a646-fd3d-35c7-d4a7-68e3afe9f64e|                     |
 |name: null | type: null | iface: null     |                     |
 |                resources                 |(multiple devices    | types
 |------------------------------------------|  not support)       |
 |                next                      | --> null            |     ifaces   +---------------+    +--------+
 +------------------------------------------+                     |     +----->  |oic.if.baseline|--->|oic.if.r|
        ◇                                                         |     |        +---------------+    +--------+
        |  resources                                              |     |
        |                                                         |     |
        v                                                         ◇     ◇
 +---------------------+       +---------------------+      +---------------------+      +---------------------+
 |  OCResourcePayload  |       |  OCResourcePayload  |      |  OCResourcePayload  |      |  OCResourcePayload  |
 |---------------------|       |---------------------|      |---------------------|      |---------------------|
 | uri: /oic/sec/doxm  |       | uri: /oic/sec/pstat |      |    uri: /oic/d      |      |     uri: /oic/p     |
 |   types/ifaces      |       |    types/ifaces     |      |    types/ifaces     |      |    types/ifaces     |
 |---------------------|       |---------------------|      |---------------------|      |---------------------|
 |      next           |  -->  |      next           |  --> |      next           | -->  |      next           |
 +---------------------+       +---------------------+      +---------------------+      +---------------------+
                                                                                                  |
                                                                                                  |
                                                                                                  v
                               +---------------------+      +---------------------+      +---------------------+
                               |  OCResourcePayload  |      |  OCResourcePayload  |      |  OCResourcePayload  |
                               |---------------------|      |---------------------|      |---------------------|
                               |  uri:/temperature   |      | uri: /binaryswitch  |      |uri:/oic/introspectio|
                               |   types/ifaces      |      |    types/ifaces     |      |     types/ifaces    |
                               |---------------------|      |---------------------|      |---------------------|
                               |      next           |  <-- |      next           |  <-- |      next           |
                               +---------------------+      +---------------------+      +---------------------+
                                    ◇      ◇                        ◇     ◇
                                    |      |                        |     |
        +--------------------+      |      |                        |     |      +--------------------+
        | oic.r.temperature  | <----+      |                        |     +----> |oic.r.switch.binary |
        +--------------------+             |                        |            +--------------------+
                                           |                        |
                  +----------------+       |                        |     +----------------+
                  |oic.if.baseline |  <----+                        +---> |oic.if.baseline |
                  +----------------+                                      +----------------+


1. foreach(uri:device) ---> "/oic/d" ---> airconditioner

2. foreach(uri:device) ---> "/binaryswitch" & "/temperature" --> BinarySwitch & Temperature


                         +----------------+               +-------------------+                +-----------------+
                         |  OCRepPayload  | ------------> | PayloadConverter  | ------------>  | PropertyBundle  |
                         |                | <------------ |                   | <------------  |                 |
                         +----------------+               +-------------------+                +-----------------+

```

Discovery UML
=============

```
                                                                    (app)               ((6))
                                                                    +------------------------+
                                                                    |  MyDiscoveryDelegate   |
                                                                    +------------------------+
                                                                                |
                                                                                |
                                                                    (abtract)   ▽
                                                                    +-------------------------+          +---------------------+
                                                                    | DeviceDiscoveryDelegate |<--+      | OCDiscoveryPayload  |
 +---------------+              +-----------------------+           |-------------------------|   |      |---------------------|
 |   ns:SH_Impl  |              |    DiscoveryQuery     |           |   onFindRemoteDevice    |   |      |       base          |
 |---------------|              |-----------------------|           +-------------------------+   |      |                     |
 |   g_config    |              |    m_deviceType       |                       △                 |      | sid/name/type/iface |
 |---------------|              |    m_hostAddress      |\                      |                 |      |     resources       |
 |    start()    |              |-----------------------| \                     |                 |      |---------------------|
 |    stop()     |              |  [get/set]DeviceType  |  \                    | m_delegate      |      |       next          |
 |    process()  |              +-----------------------+   \          +-------------------+      |      +---------------------+
 +---------------+                                           \         |  DeviceDiscovery  |      |            ◇
                                                              -----+   |-------------------|      |            | resources
     static                                          queryParam    |   |    m_delegate     |      |            |
     +----------------------                                       |   |-------------------|      |            v
     |ClientCallbackWrapper:               OCDoRequest("/oic/res") v   |   setDelegate     |      |    +------------------------+
     |  \                                 /----------------------------- findRemoteDevices |((1)) |    |  OCResourcePayload     |
     |---\ DeviceDiscoveryCallback:      /                             +-------------------+      |    |------------------------|
     |      |                           v                                     single instance     |    | uri/types/Interfaces   |
     |      |          findRemoteDevices[withQuery] <----+                                        |    | anchor/port/secure/rel |
     |      |          ((2))         \                   |   +----------------+                   |    |------------------------|
     |  \                             \                  |   | OCCallbackData | OCStack           |    |        next            |
     |---\ RemoteResourceCallback      ------------+     |   |----------------|                   |    +------------------------+
     |      |                                      |     |   |    context     |                   |
     |      |          on[Get/Set/Observe] <-------+-----+------- cb          |                   |
     |      |          [[6]]            \          |         |    cd          |                   |
     |  \                                \         |         +----------------+                   |callback from ocstack:
     |---\ +--------------------------+   |        |                 ◇                            |
     |     |     CallbackHelper       |   |        |                 | context                    |  1. discovery for all resoures
     |     |--------------------------|   |        |                 |                            |  2. resources for get/set/post
     |     |[set/destroy]RemoteDevice |   |        |                 v                            |
     |     +--------------------------+   |        |      +-----------------------+               |
     |                                    |        |      | ClientCallbackContext |               |
     \                                    |        |      |-----------------------|    delegate   |   delegate
      \                                   |        |      |       delegate  ----------------------+--------------+
                                          |        |      |        query          |   real callback              |
                                          |        |      |-----------------------|after doing some work         |
                                          |        |      |      getDelegate      |                              |
                                          |        |      |      getQuery         |                              |
                                          |        |      +-----------------------+                              v
                                          |        |   talk with ocstack using callback          +------------------------------+
            onGet <--- context.delegate   |        |                                             | SHBaseRemoteResourceDelegate |
                                          |        |                                             |------------------------------|
            onSet <--- context.delegate   |       l| ((3))                                 [[7]] |      on[Get/Set/Observe]     |
                                          |       o|    SHBaseRemoteDeviceBuilder                +------------------------------+
        onObserve <--- context.delegate   |       o|               |
                                          |       p|               |
                                          |       ||--> createSHBaseRemoteDevice
                                          |       ||             uri : "/oic/d"
                                          |       ||             type: "oic.d.airconditioner"
                                                  ||
                                                  ||    CallbackHelper
                                                  ||           |
                                                  ||           |
                                                  ||--> setRemoteDevice
                                                  ||       |                         ((4))
                                                  ||       |    SHBaseRemoteResourceBuilder
                                                  ||       |                 |
                                                  ||       |                 |
                                                  ||       |-->  createSHBaseRemoteResource
                                                  ||       |         uri : "/binaryswitch"       |  uri : "/temperature"
                                                  ||       |         type: "oic.r.switch.binary" |  type: "oic.r.temperature"
                                                  ||       |
                                                  ||
                                                  || ((5))
                                                  ||--> delegate->onFindRemoteDevice(remoteDevice)
                                                  ||

```

RemoteResource UML
==================

```


                             (base: only for on[Get/Set/Observe)
                              +------------------------------+
                   +--------> | SHBaseRemoteResourceDelegate |
   [[5]]           |          |------------------------------| ◁ ----------------------------------------------+
          +-> get  |          |      on[Get/Set/Observe]     |                                                 |
doRequest |        |          +------------------------------+                                                 |
+---------|        |          [[7]]                   △                                                        |
| OCStack |        |                                  |                                                        |
|         +-> post | m_delegate                       |                                                        |
|                  |   (ocstack cb proxy)             |               +---------------------------+            |
|                  ◇                                  |               |   SHBaseRemoteResource    |            |
|  +-------------------------------+       m_Impl     |               |---------------------------|            |
|  |   SHBaseRemoteResource_Impl   |  <---------------+-------------◇ |          m_Impl           |            |
|  |-------------------------------|                  |               |---------------------------|            |
|  |   uri/types/ifaces/devAdrr    |                  |               |       setDelegate         |            |
|  |  m_observeHandle/m_endpoints  |                  |               |  [set/get]PropertyBundle  | [[4]]      |
|  |       m_delegate              |                  |               |    [start/stop]Observe    |            |
|  |-------------------------------|         ---------+-------------> |       getter* setter*     |            |
|  |        hasResourceType        |        /         |               +---------------------------+            |
|  |        startObserve           |       /          |                         △      △                       |
|  |        setDelegate            |      /           |                         |      |                       |
+------[set/get]PropertyBundle     |     /            |         +---------------+      +-------------+         |
   |        getter*                |    /             |         |                                    |         |
   +-------------------------------+   /              |         |                                    |         |
                                      /    +---------------------------------------+       +--------------------------------------+
                m_resources 0:n      /     |       RemoteBinarySwitchResource      |       |      RemoteTemperatureResource       |
                +-------------------/      |---------------------------------------|       |--------------------------------------|
                |                          |            m_delegate (app api proxy) |       |               m_delegate             |
                ◇                          |---------------------------------------|       |--------------------------------------|
   +------------------------------+        | setRemoteBinarySwitchResourceDelegate | [[2]] | setRemoteTemperatureResourceDelegate |
   |    SHBaseRemoteDevice_Impl   |        |               on/off   [[3]]          |       |         [set/get]Temperature         |
   |------------------------------|        |              getState                 |       |        setTemperatureWithUnits       |
   |      m_resources             |        | [[8]]   on[Get/Set/Observe]           |       |          on[Get/Set/Observe]         |
   |      m_deviceTypes           |        +---------------------------------------+       +--------------------------------------+
   |      m_deviceId              |                          ◇                ^                 ^              ◇
   |------------------------------|                          |  m_delegate    |                 |  m_delegate  |
   |  [set/get][DeviceId/Types]   |                          |                |                 |              |
   |       hasDeviceType          |                          |                +------+    +-----+              |
   |  getResourceWithResourceType |         (abstract)       v                       |    |                    v        (abstract)
   |  getResourceWithResourceUri  |         +------------------------------------+   |    |   +-----------------------------------+
   |       getAllResources        |         | RemoteBinarySwitchResourceDelegate |   |    |   | RemoteTemperatureResourceDelegate |
   |   addResource (check type)   |         |------------------------------------|   |    |   |-----------------------------------|
   +------------------------------+         | [[9]]   onTurn[On/Off]             |   |    |   |      on[Get/Set]Temperature       |
                 ^                          |         onGetState                 |   |    |   +-----------------------------------+
                 |                          +------------------------------------+   |    |                     △
                 |                                           △                       |    |                     |
                 |  m_Impl                                   |                       |    |                     |
                 ◇                                           |                       |    |                     |
   +------------------------------+        (app)             |               [[10]]  |    |                     |            (app)
   |       SHBaseRemoteDevice     |       +---------------------------------------+  |    |   +-----------------------------------+
   |------------------------------|       |                                       |  |    |   |                                   |
   |         m_Impl               |       | MyRemoteBinarySwitchResourceDelegate  |  |    |   |MyRemoteTemperatureResourceDelegate|
   |------------------------------|       |                                       |  |    |   |                                   |
   |  getDevice[Id/Types]         |       +---------------------------------------+  |    |   +-----------------------------------+
   |  getResourceWithResourceType |                                                  |    |
   |  getResourceWithResourceUri  |                                                  |    |
   +------------------------------+                                                  |    |
                 △                                                                   |    |
                 |                                                                   |    |
                 |                                                                   |    | on()-->getPropertyBundle--> DoRequest
   [[1]]         |                                                                   |    |
   +------------------------------+                                                  |    |off()-->setPropertyBundle--> DoRequest
   |  RemoteAirConditionerDevice  |                                                  |    |
   |------------------------------|   m_remoteBinarySwitch                           |    |
   |     m_remoteBinarySwitch     |◇ ------------------------------------------------+    |
   |     m_remoteTemperature      |◇ -----------------------------------------------------+
   +------------------------------+   m_remoteTemperature

```
