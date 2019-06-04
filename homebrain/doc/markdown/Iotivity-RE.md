---

title: Iotivity之Resource-Encapsulation
date: 2018-05-08 12:58:38
tags: [ IOT, DrawIt ]
categories: [ Note ]

---


Server端类图
============

```
                                                    +----------------------+
                                                    |   RCSRepresentation  |
                                                    |----------------------|
                                                    |  uri/types/ifaces    |
                        +-------------------------◇ |   m_attributes       |
                        |                           |   m_children         |
                        |                           |----------------------|
                        |                           |                      |
                        |                           | fromOCRepresentation | ------------------------\
                        |                           | toOCRepresentation   |                          \
                        |                           |   Getter/Setter      |                           \
                        |                           +----------------------+                            \
                        |                                                                                \
                        |                                                                                 \
                        |                                                                                  \
                        |                                                                                   \
                        |                                                                                    \
                        |              +---------------------------+ +--------------------------+             \
                        v              | ResourceAttributesBuilder | |  OCRepresentationBuilder |              \
          +----------------------+     |---------------------------| |--------------------------|     +--------------------+
          | RCSResourceAttributes|<----|      m_target             | |      m_target            |---->|OC::OCRepresentation|
          +----------------------+     |---------------------------| |--------------------------|     +--------------------+
          |      m_values        |     |       extract()           | |       extract()          |               ^
          +----------------------+     +---------------------------+ +--------------------------+               |
          |      visit()         |                     \                       /                                |
          +----------------------+                      \                     /                                 |m_ocRep
                        ^                                \                   /                                  ◇
                        |                           +-------------------------------+                +--------------------+
                        |                           |  ResourceAttributesConverter  |                |   RequestHandle    |
                        |                           |-------------------------------|                |--------------------|
                        |                           |      fromOCRepresentation     |                |    m_errorCode     | false
                        |                           |      toOCRepresentation       |            --> |    m_customRep     |----+
                        |                           +-------------------------------+           /    |    m_ocRep         |    |
                        |                                                                      /     |--------------------|    |
                        |                  +-----------------+                                /      |  getRepresentation |    |
                        |                  |  RCSGetResponse |                               /       +--------------------+    |
                        |                  |-----------------|  m_handler                    |                  △              |
                        |                  |    m_handler    |◇ -----------------------------+                  |              |
                        |                  |-----------------|                                                  |              |
                        |                  |  defaultAction  |----+     +-----------------+         +-----------------------+  |
                        |         +------- |    create       |    |     |  RCSSetResponse |         |   SetRequestHandler   |  |
                        |         |        +-----------------+    |     |-----------------|         |-----------------------|  |
                        |         |                               |     |    m_handler    |◇ ------>|                       |  |
                        |         v                               |     |-----------------|         | applyAcceptanceMethod |  |
                        |   create(attrs) ---> m_customRep = true |     |  defaultAction  |-----+   +-----------------------+  |
                        |                                         |     |     create      |     |                              |
                        |                                         |     +-----------------+     |                              |
                        +---------------\                         |                             |                              |
                                         \                        +-----------------------------+------------------------------+
                                          \                                                    +------------------+            |
                                           \                                                   |  RCSRequest      |            |
     +------------------------+             \          +----------------------------+          |------------------|            |
     |       Builder          |              \         |    RCSResourceObject       | <------◇ | m_resourceObject |            |
     |------------------------|               \        |----------------------------|          | m_ocRequest      |            |
     |  m_uri                 |                \       |                            |          +------------------+            |
     |  m_types               |                 \      |    m_resourceHandle        |                  ◇                       |
     |  m_interfaces          |                  ----◇ |    m_resourceAttributes    |                  | m_ocRequest           |
     |  m_defaultInterface    |    OC_SECURE           |    m_interfaceHandlers     |                  v                       |
     |  m_properties          |--> OC_OBSERVABLE       |    m_getRequestHandler     |       +---------------------------+      |
     |  m_resourceAttributes  |    OC_DISCOVERABLE     |    m_setRequestHandler     |       |   OC::OCResourceRequest   |      |
     |------------------------|                        |----------------------------|       |---------------------------|      |
     |  build()               |                        | set[Get/Set]RequestHandler |       |  messageID,representation |      |
     +------------------------+                        |     [get/set]Attribute     |       |  devAddr, query, options  |      |
         |                                  server     |         notify             |       |  payload                  |      |
         +-> server = RCSResourceObject()----------->  |      sendResponse          |       |  resourceHandle           |      |
         |                                             |      entityHandler (*) -------+    |  requestHandle            |      |
         +-> registerResource()                        |      handleRequest         |  |    |---------------------------|      |
         |                                             |      handleObserve         |  +--------getRequestHandlerFlag-------+  |
         +-> bindInterfaceToResource()                 +----------------------------+       |   getRequestHandle        |   |  |
         |                                                                                  +---------------------------+   |  |
         +-> bindTypeToResource()                                                                                           |  |
         |                                                                           +--------------------------------+-----+  |
         +-> server->init()                                                          | RequestFlag       ObserverFlag |        |
                                                                                     |                                |        |
                                                                                     v                                v        |
                    1:n     +--------------------------+                        handleRequest                   handleObserve  |
  g_defaultHandlers ------->|    InterfaceHandler      |                             |                                         |
          |                 |--------------------------|                  "get"      |      "post"                             |
          |                 |       m_getBuilder       |                  -----------+-------------                            |
          |                 |       m_setBuilder       |                  |                       |                            |
          |                 |--------------------------|                  v                       v                            |
          |                 |  getGetResponseBuilder   |           handleRequestGet        handleRequestSet                    |
          | defaultAction   |  getSetResponseBuilder   |                  |                       |                            |
          |                 +--------------------------+                  v                       v                            |
          | m_customRep=false                                     m_getRequestHandler     m_setRequestHandler                  |
+---------+----------------------------------------------------\          |                       |                            |
|   "oic.if.baseline"   "oic.if.a"   "oic.if.s"     "oic.if.b"  \         v                       v                            |
|          |                |            |              |        \   RCSGetResponse          RCSSetResponse                    |
|          v                |            v              |         \       |                       |                            |
|buildGetBaselineResponse   |  buildGetRequestResponse  |          \      +----------+------------+                            |
|buildSetBaselineResponse   |  NullPtr                  |           \                |                                         |
|                           v                           v            \               v                                         |
|        (attrs) buildGetRequestResponse      buildGetBatchResponse   \         sendResponse                                   |
|                buildSetRequestResponse      buildSetBaselineResponse \                                                       |
+------------------------------------------------------------------------------------------------------------------------------+

```

