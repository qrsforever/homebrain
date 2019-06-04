---

title: Iotivity之Payload
date: 2018-05-10 12:41:16
tags: [ IOT, DrawIt ]
categories: [ Note ]

---

Payload类图
===========

```

        +-----------------+                                                 +-------------------+
        |OCSecurityPayload|----------------------+                        / | OCPresencePayload |
        |-----------------|                      |                       /  |-------------------|
        |      base       |                      ▽                      /   |       base        |
        |                 |               +-------------+              /    |                   |
        | secureData/size |               |  OCPayload  | ◁ -----------     |  sequenceNumber   |
        +-----------------+               |-------------|                   |  maxAge/trigger   |
                                          |    type     |                   |   resourceType    |
                                          +-------------+                   +-------------------+
                                             △       △ 
                                             |       |
                                    /--------+       |
           (inner)                 /                 |            (outter)
          +---------------------+ /                  |           +----------------------+
          | OCDiscoveryPayload  |/                   +---------- |    OCRepPayload      |
   sid    |---------------------|                                |----------------------|
    |     |       base          |                                |        base          |
 deviceid |                     |                                |                      |
          | sid/name/type/iface |                                | uri/types/interfaces |
   +----◇ |     resources       |                                |       values         |◇ -----+
   |      |---------------------|                                |----------------------|       |
   |      |       next          |                                |        next          |       |
   |      +---------------------+                                +----------------------+       |
   |                                                                                            |
   v                                             OCRepPayloadCreate                             |
 +------------------------+                      OCRepPayloadAppend                             v
 |  OCResourcePayload     |                      OCRepPayloadSetPropXXX  +--------+    +-------------------+
 |------------------------|                                              | NULL   |    | OCRepPayloadValue |
 | uri/types/Interfaces   |                                              | INT    |    |-------------------|
 | anchor/port/secure/rel |                                              | DOUBLE |    |     name          |
 |------------------------|  OCDiscoveryPayloadCreate                    | BOOL   |<---|     propType      |
 |        next            |  OCDiscoveryPayloadAddNewResource            | STRING |    |     value?(union) |
 +----------|-------------+  OCDiscoveryPayloadAddResource               | OJBECT |    |-------------------|
            |                                                            | ARRAY  |    |     next          |
            |                                                            +--------+    +-------|-----------+
  link all filter ok source                                                                    |
                                                                                               |
                                                                                       for collection using.

```
