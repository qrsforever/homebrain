---

title: Iotivity之请求响应流程图
date: 2018-04-24 16:40:06
tags: [ IOT, DrawIt ]
categories: [ Note ]

---

CA连接抽象层
============
```
                                                                                                    (thread3/4/5)
                                                                                               CAQueueingThreadBaseRoutine
                                                                                                                  |
                                        CAQueueingThread                                                          |
                                        +-----------------------------+                     queue_get_element  <--|
                                        |            |                |                                           |
                                        | threadTask |   dataQueue   *|----------------------                     |
                                        |            |                |                      \                    |
                                        +-----------------------------+                       \
                                          (sendThread/receiveThread)                           \
                                                                                               v
                                                                                          +----------------+
                                                                                          |      next      |
                      +--------------------------------------------------------+          |  +----------+  |
                      |     thread5       +-------------+                      |          |  | msg,size |  |
                      |         \         |             |                      |          |  +----------+  |
                 ((7))v          \        v             |                      |((6)      +----------------+
                 sendDataToAll    ---- sendData  CASendUnicastData     CASendMultica               |
                      ^                   ^             ^                      ^                   |
                      | g_adapterHandler  |             |                      |                   v
                      +---------+---------+             +-----------+----------+          +----------------+
                                |                                   |                     |      next      |
                                |                                   |                     |  +----------+  |
  get  put post delete          | +---------------------------------|-------------+       |  | msg,size |  |
   |    |    |    |             | | (x)                    |                      |       |  +----------+  |
   ----------------             | | CAReceiveThreadProcess | CASendThreadProcess  |       +----------------+
           ^                    | |                        |     thread3          |
           |                    | +-----------------------------------------------+
    CA_REQUEST_DATA   <--|      |                                   ^                                             |
                         | code |    thread4           r-3          |                                             |
    CA_SIGNALING_DATA <--|------+--- CAReceivedPacketCallback ------+-------\                                     |
                         |      |     |  get code/token from PDU    |        \              queue_add_element  <--|
    CA_RESPONSE_DATA  <--|      |     |                             |         \                                   |
                                |     |  g_receiveThread       g_sendThread    \                                  |
                                |     |         ^                   ^           \       r-4        ((5))          |  [[3]]
                                |     +-----+   | CAQueueingThread  |            ----------------> CAQueueingThreadAddData
                                |      set  |   +---------+---------+                                         ^
                                |           |             |                                                   |
+-------------------------------+-----------+-------------+---------------------------------------------------+--------------+
| DIR: connectivity             |           |             |                                                   |              |
|                                           |             |                                                   |              |
|                       interfacecontroller |             |                   retransmission                  |              |
|                               |           |             |                          |                        |              |
|                               |           |                                        |                                       |
| connectivitymanager           |           |      messagehandler                    |                 queueingthread        |
|        |                      |           |             |                          |                        |              |
+--------+----------------------+-----------+-------------+--------------------------+------------------------+--------------+
         |                      |           |             |                          |                        |
    h-1  |                      |           |  h-2        |                          |             h-4        |
    CAInitialize                |           |  CAInitializeMessageHandler            |             CAQueueingThreadInitialize
                                |           |      /                                 |             CAQueueingThreadStart
                                |           |     /CAReceivedPacketCallback          |                 (thread create)
                      h-3       |      r-1  v    /                                   |
                      CASetPacketReceivedCallback                                    |
                      CASetErrorHandleCallback                                       |
                                                                         h-5         |
                         g_adapterHandler  --------------\               CARetransmissionInitialize
                                                          \              CARetransmissionStart
                                                           \ 1...n
             CAInitializeRA             CAInitializeNFC     ------------------------\
                       \                  /                                          \             +--------------------------+
                        \                /                                            -----------> |   CAConnectivityHandler  |
                      h-6\  transtype   /                                 r-2                      |--------------------------|
                      CAInitializeAdapters                   +--> CAReceivedPacketCallback  <--------startAdapter (thread4/5) |
                      /         |        \                   |                                     | stopAdapter              |
                     /          |         \                  +--> CAAdapterChangedCallback         | startListenServer        |
                    /           |          \          params |                                     | stopListenServer         |
       CAInitializeTCP   CAInitializeEDR   CAInitializeIP ---+--> CAConnectionChangedCallback      | startDiscoveryServer     |
                                                             |                                     | sendData      (unicast)  |
                                                         non +--> CAAdapterErrorHandleCallback     | sendDataToAll (muticast) |
    h-7                                                      |                                     | getNetInfo        ((8))  |
    CASelectNetwork                                          +--> CARegisterCallback  -----------> | readData                 |
                                                                                         handler   | terminate                |
                                                                                                   | transportType            |
    h-8                                        h-9                                                 +--------------------------+
    CARegisterHandler                          CASetInterfaceCallbacks
      /     |     \                             /         |         \
     /      |      \                           /          |          \
    /       |       \     call                /           |           \
Request  Response  Error  <--- g_requestHandler   g_responseHandler   g_errorHandler
 cb-a     cb-b     cb-c             cb-a                cb-b               cb-c

```