Client端类图
============

```
                                                                                "Caching data [attributes] of remote resource"
                                                        single instance                +-----------------------+
                                                      +------------------------------> |  ResourceCacheManager |
                                                      |                         1:n    |-----------------------|
                                                      |                        +-----◇ |  s_cacheDataList      |   1:n
"Monitor the state of remote resource"                |                        |       |  m_observeCacheList   |◇ ------+
  +----------------------+                            |                        +-----◇ |  cacheIDmap           |        |
  |   ResourceBroker     |--------------+             |                        |       |  observeCacheIDmap    |◇ ------+
  |----------------------|              |             |                        |       |-----------------------|        |
  |   s_presenceList     |◇ --+         |             |  (cache attrs)         |       |  requestResourceCache |        |
  |   s_brokerIDMap      |    |         |             |  RCSResourceAttributes |       |  updateResourceCache  |        |
  |----------------------|    |1:n      |             |               ^        |       |  getCachedData        |        |
  |   hostResource [[2]] |    |         |             |               |        |       |  findDataCache        |        |
  | findResourcePresence |    |         |  (cb-2)     |               |        |       +-----------------------+        |
  +----------------------+    |         | +--------+  |    attributes |        |                                        |
                              |         | |brokerId|  |               ◇        v                                        v
                   +----------+         | |brokerCB|  |          +--------------------------+               +--------------------+
                   |                    | +--------+  |          |       DataCache          |               |    ObserveCache    |
      +------------v---------------+    |     ^       |          |--------------------------|               |--------------------|
      |     ResourcePresence       |    |     |       |          |       sResource          |◇ ---+   +---◇ |    m_wpResource    |
      |----------------------------|    |     |1:n    |          |       attributes         |     |   |     |    m_attributes    |
      |     requesterList          |◇ --------+       |          |     subscriberList       |     |   |     |    m_reportCB      |
      |     primitiveResource      |◇ ----------------------\    |--------------------------|     |   |     |--------------------|
      |       expiryTimer (5s)     |    |             |      \   |  [add/delete]Subscriber  |     |   |     |  [start/stop]Cache |
      |----------------------------|    |             |       |  |      getCachedData       |     |   |     |     getCachedData  |
 +----|   registerDevicePresence   |    |             |       |  | on[Observe/Get/Timeout]  |     |   |     |     onObserve      |
 |    |[add/remove]BrokerRequester |    |             |       |  |   notifyObservers        |     |   |     +--------------------+
 |    |  [get/timeOut/polling]CB   |    |             |       |  +--------------------------+     |   | only observe
 |    | executeAllBrokerCB (cb-2)  |    |             |       |                                   |   |
 |    +----------------------------+    |             |       |                                   |   |
 |        |                             |             |       |                                   |   |
 |        |            +-----------------------------------+  +-----------------------------------+---+---+
 |        |            |      RCSRemoteResourceObject      |                                      |   |   |
 |        |            |-----------------------------------|                                      |   |   |
 |        |            |        m_primitiveResource        |◇ --------------------------------+   |   |   |
 |        |            |        m_cacheId                  |                                  |   |   |   |
 |        |            |        m_brokerId                 |                                  |   |   |   |
 |        |            |-----------------------------------|                                  |   |   |   |
 |        |            |  [from/to]OCResource              |-----------+                      v   v   v   v
 |        |      [[1]] |  start[Monitoring/Caching]        |           |                  +--------------------------------+
 |        |            |  [get/set]RemoteAttributes        |           |                  |      PrimitiveResource         |
 |        |            |  get[Address/Uri/Types/Interfaces]|           |                  |--------------------------------|
 |        |            +-----------------------------------+           |                  |            create              |---+
 \        |                         ^                                  |             [[3]]|   request[Get/Set/Put][With]   |   |
  \       |1:n  ? one to one ?      |cb-1                          from|remote            | get[Uri/Host/Types/Interfaces] |   |
   \      |resourcePresenceList     |                       +----------------------+      |       requestObserve           |   |
    \     ◇                         |                       |   OC::OCResource     |      +--------------------------------+   |
+------------------------------+    |                       |----------------------|                      △                    |
|       DevicePresence         |    |                       |   m_clientWrapper    |                      |                    |
|------------------------------|    |                       |----------------------|                      |                    |
|     resourcePresenceList     |    |                       | uri/types/Interfaces |           +-----------------------+       |
|         address              |    |                       |devAddr/useHostString |           | PrimitiveResourceImpl |  <----+
|     presenceSubscriber       |◇ ----------                |                      |           |-----------------------|                                                                                                                                                                                                                                                                                             +--------------------------------+
|     presenceTimer (15s)      |    |       \               | serverHeaderOptions  | <-------◇ |     m_baseResource    |                                                                                                                                                                                                                      -------------------------------+                                       |      PrimitiveResource         |
|------------------------------|    |        \              | observeHandle        |           +-----------------------+                                                                                                                                                                                                                           PrimitiveResource         |                                       |--------------------------------|
|     addPresenceResource      |    |         \             | children/endpoints   |                                                                                                                                                                                                                                                          -------------------------------|                                       |            create              |---+
|    [subscribe/timeOut]CB     |    |          \            | headerOptions        |                                                                                                                                                                                                                                                                     create              |---+                                   |   request[Get/Set/Put][With]   |   |
| [add/remove]PresenceResource |    |           \           |                      |                                                                                                                                                                                                                                                            request[Get/Set/Put][With]   |   |                                   | get[Uri/Host/Types/Interfaces] |   |
+------------------------------+    |            \          |----------------------|                                                                                                                                                                                                                                                          get[Uri/Host/Types/Interfaces] |   |                                   |       requestObserve           |   |
                                    |   "/oic/ad" |         | get/put/post/observe |                                                                                                                                                                                                                                                               requestObserve           |   |                                   +--------------------------------+   |
                                    |             |         |  subscribe/publish   |                                                                                                                                                                                                                                                                               -------------------------------+   |                                                   △                    |
                                    |             |         +----------------------+                                                                                                                                                                                                                                                                                                      △                    |                                                   |                    |
                                    |             v                                                                                                                                                                                                                                                                                                                            |                    |                                                   |                    |
                                    |     +------------------------+                                                                                                                                                                                                                                                                                                                                                                           +-----------------------+       |
                                    |     |  PresenceSubscriber    |                                                                                                                                                                                                                                                                                                      +-----------------------+       |                                        | PrimitiveResourceImpl |  <----+
                                    |     |------------------------|                                                                                                                                                                                                                                                                                                  | PrimitiveResourceImpl |  <----+                                        |-----------------------|
                                    |     |        m_handle        |                                                                                                                                                                                                                                                                                                   |-----------------------|                                        ------◇ |     m_baseResource    |
                                    |     |------------------------|                                                                                                                                                                                                                                                                                    -◇ |     m_baseResource    |                                                +-----------------------+
                                    |     |  [un]subscribePresence |                                                                                                                                                                                                                                                                             +-----------------------+
                                    |     +------------------------+
                      +-------------+
                      |
                      |                                                                             +------------------+
                      |     +---------------------------+         +--------------+              --> | RCSAddressDetail |
                      |     |   RCSDiscoveryManager     |         |  RCSAddress  |             /    |------------------|
                      |     | ------------------------- |         |--------------|            /     |     m_addr       |
                      |     |                           |         |   m_detail   |◇ ---------/      |------------------|
                      |     |    discoverResource ((1)) |         |--------------|                  |    getAddress    |
                      |     |  discoverResourceByType   |         |   multicast  |                  +------------------+
                      |     |  discoverResourceByTypes  |         |   unicast    |
                      |     +---------------------------+         +--------------+
                      |                  |                              ^                    +-------------------------+
                      |                  |                              |                    |  DiscoveryRequestInfo   |
                      |                  |                              |      m_address     |-------------------------|
                      |   +--------------------------------+            +------------------◇ |     m_address           |
                      |   |     RCSDiscoveryManagerImpl    |                                 |     m_relativeUri       |
                      |   |--------------------------------|     1:n                         |     m_resourceTypes     |
                      |   |        m_discoveryMap          |◇ ------------------------------>|     m_discoverCb ((4))  | <---+
                      |   |        m_timer                 |     m_discoveryMap              |     m_knownResourceIds  |     |
                      |   |--------------------------------|             |                   |-------------------------|     |
                      |   |        startDiscovery ((2))    |<---+        |               +---|     discover            |     |
                ((4)) +---|  onResourceFound (call cb-1)   |    |loop    |               |   |     addKnownResource    |     |
                      |   |        onPolling(60s)          |----+        |               |   +-------------------------+     |
                      |   |        onPresence              |             | (erase)       |                                   |
                      |   |        cancel                  |-------------+               v  ((3))                            |
                      |   | subscribePresenceWithMulticast |      (id)          OC::OCPlatform::findResource                 |
                      |   +--------------------------------+                                                                 |
                      |                                                                        OC::OCResource                |
                      +------------------------------------------------------------------------------------------------------+
                                                      std::function  DiscoverCallback
```

