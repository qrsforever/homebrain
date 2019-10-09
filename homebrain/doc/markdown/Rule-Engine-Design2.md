---

title: 本地规则引擎设计

date: 2019-06-17 15:59:42
tags: [IOT, Design]
categories: [Note]

---


<!-- vim-markdown-toc GFM -->

* [总体设计](#总体设计)
* [日志系统](#日志系统)
* [消息系统](#消息系统)
* [引擎设计](#引擎设计)
    * [模型](#模型)
        * [设备模型](#设备模型)
        * [规则模型](#规则模型)
    * [CLP语言](#clp语言)
    * [RulePayload设计](#rulepayload设计)
    * [CLP设计](#clp设计)
    * [引擎UML](#引擎uml)
* [HttpHandler](#httphandler)
* [调试系统](#调试系统)
    * [系统监控(Python)](#系统监控python)
    * [规则配置(JS)](#规则配置js)

<!-- vim-markdown-toc -->

<!-- more -->

# 总体设计


```
  +----------------+         +-----------------+      +------------------+
  |                |         |                 |      |                  |
  |  elink cloud   |         |  debug monitor  |      |   debug webpage  |
  |                |         |    python       |      |       js         |
  |                |         |                 |      |                  |
  +----------------+         +-----------------+      +------------------+
             \                       |                                |
              \                      | tcp                            |
               \                     |                        restapi |
 +-----+        \                    v                                |      +-----+
 |     |   +----------------+    +---------+   +---------+            |      |     |
 |     |   |  gateway_agent |    | monitor |   |   clp   |            |      |     |
 |     |   |    (mqtt)      |    | (8192)  |   |         |            |      |     |
 |     |   +----------------+    +---------+   +---------+            |      |     |
 |     |                             |              |                 |      |     |
 |     |                             |              |                 |      |  m  |
 |     |                             v              v                 |      |  e  |
 |     |          +--------------------------------------+            |      |  s  |
 |     |          |                                      |            |      |  s  |
 |     |   +----> |            rule engine               |            |      |  a  |
 |     |   |      |                                      |            |      |  g  |
 |     |   |      +--------------------------------------+            |      |  e  |
 |     |   |         |                 ^                              |      |     |
 |     |   |         |                 |                              |      |     |
 |     |   |         |                 |  message queue               |      |     |
 |     |   |         |                 |                              |      |     |
 |     |   |         |                 |                              |      |     |
 |     |   |         |    +---------------------------+               |      |     |
 |     |   |         |    |          payload          |               |      |     |
 | log |   |         |    |  -----------------------  |               |      |utils|
 |     |   |         |    | rule,class,instance,scene |               |      |     |
 |     |   |         |    +---------------------------+               |      |     |
 |     |   |         |                          ^                     |      |     |
 |     |   |         |                           \                    |      |     |
 |     |   |         |                            \                   |      |     |
 |     |   |         v                             \                  |      |     |
 |     |   |  +-----------------+            +---------------------+  |      |     |
 |     |   |  |     clipscpp    |            |     datachannel     |  |      |     |
 |     |   |  +-----------------+            | ------------------- |  |      |     |
 |     |   |  |   clips core    |            | device, cloud, app  |  |      |     |
 |     |   |  +-----------------+            +---------------------+  |      |     |
 |     |   |                                    ^            ^        |      |     |
 |     |   |                                   /              \       |      |     |
 |     |   |                                  /                \      |      |     |
 |     |   |                                 /                  \     v      |     |
 |     | +-----------------+        +----------------+  +---------------+    |     |
 |     | |    sql table    |        | device manager |  |  http handler |    |     |
 |     | |                 |        |       hu       |  |    (8899)     |    |     |
 |     | +-----------------+        +----------------+  +---------------+    |     |
 +-----+                              ^   ^    ^   ^                         +-----+
                  not                 |   |    |   |
             ----\--------------------+   |    |   +----------------------
            /             +---------------+    +-------------+            \
           /              |                                  |             \
 +--------------+         |      +---------------------+     |        +------------+
 |              |    ocf  |      |      ipc system     |     | ocf    |            |
 | dev profiles |         |      |        process      |     |        | ocf device |
 |              |   coap  |      +---------------------+     | coap   |            |
 +--------------+         |                 |                |        +------------+
                          |                 |                |
                 +----------------+         |         +--------------+
                 |  konke bridge  |   <-----+----->   |  hue bridge  |
                 +----------------+    fork    fork   +--------------+
                         |                                     |
                         |  tcp                                |  alljoy
                         |                                     |
                 +---------------+                     +---------------+
                 | konke gateway |                     |  hue gateway  |
                 +---------------+                     +---------------+
                         |                                     |
                         |  zigbee                      unkown |
                         |                                     |
               *****************                           *********
         ******        sos      ******                  ***         ***
      ***  lightctrol          red    ***               *    light    *
      *              smartplug          *               ***         ***
      ***   scenectrol    envsonsor   ***                  *********
         ******                 ******
               *****************
```

# 日志系统

规则引擎重要的一个功能就是将用户配置的规则文本转换为clp脚本, 而调试clp脚本需要好的工具
, 将clp脚本的调试日志输出到统一的日志系统中是必然要做的.

首先使用固定`buffer`作为日志缓冲, 即`ring buffer`, 所有模块均可往这buffer中写入, 日志
的最终输出采用多通道方式, 可以需要依次输出到终端, 文件或者网络.


```
                                                                  +-----------------------+
        +-------------------+     +----------------+              |      RingBuffer       |
        |  MessageHandler   |     |    DataSink    |         ---->|-----------------------|
        |-------------------|     |----------------|        /     |      mBufHead         |
        |  mMessageQueue    |     |   mRingBuffer  |-------/      |      mBufLength       |
        |  mMessageHandler  |     |   m_dataSize   | mRingBuffer  |-----------------------|
        |-------------------|     |----------------|              |  get[Write/Read]Head  |
        |   handleMessage   |     |  onDataArrive  |              |  submit[Write/Read]   |
        +-------------------+     +----------------+              +-----------------------+
                △                       △   ^
                |                       |   |
                +-----------+-----------+   +-------------------------------------------------------------------------------+
                            |                                                  +------------+                               |
                            |                                                  |            |                               |
                   +-----------------+                                         |            |                               |
         +-------> |    LogPool      |                                         v            |                               |
         |         |-----------------|  mFilterHead                      +-----------+      |                               |
         |         |   mFilterHead   |◇ -------------------------------->| LogFilter |      |                               |
         |         |-----------------|                                   |-----------|      |                               |
         |         |   attachFilter  |                                   |  m_next   |◇ ----+                               |
         |         |   detachFilter  |                                   |-----------|  m_next                              |
         |         |                 |                                   | pushBlock |                                      |
         |         |   onDataArrive  |                                   +-----------+                                      |
         |         |   receiveData   |                                     △   △   △                                        |
         |         |                 |                                     |   |   |                                        |
         |         |  handleMessage  |                                     |   |   |                                        |
         |         +-----------------+              +----------------------+   |   +-----------------------+                |
         |                                          |                          |                           |                |
         |                                          |                          |                           |                |
         |         +-----------+           +-----------------+        +----------------+          +-----------------+       |
         |         | LogThread |           |  ConsoleFilter  |        |   FileFilter   |          |  NetworkFilter  |       |
         |         |-----------|           |-----------------|        |----------------|          |-----------------|       |
         |         |           |           |                 |        |                |          |                 |       |
         | Create  |-----------|           |    pushBlock    |        |   pushBlock    |          |    pushBlock    |       |
         +---------|   run     |           +-----------------+        +----------------+          +-----------------+       |
                   +-----------+                                                                                            |
                         |                                                                                                  |
                         |                                                                                                  |
                         |                                                                                                  |
                         ▽                                                                                                  |
              +----------------------+                   +---------------+                                                  |
              |    MessageLooper     |                   |    Logger     |                                                  |
              |----------------------|                   |---------------|    mDataSink                                     |
              |       mMsgQueue      |                   |   mDataSink   |◇ ------------------------------------------------+
              |----------------------|                   |---------------|
              |        run           |                   |     log       |
              +----------------------+                   +---------------+

```

# 消息系统

消息系统是Homebrain主程序的血脉, 贯穿与整个引擎系统中. 也是程序设计中解耦合的重要方法.

```
            +--------------------------------------------------------------------------------------------------+
            |                                                                                                  |
            |                                                                                                  |
            |                                                                       +-----------------+        |
            |                                                                       |     Message     |        |
            |                                                                    -->|-----------------|        |
            |                                       +----------------+          /   |      what       |        |
            |                                       |  MessageQueue  |         /    |    arg[1|2|3]   | target |
            |                                 ----->|----------------|        /     |     target      |◇ ------+
            |                                /      |    mMessages   |-------/      |-----------------|
            |                               /       |----------------| mMessage     |     obtain      |
            |                              /        | enqueueMessage |              |    recycle      |
            v                             /         |  removeMessage |              |      next       |
 +--------------------+                  /          |   nextMessage  |              +-----------------+
 |   MessageHandler   |                 /           +----------------+
 |--------------------| mMessageQueue  /
 |   mMessageQueue    |---------------/
 |   mMessageHandler  |◆ -----------------+
 |--------------------|  mMessageHandler  |
 |  dispatchMessage   |                   |                +------------+
 | sendMessage[Delay] |                   |        +-----▷ |   Thread   |
 |   handleMessage    |                   |        |       |------------|
 +--------------------+                   |        |       |    PID     |
           △                              |        |       |------------|
           |                              |        |       |    run     |
           |                              |        |       +------------+
           |                              v        |
 +-------------------+              +----------------------+
 | RuleEngineHandler |              | MessageHandlerThread |
 |-------------------|              |----------------------|
 |   handleMessage   |              |    mMessageQueue     |      +--------------------------------------+
 +-------------------+              |----------------------|      |while(true)                           |
                                    |        run           |----->|   msg = mMessageQueue->nextMessage() |
                                    +----------------------+      |   msg->target->dispatchMessage()     |
                                               △                  +--------------------------------------+
 +-------------------+                         |
 | RuleEngineThread  |                         |
 |-------------------|-------------------------+
 |                   |
 |-------------------|
 |       run         |
 +-------------------+

```

# 引擎设计

采用专家系统工具Clips进行前向推导, clips是工具也是一种语言, 提供c接口, 同时也支持脚本
独立运行, 所以规则引擎的设计其实就转变为clips脚本设计.

## 模型

### 设备模型

```{.json .numberLines startFrom="1"}
{
    "devicetype": "oic.d.hue_bri_light",
    "supertype": "oic.d.light",
    "devicename": "飞利浦灯",
    "manufacture": "Philips",
    "iconid": "ic_default_light",
    "version": "0.9.2",
    "profile": {
        "OnlineState": {
            "tag": "在线状态",
            "write": "F",
            "read": "T",
            "type": "enum",
            "range": {
                "0": "离线",
                "1": "在线"
            }
        },
        "value": {
            "rt": "oic.r.switch.binary",
            "iconid": "ic_default_switch",
            "tag": "开关",
            "read": "T",
            "write": "T",
            "type": "enum",
            "range": {
                "0": "关闭",
                "1": "开启"
            },
            "unit": ""
        },
        "brightness": {
            "rt": "oic.r.light.brightness",
            "tag": "亮度",
            "read": "T",
            "write": "T",
            "type": "int",
            "range": "v >= 0 and v <= 100",
            "unit": ""
        }
    }
}

```

### 规则模型

``` {.json .numberLines startFrom="1"}
{
    "ruleName":"example",
    "ruleId":"123456",
    "description":"this is a example for rule definition",
    "trigger":{
        "triggerType":"auto|manual",
        "switch":{
            "enabled":"on",
            "timeCondition":"on",
            "deviceCondition ":"on",
            "notify":"on",
            "manual":"on"
        }
    },
    "conditions":{
        "conditionLogic":"and",
        "timeCondition":[
        {
            "year":"2018",
            "month":"10",
            "day":"10|13|17",
            "hour":"every",
            "minute":"every",
            "second":"1"
        }
        ],
        "deviceCondition":{
            "deviceLogic":"or",
            "deviceStatus":[
            {
                "deviceId":"0007A895C8A7",
                "propName":"CurrentTemperature",
                "propValue":"v>50"
            },
            {
                "deviceId":"DC330D799327",
                "propName":"onOffLight",
                "propValue":"v==true"
            }
            ]
        }
    },
    "actions":{
        "execScene":"",
        "notify":{
            "title": "xxx",
            "message":"Girlfriend Birthday!"
        },
        "deviceControl":[
        {
            "deviceId":"0007A895C7C7",
            "propName":"CurrentTemperature",
            "propValue":"50"
        },
        {
            "deviceId":"DC330D79932A",
            "propName":"onOffLight",
            "propValue":"true"
        }
        ],
        "manualRuleId":[
            "1528374365.417.48775",
            "1528424679.929.67961"
        ]
    }
}
```

## CLP语言

```
                              ruleID
                                ^                   -----> comment, here we want is rule name.
                                |                  /
                                |                 /                    -----> MultiSlot
                  (defrule rul-0001 "this is an example"              /
        +--------   (and                                             /
        |             (and                   -------------------------------------------------------
        |               ?fct_t1 <- (datetime ?clock ?year ?month ?day ?hour ?minute ?second $?others) ---+
        |      1        (test (= ?year 2018))                                                            |  Condition: Fact
        |    layer      (test (= ?month 06))                                                             | (not use Template)
        |               (test (or (= ?day 20) (= ?day 21) (= ?day 22) ))                              ---+
        |             )                                    -----------\
        |             (or                                              \------> Cell
LHSNode |               (object (is-a TempSensor)                                                     ---+
        |                 (ID ?id &:(eq ?id ins-0007A895C8A7))                                           |
        |      2          (CurrentTemperature ?CurrentTemperature &:(> ?CurrentTemperature 50))          |
        |    layer      )                                                                                |
        |               (object (is-a Light)                                                             | Condition: Instance
        |                 (ID ?id &:(eq ?id ins-DC330D799327))          /-----> Cell                     |
        |                 (onOffLight ?onOffLight &:(= ?onOffLight 1)) /                                 |
        |               )       \                   -------------------                               ---+
        |             )          \                           +--> timeout-ms
        +--------   )             --------> SlotPoint        |  +--> retry-count
                   =>                                        |  |
        +--------   (bind ?c (create-rule-context rul-0001 5000 5))   ------> create the rule context
        |           (if (eq ?c FALSE) then (return))
        |           (send ?c act-control ins-0007A895C7C7 CurrentTemperature 50)   ---> action: device control
RHSNode |           (send ?c act-control ins-DC330D79932A onOffLight 1)
        |           (send ?c act-notify 10000001 "tellYou" "Girlfriend Birthday")  ---> action: message notify
        |           (switch-scene room1 commehome)                                 ---> action: switch scene
        +---------  (send ?c act-scene rul-1000)                                   ---> action: active manual rule
                  )

------------------------------------------------------------------------------------------------------------------------------

                                                     /---> DEVICE is superclass
          + --------------  (defclass SmogAlarm     /
          |                   (is-a DEVICE) -------/
          |             /---- (role concrete) (pattern-match reactive) ----> can triggered by rule
  Class   |            v      (slot OnlineState (type NUMBER) (allowed-numbers 2 1))
          | (not abstract)    (slot PowerOnOff (type INTEGER) (allowed-numbers 2 1))
          |                   (slot SecurityLevel (type SYMBOL) (allowed-symbols low normal high))
          |                   (slot Battery (type NUMBER) (allowed-numbers 2 1))
          +---------------- )                        /
                                                    /
                                                   v
                                          (INTEGER or FLOAT)
```

## RulePayload设计

```
LHSNode Tree

                                              *********          "and": condition logic
                                           *** LHSNode  **
                                           *             *
                                           ***  "and"  ***
                                         /    *********    \
                                        /         |         \
                             child     /          |          \   child
                                      /      cond |           \
                                     /            |            \
                                    /             |             \
                                   /              |              \
                                  /               |               \
                                 /                |                \
                      *********            +-------------+            *********
                   *** LHSNode ***         |  Condition  |         *** LHSNode ***
                   *             *         |             |         *             *
                   ***  "or"   ***         |    "and"    |         ***   "or"  ***
                   /  *********  \         +-------------+            *********
                  /               \
                 /                 \            "and": slot logic, only support "and"
                / cond         cond \
               /                     \                                                +---------------------------+
              /                       \                                               |          +------+         |
      +-------------+           +-------------+                                       |          |      |         |
      |  Condition  |           |  Condition  |                                     Fact    Instance    |         |
      |             |           |             |                                       \        /        |         |
      |   "and"     |           |    "and"    |                                        \ type /         |         |
      +-------------+           +-------------+                                         \    /          v         v
                                       |                                               Condition <---> Device | TimeEvent
                                       |                                               SlotPoint <---> Property
                                       |
                -----------+-----------+-----------+---------------
                           |                       |
                           |                       |
                           v                       v
                    +-------------+         +-------------+     "&": cell logic
                    |  SlotPoint  |         |  SlotPoint  |     "|": cell logic
                    |             |         |             |     "~"
                    |    "&"      |         |     "|"     |
                    +-------------+         +-------------+
                           |                       |
                           |                       |
                           v                       v
                 +-------------------+    +-------------------+
                 |       Cell        |    |       Cell        |    Compare Symbol: "=, >, <, >=, <=, <>"
                 |                   |    |                   |    Connective Symbol: "&, |, ~"
                 |(v > 10) & (v < 20)|    |(v = 10) | (v > 20)|
                 +-------------------+    +-------------------+

```


## CLP设计

```
                                                                  (auto)     (manual)
                                          device                 property
                      profile  rule   online offline             changed      scene
               TestCase --------------------------------------------------------------------------------------------------->
                         |       |       |      |        ║          |           |
                         |       |       |      |    mainHandler    |           |
            mainHander() |       |       |      |        ║          |           |
                v        v       v       v      v        ▼          v           v
   MainThread ------------------------------------------------------------------------------------------------------------->
                 \    profile  rule      \      \          ║        \           \
                  \      json    json     \      \     ruleHandler   \           \
                   \        \      \       |      |        ║          |           |
                    \        \      \      |      |        ▼          |           |
                     \        \      \     v  T   v     T         T   v     T     v   T         T         T         T
      RuleThread     --------------+----------+---------+---------+---+-----+---------+---------+---------+---------+----->
                      ^            T      add |  del    |         |   \     |     /   |         |         |         |
                  ruleHander()     |          |         |         |    \    |    /    |         |         |         |
                                   |          |         |         |     |   |   |     |         |         |         |
                                   +--\       +--\      +--\      +--\  v   v   v  /--+      /--+      /--+      /--+
                                       \          \         \         \+---------+/         /         /         /
 T: timer (default 1s)                  \          \         \         | refresh |         /         /         /
                                         ----------------------------->|         |<----------------------------
                                                                       |   run   |
                                                                       +---------+
                                                                            |               +--------+
                (LHS)                                                       |         ----▷ |  USER  |
             +--------------------------------------------------------------+        /      +--------+
   salience  |                                                                      /            △
             |                        +-------------------+                        /             |
             v                        |   RuleContext     |                       /              |
        +--------+                    |-------------------|                      /              /
    +---| Rule-1 |                    |   rule-id         |                     /              /
    |   +--------+                    |   timeout-ms      |           +---------+       +-----------+        +----------------+
    |                                 |   retry-count     |---------▷ | Context |       |  DEVICE   |        |  SmogAlarm     |
    +-> +--------+                    |   current-try     |           |         |       |-----------|        |----------------|
    +---| Rule-2 |                    |   start-time      |           +---------+       |   ID      |◁ ------|   OnlineState  |
    |   +--------+                    |   act-error       |                             |   UUID    |        |   PowerOnOff   |
    |                                 |   unanswer-list   |                             |   Class   |        |   Battery      |
    +-> +--------+                    |   response-rules  |                             +-----------+        +----------------+
    +---| Rule-3 |                    |-------------------|
    |   +--------+                    |   try-again       |   由于效率问题, 规则超时设计已经不再使用, 给性能带来负担
    |                                 |   unanswer-count  |
    +-> +--------+                    |   act-control     |
        | Rule-4 |                    |   act-notify      |
        +--------+                    |   act-scene       |
            |                         +-------------------+
            |                                      +-------------------------+----------------------+------------+
            |                                      |                         |                      |            |
            |          Action                      |                         |                      |            v
            +-------------------------->  act-control ------------> act-notify -----------> act-scene   |   send-message
            |  (RHS)                          |  ^                      |   ^                   /       |        |
            |               act-step-control  |  | true                 |   | true             /        |        | success
            |                                 |  |                      |   |                 v                  |   or
            |                                 |  +------>  make-rule    |   +------>  make-rule                  |  fail
            |                                 |  | false  (response)    |   | false  (response)                  |
            |                                 v  |                      v   |                                    v
            |    RuleEngineService  -------------------------------------------------------------------------------------->
            |       ( HB )                   ins-push                   txt-push                              msg-push
            |                                ^    ^
            |                                |    |               services
            |                                |    |             +--------------------------------------------------+
    Action  +------> onEnter ----------------+    |             |                                                  |
            |                             ^       |             ◇                                                  |
            |                             |       |     +-------------------+                                      v
            +------> onExit --------------+-------+     |   SceneContext    |                              +----------------+
            |                  ^          |             | ----------------- |                              |  (MS)service   |
            |                  |          |             |     stime         |                              |--------------- |
            |                  |          |             |     where         |   +--> enter     start <--+  |     master     |
            v                  |           \            |     target        |   |                       |  |    name:skey   |
       micro service           |            \           |     action        ----+--> story   running <--+---     state      |
        (onStory)              |             \          |     services      |   |                       |  |     ntime      |
            |                  |              \         |     zombies       |   +--> exit       stop <--+  |     saves      |
            |                  |               \        |-------------------|                              |      args      |
    +-------+------+           |                \       |    add-service    |                              |                |
    |              |           |                 \      |    del-service    |                              +----------------+
    v              v           |                 |      +-------------------+
gradligth       autolight      |2              3 |
                               |                 |              services: 存放正在运行的服务
        ^     ^                |                 |              zombies: 存放已经结束的服务
        |     |                |                 |              (以上两个配合实现启动服务依赖先后顺序)
        |     |  1   +-----------+  swtich    +----------+
        |     +------| gotosleep | ---------> |  wakeup  |
        |   stop     +-----------+    to      +----------+
        |                                             |
        | start                            4          |
        +---------------------------------------------+
```

## 引擎UML

```
                                          +---------------+
 +--------------+                         |   Condition   |                 +------------+               +----------+
 |    Action    |                         |---------------|                 | SlotPoint  |               |  _Cell_  |
 |--------------|                         |   mSlotLogic  |          -----> |------------|        ------>|----------|
 |  mType       |                         |    mType      |         /       | mCellLogic |       /       | nSymbol  |
 |  mCall       | ^                       |    mCls       |        /        | mSlotName  |      /        | nValue   |
 |  mID         |  \                      |    mID        |       /         | mCells     |◆ ---/mCells   +----------+
 |  mSlotName   |   \                     |   mSlots      |◆ ----/mSlots    +------------+
 |  mSlotValue  |    -------+             +---------------+
 +--------------+    1:n    | mActions       ^                  +-----------+     mCondLogic: and/or/not
                            |                |                  |           |     mSlotLogic: and             +------------+
                          +------------+     |          +--------------+    |     mCellLogic: &,|,~           |  SlotInfo  |
                          |  RHSNode   |     |          |   LHSNode    |    |     nSymbol: =,>,<,>=,<=,<>     |------------|
                          | -----------|     |          |--------------|    |                                 |   nName    |
                          |  mActions  |     |          |  mCondLogic  |   /mChildren                         |   nValue   |
                          +------------+     +--------◇ |  mConditions |  /                                   +------------+
                                   ^         mConditions|  mChildren   |-/  +---------------+                         ^
                                   |                    +--------------+    |  RulePayload  |                  mSlots |  1:n
                                   +----------\                ^            |---------------|                         |
                                               \               |            |  mRuleName    |                         ◆
                              +-------------+   \              |            |  mRuleID      |            +-------------------+
                              | DataChannel |    \             |     mLHS   |  mVersion     |            |  InstancePayload  |
                              |-------------|     \            +----------◇ |  mLHS         |            |-------------------|
                              |             |      -----------------------◇ |  mRHS         |         /--|      mInsName     |
                              |send(payload)|                        mLHS   |---------------|        /   |      mClsName     |
                              +-------------+                               |  toString()   |       /    |      mSlots       |
                                  △ ^ ^ △                                   +---------------+      /     +-------------------+
                                  | | | |                                             |           /
                                  | | | |                                             |          |
                  +---------------+ | | +----------------+                            |          |
                  |                 | |                  |                            |          |
                  |                 | |                  |                            ▽          |
         +------------------+       | |     +--------------------------+        +----------+ ◁ --+          +-----------------+
         | RuleDataChannel  |       | |     |    DeviceDataChannel     |        | Payload  |                |  ClassPayload   |
mCloudMgr|------------------|       | |     | ------------------------ |        |----------| ◁ -------------|-----------------|
  +----◆ |    mCloudMgr     |       | |     |       mDeviceMgr         |        |  type()  |                |   classname     |
  |      |    mHandler      |       | |     |       mHandler           |        +----------+                |   superclass    |
  |      +------------------+       | |     |--------------------------|             |             mSlots   |   version       |
  |               △                 | |     |    onStateChanged()      |             |                +---◇ |    mSlots       |
  |               |                 | |     |    onPropertyChanged()   |-------------+            1:n |     |-----------------|
  |               |                 | |     |         send()           |                              |     |   toString()    |
  |    +------------------------+   | |     +--------------------------+◆ ---------------------+      |     +-----------------+
  |    |  ElinkRuleDataChannel  |   | |                △          ◆        mHandler            |      |
  |    |------------------------|   | |                |          |                            |      |
  |    |                        |   | |                |          +----------+                 |      |
  |    |      onRuleSync()      |   | |   +-------------------------+        |                 |      |     +---------------+
  |    |        send()          |   | |   | ElinkDeviceDataChannel  |        |                 |      |     |     Slot      |
  |    +------------------------+   | |   |-------------------------|        |                 |      +---> |---------------|
  |                                 | |   |      onProfileSync()    |        |                 |            |    mType      |
  |                                 | |   +-------------------------+        |                 |            |    mName      |
  |                                 | |                                      |                 |            |   mMin/mMax   |
  |    +-------------------------+  | |                           mDeviceMgr |                 |            |   mAllowList  |
  |    |     CloudManager        |  | |                                      v                 |            |---------------|
  |    |-------------------------|  | |   +----------------------------------------+           |            |   toString()  |
  +--->|                         |  | |   |             DeviceManger               |           |            +---------------+
       | registRuleSyncCallback  |  | |   | -------------------------------------- |           |
       |                         |  | |   |                                        |           |
       +-------------------------+  | |   |   registDeviceStateChangedCallback     |           |
                                    | |   |   registDevicePropertyChangedCallback  |           |
                                    | |   |   registDeviceProfileSyncCallback      |           |
+-----------------------------------+ |   |        setProperty                     |           |
| +-----------------------------------+   +----------------------------------------+           |
| |                                                                                            |
| |                    +----------------------------+        +----------------+                |
| |             +----▷ |  MessageHandler::Callback  | <----◇ | MessageHandler | ◁ ---------+   |
| |             |      +----------------------------+        |----------------|            |   |
| |             |                                            |   mCallback    |            |   |
| |             |                                            +----------------+            |   |
| |  +----------------------+                                                              |   |
| |  |  RuleEngineService   |                                                              |   v
| |  |----------------------|   mUrgentHandler                                    +--------------------+
| |  |     mUrgentHandler   |◇ -------------------------------------------------->|  RuleEventHandler  |
| |  |                      |    mCore         +----------------------+    ------>|--------------------|
| |  |  mCore/mCoreUrgent   | ◆ -------------> |    RuleEngineCore    |   /       |    handleMessage   |
| |  |     mServerRoot      |                  |----------------------|  /        +--------------------+
| +--|     mDeviceChannel   |          mEnv    |    mHandler          | /mHandler           |
+----|     mRuleChannel     |        +-------◆ |    mEnv              |◆                    |
     | mOfflineInsesCalled  |        |         |----------------------|                     |
     |----------------------|        |         |    driver()          |                     |
     |    callInstancePush  |        |         |                      |            +------------------+
     |    callMessagePush   |        |         |   handleTimer        |            | RuleEventThread  |
     |    setDeviceChannel  |        |         |   handleClassSync    |            |------------------|
     |    setRuleChannel    |        |         |   handleRuleSync     |            |   mMessageQueue  |
     |     handleMessage    |        |         |   handleInstanceAdd  |            +------------------+
     +----------------------+        |         |   handleInstanceDel  |                     △
                                     v         |   handleInstancePut  |                     |
                           +---------------+   +----------------------+                     |
                           |  Environment  |                                                |
                           |               |--\                                   +---------------------+
                           +---------------+   \  CLP                             |   RuleUrgentThread  |
                            Clipscpp            ------> clips6.30                 |---------------------|
                                                                                  |     mService        |
                                                                                  +---------------------+
                            +----------------------+
                            |     HBDatabase       |
                            |  (template class)    |
           /--------------◆ |----------------------|
          /        mDB      | mAutoCloseInterval   |
         /                  | mDBPath   mMutex     |
        |                   |----------------------|
        |                   |   updateOrInsert     |
        v                   |   deleteBy           |                       +--------------------+
+--------------+            |   queryBy            |                       | DeviceProfileTable |
|SQLiteDatabase|            |                      |                       |                    |
|              |            +----------------------+                       +--------------------+
|              |                    |                                               |
+--------------+                    |                                               |
        ^                        mainDB()              +---------------------+      |      +------------------+
        |                                              | GatewayConnectTable |      |      |  OCFDeviceTable  |
        | mDB                                          |                     |      |      |                  |
        |                                              +---------------------+      |      +------------------+
        ◆                                                        |                  |               |
 +------------------+                                            |                  |               |
 |   SQLiteTable    | ◁ ------------------------------------------------------------+---------------+
 |------------------|                      |                              |                         |
 |                  |                      |                              |                         |
 |                  |              +-----------------+          +------------------+         +-----------------+
 +------------------+              | RuleEngineTable |          | SceneEngineTable |         |  HBGlobalTable  |
                                   |                 |          |                  |         |                 |
                                   +-----------------+          +------------------+         +-----------------+
```

# HttpHandler

自行百度craw, 一个非常快速和易于使用的C++微型web框架.


# 调试系统

## 系统监控(Python)

python写的monitor工具, 设计了很多调试规则的选项, 具体功能:

- 显示系统信息

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/monitor_systeminfo.png)

- 设置各模块的日志级别, 配置调试clp的各个变量信息等

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/monitor_loglevel.png)

- 将日志输出到monitor端,可以进行过滤, 可配置rule的调试选项等

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/monitor_logout.png)

- 显示当前系统的中的所有设备实例, 根据类型可以查询, 设置设备属性, 或直接设置到引擎中.

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/monitor_devicecontrol.png)

- 新建,查看,编辑,删除规则(只有查看功能有效, 其他功能已屏蔽, 可以使用webpage完成操作).

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/monitor_rule_crud.png)

- 执行一些定义好的模式, 或者直接发送"事实"触发规则

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/monitor_rule_scene.png)

## 规则配置(JS)

基于restapi实现规则的配置与执行, 具体功能:

- 网关(桥:hue, konke)配置管理

网关(桥)列表:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_gateway_list.png)

添加新的网关:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_gateway_add.png)

- 设备的发现及控制管理

设备发现:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_device_discover.png)

设备列表及控制:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_devicecontrol.png)

- 规则的编辑及管理

规则列表:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_rule_list.png)


规则配置:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_rule_edit.png)


- 场景的编辑及管理

场景列表:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_scene_list.png)

场景微服务配置:

![](https://raw.githubusercontent.com/qrsforever/asset_blog_post/master/Note/IOT/www_scene_ms.png)