Handle层
========

```
```                                                         |
                                                            |-->  g_receiveThread
                                               h-11         |                          r-6
+--------------------------------------------> CAHandleRequestResponseCallbacks (queue_get_element)
|                                                       thread2
| +---------------------------------+                                                     g_serverRequestTree
| | HandleVirtualResource           |   call               cb-1                                   |
| | HandleResourceWithEntityHandler |  -----> resource->entityHandler()                           |GetServerRequestUsingToken
| | HandleDefaultDeviceEntityHandler|                                                             |
| +---------------------------------+  /----> resouce = FindResourceByUri(request->resourceUrl)   v
|       ^                             /    +---------------------------------------------------------------+
|  h-12 |                            / +-->|                       OCServerRequest                         |<-----------------+
|  ProcessRequest     +-------------/  |   |---------------------------------------------------------------|                  |
|       ^             |                |   |   method, resourceUrl, acceptFormat, payloadFormat, devAddr   |                  |
|       | (by uri)    |                |   |   numResponses, qos, options, observeResult, delayedResNeeded |                  |
|DetermineResourceHandling             |   |   ehResponseHandler, requestId, requestToken, coapID, query   |                  |
|       ^                              |   +---------------------------------------------------------------+                  |
|       |          AddServerRequest----+              | ehResponseHandler                                                     |
|       | +3        +2 ^                      [[2]]   v       cb-3                                                            |
|       |              |                      HandleSingleResponse                                                            |
|      HandleStackRequests                                   |                                                                |
|             ^  | +1                                        v                                                                |
|             |  |                                   OCSendResponse --->  CASendResponse  --->  CAQueueingThreadAddData       |
|             |  |                                                                                                            |
|             |  +--> GetServerRequestUsingToken         +---->  GetClientCBUsingToken  <-----------+                         |
|             |                                          |               |                          |                         |
|             |                                          |               |                          |                         |
|       OCHandleRequests                          OCHandleResponse       |      cb-2                |                         |
|             ^                                          ^             client->callback()           |                         |
|       cb-a  |   thread2                         cb-b   |                                  cb-c    |                         |
|       HandleCARequests [get|put|post|delete]    HandleCAResponses                         HandleCAErrorResponse             |
|             ^                                          ^                                          ^                         |
|             |   r-7                                    |  r-7                                     |  r-7                    |
|             +------------------------------------------.------------------------------------------+                         |
|                                                receive | handle                                                             |
|                                                        |                                                                    |
```

启动架构
========

```
|                                                       CSDK                                                                  |
|                                          +-----------------------------+                                                    |
|                                          |                             |                                                    |
|                                          |    OCInitializeInternal     |                                                    |
|           server sample                  |             ^               |                   client sample                    |
|     +-----------------------+            |             |               |             +----------------------+               |
|     |                       |            |             |               |             |                      |               |
|     |Platform::start()   ----------------+--------> OCInit2 <----------+-----------------  Platform::start()|               |
|     |                       |            |             |               |             |                      |               |
|     |                       |            |             |               |             |                      |               |
|     |     +--------------+  |            |             v               |             |   +--------------+   |               |
|     |     |     start    |  |            |      init core resources    |             |   |     start    |   |               |
|     |     |              |  |            |                             |             |   |              |   |               |
|     |     |              |  |thread2     |           r-5               |      thread2|   |              |   |               |
|     |     |  processFunc  ---------------+-------> OCProcess <---------+-----------------  listeningFunc|   |               |
|     |     |              |  |            |             |               |             |   |              |   |               |
|     |     +--------------+  |            |             |               |             |   +--------------+   |               |
|     |    wrapper            |            +-------------+---------------+             |   wrapper            |               |
|     |                       |                          |                             |                      |               |
|     +-----------------------+                          |                             +----------------------+               |
|     main thread                               /--------+                                          main thread               |
|                                              /                                                                              |
|                          /------------------/                                                                               |
|                         /                                                                                                   |
|   h-10                 v                                                                                                    |
+-- CAHandleRequestResponse                                                                                                   |
                                                                                                                              |
