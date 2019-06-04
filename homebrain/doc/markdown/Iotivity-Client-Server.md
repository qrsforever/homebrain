---

title: Iotivity之Client&Server回调流程图
date: 2018-04-25 18:32:50
tags: [ IOT, DrawIt ]
categories: [ Note ]

---

C++层类图
=========

```
Client ---> OC::OCResource ---> InProcClientWrapper ---> CSDK
                            |
                            |
                     OC::OCRepresentation
                                   |
                                   |
Server ---> OC::OCResourceRequest ---> OC::OCResourceResponse ---> InProcServerWrapper ---> CSDK

       +-------------------+
  +--->|OCResourceResponse |
  |    |-------------------|                            +-------------------+
  |    |  newResourceUri   |                            |                   |
  |    |  interface        |                            v                   |
  |    |  headerOptions    |  representation   +--------------------+       |
  |    |  representation   |◇ -------------->  | OCRepresentation   |       |
  |    |  requestHandle    |     (report)      |--------------------|       |
  |    |  resourceHandle   |                   | host/uri/types/ifs |   1:n |
  |    |-------------------|                   |    m_children      |◇ -----+
  |    |    getX/setX      |                   |--------------------|
  |    +-------------------+                   |  [get/set]Value (*)|
  |                                            |                    |
  |                                            |    setter/getter   |
  |                                            +--------------------+
  |                                                   |      |
  |                                                   |      |  get/put/post
  |           +---------------------------------------+      +--------------------+
  |           | representation                                                    |
  |           |          (update)                                                 |
  |           ◇                                                                   |
  |  +---------------------------+                                      +----------------------+
  |  |   OC::OCResourceRequest   |                                      |   OC::OCResource     |
  |  |---------------------------|      +----------------------+        |----------------------|      +----------------------+
  |  |  messageID,representation |      | InProcServerWrapper  |        |   m_clientWrapper    |◇ --->| InProcClientWrapper  |
  |  |  devAddr, query, options  |      |----------------------|        |----------------------|      |----------------------+
  |  |  payload                  |      |    start/stop        |        | uri/types/Interfaces |      |   start/stop         |
  |  |  resourceHandle           |      |    registerResource  |        |devAddr/useHostString |      |   ListenForResource  |
  |  |  requestHandle            |      |                      |        |                      |      |                      |
  |  |---------------------------|      |    bindxxxToResource |        | serverHeaderOptions  |      | xxxRepresentation    |
  |  |       getX/setX           |      |    [type/interface]  |        | observeHandle        |      |   [Get/Put/Post]     |
  |  +---------------------------+      |                      |        | children/endpoints   |      |                      |
  |            ◇                        |    startPresence     |        | headerOptions        |      | ObserveResource      |
  |            | requestHandle          |    sendResponse      |        |                      |      | SubscribePresence    |
  |            |                        |----------------------|        |----------------------|      |----------------------|
  |            v                        |    processFunc       |        | get/put/post/observe |      | listeningFunc        |
  |   +---------------------+           +----------------------+        |  subscribe/publish   |      +----------------------+
  |   |  OCServerRequest    |                                           +----------------------+
  |   |---------------------|
  |   |  ehResponseHandler  |
  |   +--------+------------+
  |            |
  |            v
  |  HandleSingleResponse
  |            |
  |            |
  +------------+
```

C-S流程图
============
```

 SERVER                                                                                                        CLIENT
                                                                 OC::OCResource

                           entityHandler                 stack                   FoundResource
                                cb  uri-1                  |                          cb   call-1
        simpleserver            |                          |                           |        simpleclient
                                |                          |                           |
   Platform::registerResource   |                          |                           |    Platform::findResource
              |                 |                          |                           |                |
              |  "/a/light"     |OCCreateResource          |                           |                |
              +-----------------+------------------------> | OCDoResource              |  "/oic/res"    |
                                |     OCResourceRequest    | <-------------------------+----------------+
                                | <----------------------- |                           |
                                |     OCDoResponse         |                           |  curResource
         Platform::sendResponse | -----------------------> | OCResource                |      ||
                                |                          | ------------------------> | "/a/light"
                                |                          | OCDoResource              |
                                |     OCResourceRequest    | <------------------------ | GetResourceRepresentation
                          doGet | <----------------------- |                    onGet  |
                                |                          |                     cb    |
         Platform::sendResponse | -----------------------> | OCResource           |
                                |                          | -------------------> |
                                |                          | OCDoResource         |
                                |                          | <--------------------| PutResourceRepresentation
                          doPut | <----------------------- |              onPut   |
                                |                          |               cb     |
         Platform::sendResponse | -----------------------> |                |
                                |                          | -------------> |
                                |                          |                |
                                |                          | <------------- | PostResourceRepresentation
                         doPost | <----------------------- |        onPost  |
                          /     |                          |          cb    |
                         /      |                          |           |
                        /       |                          |           |         FoundResource
                       /        |     entityHandler        |           |              cb  call-2
                      /         |          cb  uri-2       |           |               |
   Platform::registerResource   |          |               |           |               |    Platform::findResource
              |                 |          |               |           |               |                |
              |  "/a/light1"    |          |               |           |               |                |
              +-----------------+----------|-------------> |           |               |                |
                                |          |               | <---------|---------------+----------------+
                                | <--------|-------------- |           |               |
                                |          |               |           |               |  curResource
         Platform::sendResponse | ---------|-------------> |           |               |      ||
                                |          |               | ----------|-------------> | "/a/light1"
                                |          |               |           |               |
         Platform::sendResponse | ---------|-------------> | --------> |               |
                                |          |               |           |
                                |          |               |           |  "/a/light1"
                                           |OC_REST_OBSERVE| <-------- | observeResource <--- OC::OCRecource::observe
                                           | <------------ |
                          Observers.insert |               |                             onObserve
                                           |               |                                 cb
                                           |               |                                 |
                       |                   |               |                                 |
         loop   +----- |                   |               |                                 |
                |      |                   |               |                                 |
                |      |      OCDoResponse |               |                                 |
notifyListOfObservers  | ------------------|-------------> |      wrapper switch             |
                |      |                   |               | ------------------------------> |
                |      |
                |      |
                +----> |
                       |
                       |
                       |
                       |
                       |
                       |
  ChangeLightRepresentation  presence/representation
         thread3
```