Sample实例流程图
================
```

                SampleResourceServer                                      RCSRemoteResourceObject           SampleResourceClient
                      |                                                            |                   ((1))               |
                      |--> startPresence                                           |(param:rro)        discoverResource <--| 1
                      |                                                            |                                       |
                      |--> initServer                                              |         discoverResourceByType        |
                      |                   (server use)                 cb-1        v                        |              |
                      |     g_resource (RCSResourceObject)            onResourceDiscovered                  |              |
                      |         |                                     ((4)) |             ((2))             |              |
                      |         |--> setAutoNotifyPolicy                    |             startDiscovery <--|              |
                      |         |--> setSetRequestHandlerPolicy             |                        |      |              |
                      |         |                                           |  m_discoveryMap.insert |      |              |
                      |         |--> setAttribute("Brightness")             |                        |                     |
                      |         |--> setGetRequestHandler ---------+        |           discovery <--|                     |
                      |         |--> setSetRequestHandler ---------+        |  ((3))           |     |                     |
                      |         |                                  |        |  findResource <--|                           |
                      |                                            |        |                  |"/oic/res?rt=oic.r.light"  |
                 loop |--> updateAttribute                         |        |                                              |
                      |                                            |        |--> rro->getUri                               |
                      |     g_resource (RCSResourceObject)         |        |                                              |
                      |         |                                  |        |--> rro->getAddress                           |
                      |         |--> getAttributes                 |        |                                              |
                      |         |                                  |        |--> g_discoveredResources.push_back(rro)      |
                      |         |--> getAttributeValue             |        |                                              |
                      |         |                                  |                                (client use)           |
                                                                   |          g_selectedResource (RCSRemoteResourceObject) |
             +----------------------------------------+------------+                                                       |
             |                                        |                                                                    |
             |                                        |                                                 [[1]]              |
             v                                        v                cb-2                             startMonitoring <--| 2
   requestHandlerForGet                     requestHandlerForSet      onResourceStateChanged                      |        |
     |                                                      |         [[4]] |                     [[2]]           |        |
     |                                                      |               |                     hostResource <--|        |
     |    RCSGetResponse                 RCSSetResponse     |               |                              |               |
     |        |                                |            |               |   initializeResourcePresence |               |
     |        |         defaultAction          |                            |        addBrokerRequester <--|               |
     |        +--------------------------------+                            |                 |            |               |
     |                        |                                             |   [[3]]         |            |               |
     |                        |                                             |   requestGet <--|                            |
     |                        +-------------------------------\             |                                              |
     |                                                         \            |                      (polling)               |
     |                                                          \           |--> do nothing                                |
     |                                                           \          |                                              |
     |                                                            \                                                        |
     |                                                             \                                                       |
     |                                                              v  cb-3                   [get/set]RemoteAttributes <--| 3
     |                                                                onRemoteAttributesReceived                  |        |
     |                                                                  /         |                               |        |
     |                                                        (param)  /          |           request[Get/Set] <--|        |
     |                                                                /           |                               |        |
     |                                                               /            |      request[Get/Set]With              |
     |                                               RCSResourceAttributes        |                                        |
     |                                                       ^                    |                                        |
     |                                                       |                    |                                        |
     |                                                       ◇                                                             |
     |                                               RCSRepresentation                                                     |
     |                                                           \                                                         |
     |                                                   (param)  \                              [get/set]WithInterface <--| 4
     |                                                             \   cb-4                                       |        |
     |                                                              \ onRemote[Get/Set]Received                   |        |
  if |--> "interface == test.custom"                                              |             "?if=test.custom" |        |
     |         |                                                                  |                               |        |
     |         |--> attr["blob"] = bin  (new attr)                                |       request[Get/Set]With <--|        |
     |         |                                                                  |                               |        |
     |         |--> RCSGetResponse::create(attr)      interfaces is empty         |                               |        |
     |         |                                      resourceTypes is empty      |                                        |
     |         |       m_customRep = true        ----------------------------->   |                                        |
     |         |                                      key : blob                  |                                        |
     |                                                                            |                                        |
 else|--> "interface != test.custom"                                              |                                        |
     |         |                                                                  |                                        |
     |         |                                                                  |                                        |
     |         |--> RCSGetResponse::defaultAction()                               |                                        |
     |         |                                      g_defaultHandlers           |                                        |
     |         |       m_customRep = false       ----------------------------->   |                                        |
     |                                                                            |                                        |
     |     "oic.if.baseline"   "oic.if.a"   "oic.if.s"     "oic.if.b"                                                      |
     |                                                                                                                     |
     |                                                                                                                     |
     |                                                                                         startCachingWithCallback <--|5
                   (observes)                                          cb-5                                       |        |
                   subscriberList.findSubscriber(reportID)            onCacheUpdated                              |        |
                                        \                              ^  |                                       |        |
                                         \                             |  |               requestResourceCache <--|        |
                                   +-----------------+                 |  |                              |        |        |
                                   | +-------------+ |                 |  |    OBSERVE_WITH_POLLING      |        |        |
                                   | | Report_Info | | +-> NONE        |  |    UPTODATE (10s)            |                 |
                                   | |-------------| | |               |  |                              |                 |
                                   | |    rf       |---+-> UPTODATE    |  |           DataCache.init  <--|                 |
                                   | |  reportID   | | |               |  |                       |      |                 |
                                   | |  repeatTime | | +-> PERIODICTY  |  |                       |      |                 |
                                   | |  timerID    | |                 |  |      ObserveResource  |      |                 |
                                   | +-------------+ |                 |  |                       |      |                 |
                                   |                 |                 |  | onGet      onTimeOut  |      |                 |
                                   | +-------------+ |                 |  |   |           |       |      |                 |
                                   | |    fun      |-------------------+  |   |           | (15s) |      |                 |
                                   | +-------------+ |                    |   | onObserve |       |      |                 |
          (main thread)            +-----------------+                    |   |     |     |       |      |                 |
         runResourceControl                                               |   +---+ |                    |                 |
             |                                                            |       | |   addSubscriber <--|                 |
             |                                                            | call  v v                    |                 |
             |--> updateAttribute                                         |<-- notifyObservers           |                 |
             |    {                                                       |               (client)       |                 |
             |        RCSResourceObject::LockGuard                        |  foreach:                                      |
             |                               \       notifyAllObservers   |        subscriberList                          |
             |    }                           \                (server)   |                                                |
             |                              autoNotify -----------------> |                                                |
             |                                           DoResponse       |                                                |
             |                                                            |                                                |
                                                                                                    getCachedAttributes <--|6
                                                                                                                  |        |
                                                                                                                  |        |
                                                                                                                  |        |
                                                                                                getCachedData  <--|        |
```