```

CS交互API框图
=============
```
                                                   |                                                                          |
                                                   |                                                                          |
                               RPC      <--------- | --------->      RPC                                                      |
                                                   |                                                                          |
                                         a         |          1                                                               |
                                OCDoResponse       |         OCDoRequest                                                      |
                                        \                      /                                                              |
                            +------------\--------------------/------------+                                                  |
                            |             \  b          2    /             |                                                  |
                   SERVER   |      CASendResponse     CASendRequest        |   CLIENT                                         |
                            |              \                /              |                                                  |
                     ^      |               \ c         3  /               |     ^                                            |
                     |      |             CADetachSendMessage              |     |                                            |
                     |      |                      ^                       |     |                                            |
                 ---------  |                      |                       | ---------                                        |
                     |      |            d         v         4             |     |                                            |
                     |      |           CAHandleRequestResponse            |     |                                            |
                     v      |                /            \                |     v                                            |
                            |         e     /              \     5         |                                                  |
                   CLIENT   |   HandleCAResponses     HandleCARequests     |   SERVER                                         |
                            |             /                  \             |                                                  |
                            +------------/--------------------\------------+                                                  |
                                   f    /                      \     6                                                        |
                                OCHandleResponse         OCHandleRequests                                                     |

```

Sample流程
==========

```
                                                                                                                              |
=================server==================================sdk========================================================          |
                                                                                                                              |
     {{1}}                                                                                                                    |
     Platform::registerResource                                                                                               |
              |                           cb-1                                                                                |
              |(uri,type,interface,attr,eHandler)    {{2}}                                                                    |
              +---------------------------------->   OCCreateResource                                                         |
                                        thread2       insertResource         headResource                                     |
                                                                                 |                                            |
                                                    +----------------+           |                                            |
                                                    |   OCResource   | <---------+ (resourceHandler)  <-------------------+   |
                                OCResourceType      |----------------|                                                    |   |
                               +-------------+      |  uri           |      OCResourceInterface                           |   |
                               | next | name | <----|  resType       |       +-------------+                              |   |
                               +-------------+      |  resInterface  |-----> | name | next |                              |   |
                           OCResourceProperty       |  resAttributes |-----+ +-------------+                              |   |
                    +------+--------+-----+---------|  resProperties |     |                               uri            |   |
                    |      |        |     |         |  actionsetHead |     |       OCAttribute  OCF only for platform     |   |
                    |      v        |     v   +-----|  childresHead  |---+ |   +---------------------+   and device info  |   |
                    |  discoverable |  active | +---|  observerHead  |   | +-> | name | value | next |                    |   |
                    |               |         | |   |  entityHandler |   |     +---------------------+                    |   |
                    v               v         | |   |  uuid  (cb-1)  |   |                                                |   |
                  slow         observable    /  |   |----------------|   |       OCChildResource                          |   |
                                            /   |   |       next     |   |     +-----------------+                        |   |
                                     +------    |   +----------------+   +---> | resource | next |                        |   |
                    OCActionSet      v          |                              +-----------------+                        |   |
               +-------------------------+      |                                                                         |   |
               | name | timesteps | type |      v ResourceObserver                                                        |   |
               |-------------------------|    +-------------------------------------------------+                         |   |
               |     head   |   next     |    | id | uri | query | token | devAddr | qos | next |                         |   |
               +-------------------------+    +-------------------------------------------------+                         |   |
                                                                                                                          |   |
