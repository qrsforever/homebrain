---

title: Iotivity之Colloction流程
date: 2018-04-27 15:02:43
tags: [ IOT, DrawIt ]
categories: [ Note ]

---

资源Collection类图
==================

```
                                                                ((1))
                                         +---------------+      OCBindResourceInterfaceToResource
                                         |     room      |
                                         |---------------|
                              /a/room <--|    uri        |     (must first)
                            core.room <--|    type       |     +-----------------------+
                                         |   interface   |---> |oic.if.baseline| next  |
                                         |               |     +-------------------|---+
     ((2))                     +---------| childresHead  |                         |
     OCBindResource            |         | entityHandler |----+                    v
                               |         |---------------|    |           +----------------+
                               |         |    next       |    |           |oic.if.b | next |
                               |         +---------------+    |           +------------|---+
                               |                              |                        |
                               v                              |                        v
                       +---------------+                      |               +-----------------+
                       |      fan      |                      |               |oic.if.ll | null |
                       |---------------|                      v               +-----------------+
             /a/fan <--| uri           |                    roomCB
           core.fan <--| type          |
    oic.if.baseline <--| interface     |
                       |               |
                       | childresHead  |
                       | entityHandler |---> fanCB
                       |---------------|                         +---------------+
                       |     next     -------------------------> |     light     |
                       +---------------+                         |---------------|
                                                     /a/light <--| uri           |
                                                   core.light <--| type          |
                                              oic.if.baseline <--| interface     |
                                                                 |               |
                                                                 | childresHead  |
                                                                 | entityHandler |---> lightCB
                                                                 |---------------|
                                                                 |    next       |
                                                                 +---------------+
```

Sample流程图
============

```

                                                HandleVirtualResource
                                                        |
        ((3))                                           |
        OCDoRequest                                     |
            |           uri = "/ioc/res"                |
 "discover" | ----------------------------------------> | ((4))
            |           all resoures                    |
            | <---------------------------------------- |
            |                                                       (if childresHead is not null)
            |      ((5))                                    ((6))
            | cb = discoveryReqCB                         HandleResourceWithEntityHandler
            |            |                                               |
            |            |                                               |     ((6))          (default)
                    OCDoRequest                                          |     HandleCollectionResourceDefaultEntityHandler
                         |                                               |                        |
                         |  "/a/room?if=oic.if.b"          roomCB != 0   |                        |
                   "get" | --------------------------------------------> |     roomCB == 0        |
                         |  "/a/room?if=oic.if.ll" (default)             | ---------------------> |
                         |  "/a/room?if=oic.if.baseline"      ((7))      |                        |
                         |                                    roomCB <-- |            (get)       |    (get/put/post)
                         |                                       |       |           ---+----------------------+---
                         |                (room,fan,light)       |                      |                      |
                         |           OCRepPayloadSetPropInt <--- |                      v                      v
                         |               OCRepPayloadAppend      |                  oic.if.ll               oic.if.b
                         |                         |             |                  oic.if.baseline            |
                         | <----- OCDoResponse --- |                                    |                      |
                         |                         |                         ((7))      v                ((7)) v
                         |                                                   HandleLinkedListInterface   HandleBatchInterface
                         |                                    (fan, light)              |                      |
                         |                              OCLinksPayloadArrayCreate <---- |                      |
                         |                         OCRepPayloadSetPropObjectArray       |                      |
                         |                                            |                                        |
                         |                                            |                                        | foreach (child)
                     +-- | <------------------- OCDoResponse -------- |                                        |
                     |   |                                                                   lightCB <-------- | "light"
           ((8))     |   |                                                                      |              |
      cb = getReqCB  |   | <------------------- OCDoResponse -----------------------------------|              |
              |      |   |                                                                                     |
              |      |   |                                                            fanCB <----------------- | "fan"
              |      |   |                                                              |                      |
              |      +-- | <------------------- OCDoResponse ---------------------------|                      |
              |          |
              |
          OCDoRequest                                      HandleResourceWithEntityHandler
              |                                                          |
              |                 (see above)                              |
        "put" | -------------------------------------------------------> |
              |                                                          |
              |                                                          |
```