=========================================================sdk======================================client============      |   |
                                                                                                                          |   |
                                                                                           ((1))                          |   |
                                                                                           Platform::findResource         |   |
                                                                                                        |                 |   |
                                                     ((2))                                              |                 |   |
                         gen: resHandle & token      OCDoResource    <----------------------------------+                 |   |
                       /---------------------------- OCDoRequest       (host, uri, conntype, callback)                    |   |
                      /                                                                          | cb-2                   |   |
               ((3)) /                                                                           |                        |   |
               AddClientCB                                                          [Find|Get|Put|Post]Callback           |   |
                                                       +--------------------+                                             |   |
                g_cbList ----------------------------> |    ClientCB        |                                             |   |
                                                       |--------------------|      +--> con                               |   |
                                    text <---+         |    callback(cb-2)  |      |                                      |   |
  GetClientCBUsingUri                        |         |    handle (random) |      +--> non                               |   |
  GetClientCBUsingToken             xml  <---+         |    type            |------|                                      |   |
  GetClientCBUsingHandle                     |         |    token  (random) |      +--> ack                               |   |
                                    json <---+         |    options         |      |                                      |   |
                                             |         |    payload         |      +--> reset            discover         |   |
                                    cbor <---+---------|    payloadFormat   |                               ^             |   |
                 ip <---+                              |    context         |                               |             |   |
                        |   +---------------+          |    method          +---+-----+-----+-----+-----+---+             |   |
          bluetooth <---+   |   OCDevAddr   |          |    sequenceNumber  |   |     |     |     |     |                 |   |
                        |   |---------------|          |    requestUri      |   v     v     v     v     v                 |   |
                nfc <---+---|    adapter    |<---------|    devAddr         |  get   put  post  delete observe            |   |
                        |   |    flags      |          |--------------------|                                             |   |
                nfc <---+   |    port       |          |    next            |                                             |   |
                        |   |    addr       |          +--------------------+                                             |   |
               xmpp <---+   |---------------|                                                                             |   |
                            |   routeData   |                                                                             |   |
                            |   remoteId    |        ((4))                                                                |   |
                            +---------------+        OCSendRequest  --->  CASendRequest  --->  CAQueueingThreadAddData    |   |
                                                                                                                          |   |
                                                                                                                          |   |
=================server==================================sdk========================================================      |   |
                                                                                                                          |   |
                            entityHandler (request)                                                                       |   |
                                [[0]] |       |                                                                           |   |
                                      |       |                                                                           |   |
 [[1]]                                |       | (param)                            Request  ---->  Response               |   |
 Platform::sendResponse (response) <--|       |                                                                           |   |
                |               \     |       |                                                                           |   |
                |  (response)    \            |      [[2]]                                                                |   |
                +-----------------\-----------+--->  OCDoResponse   -----------\ call                                     |   |
  Wrapper:                         \          |                                 \                                         |   |
                                    ------+   |           csdk                   -----> requestHandle->ehResponseHandler  |   |
c++ +---------------------------+         |   |           +------------------------+                   cb-3               |   |
    |    OCResourceRequest      |<------------+           | OCEntityHandlerRequest |                                      |   |
    |---------------------------|         |               |------------------------|    resource (resourceHandle)         |   |
    |  messageID,representation |         |               |    method, resource    |--------------------------------------+   |
    |  devAddr, query, options  |         |     form      |    devAddr, query      |                                          |
    |  payload                  |<------------------------|    options             |                                          |
    |  requestHandle            |         |               |    payload             |     requestHandle                        |
    |  resourceHandle           |         |               |    requestHandle       |------------------------------------------+
    +---------------------------+         |               |    messageID           |                                          |
                 |                        | (param)       +------------------------+                                          |
          handle | get/put/post           |               csdk                                                                |
                 v                        |               +------------------------+                                          |
         c++ +-------------------+        |               |OCEntityHandlerResponse |                                          |
             |OCResourceResponse | <------+               |------------------------|     requestHandle                        |
             |-------------------|              form      |     requestHandle      |------------------------------------------+
             |  newResourceUri   |----------------------->|     ehResult           |
             |  interface        |                        |     resourceUri        |
             |  headerOptions    |                        |     resourceHandle(x)  |
             |  representation   |                        |     payload            |
             |  requestHandle    |                        +------------------------+
             |  resourceHandle   |
             +-------------------+
```
