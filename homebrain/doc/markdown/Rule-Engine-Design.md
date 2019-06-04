---

title: IOT之规则引擎
date: 2018-05-27 21:04:19
tags: [ IOT, DrawIt ]
categories: [ Note ]

---


<!-- vim-markdown-toc GFM -->

* [Design](#design)
    * [Framework](#framework)
    * [Class](#class)
* [Develop](#develop)
    * [Module Tasks](#module-tasks)
    * [Module Sample](#module-sample)
* [TODO](#todo)
    * [Test Supported](#test-supported)

<!-- vim-markdown-toc -->

Design
======

Framework
---------

```
v0.0.1
                             ╔═════════════════╦════════════════════════════════════════╗
   *******      |            ║                 ║                                        ║
*** Cloud ***   |            ║                 ║ Log / MQ / Json / DataChannel / Time   ║
   *******      |     Rule   ║                 ║                                        ║
      |         |  --------> ║     Rules       ╠════════════════════════════════════════╣
      |         |            ║    Assemble     ║                                        ║
      +-------> |            ║                 ║         Rule Engine Driver             ║
    json rule   |            ║                 ║                                        ║
                |            ║                 ║                                        ║
                |            ╠════════════╦════╩══════╦═╦═══════════════════════════════╣
          +-----------+      ║            ║           ║ ║                               ║
          |           |      ║   Global   ║  Devices  ║ ║      Clips C++ Interface      ║
          |  convert  |      ║            ║           ║ ║                               ║
          |           |      ╠═══════════CLP══════════╣ ╠═══════════════════════════════╣
          +-----------+      ║            ║           ║ ║                               ║
   1.json rule parse         ║    Rules   ║   Utils   ║ ║         Clips Core            ║
   2.map profile/alias       ║            ║           ║ ║                               ║
                             ╚════════════╩═══════════╩═╩═══════════════════════════════╝

```

说明:

1. 云端下发的json规则, 需要转换成Rule对象(这个转换不属于规则引擎模块, 只提供Rule类头文件), 传递给rule translate模块.
2. 云端下发的json规则本地化存储不属于规则引擎模块负责, 但传给rule translate模块翻译后的clip规则文件, 规则引擎模块负责存储该文件.

3. 触发规则引擎的事件(属性变化等)需要胡老师提供注册回调的API
4. 规则引擎触发的动作(改变属性等)需要胡老师提供相关调用的API (同步异步)


Class
-----

### Message

```
v0.0.1
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
说明:

1. 支持延时消息, 可以间接实现Timer
2. 所有与RuleEgineDriver交互的事件尽可能使用消息驱动, 比如: PropertyEvent, RuleEvent, TimerEvent, SystemEvent

what | arg1 | arg2 | obj | comment
:----|:----:|:----:|:---:|:----
PropertyEvent | REPORT | undef | undef | 属性上报事件
| ERROR  | undef | undef | 属性错误事件
RuleEvent |  ADD | undef | undef | 规则添加事件
|  UPDATE | undef | undef | 规则更新事件
TimerEvent | undef | undef | undef | 目前只用来做定时器
SystemEvent | undef | undef | undef | 系统消息

3. RuleEngine需要单独的线程处理事件, 任何Dispatch出去的消息执行过程不得有阻塞

### Log

```
v0.0.1
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
说明:
1. 日志采用循环Buffer输入输出, 在独立的日志线程处理buffer中日志
2. 日志可以实现多个后端输出如 Console/File/Network等

### Rule Engine Driver

```
v0.0.1

            +--------------+           +---------------+
            | DeviceShadow |           |  PropertySlot |
            |--------------|     +---->|---------------|
            |   mClsName   |     |     |    mName      |
            |   mSlots     |◇ ---+     |    mType      |
            |--------------| mSlots    |---------------|
            |              |           |               |
            +--------------+           +---------------+
                   ^                                                        +----------------------+
                   |                                                  ----->|  RuleEngineHandler   |
                   |                                                 /      |----------------------|
                   |                                                /  /--◇ |        mDriver       |
                   |          +-----------------------+            /  /     |----------------------|
                   |          |   RuleEngineDriver    |           /  /      |   handleMessage      |
                   | 1:n      |-----------------------|          /  /       |                      |
                   +--------◇ |       mShadows        | mHandler/  /        |  handleRuleEvent     |
               mShadows       |       mHandler        |◇ ------/  /         |  handlePropertyEvent |
                              |       mClips          |<---------/          |  handleTimerEvent    |
                              |-----------------------|         mDriver     +----------------------+
                              |  [setup/start]Clips   |                           |     |
                              |                       |                           |     |
                              |    doPropertyEvent    |                    -------+     +------\
                              |    doTimerEvent       |                   /                     \
                              |    doRuleEvent        |                  /                       \
                              +-----------------------+                 /                         \
                                                           +---------------------+        +----------------------+
                                                           |                     |        |                      |
                                                           |DeviceManagerWrapper |        |   CloudRuleCovert    |
                                                           |                     |        |                      |
                                                           +---------------------+        +----------------------+


```
说明:


Develop
=======

Module Tasks
------------

1. Common Modules: Message Queue and Message Handle, Log, Timer

2. C++ Clips Interface

3. Rule Engine Manage

4. CLP files: Global, Devices, Utils, Rules

5. Rules Translate

Module Sample
-------------



TODO
====

Test Supported
--------------

Items | Support | Sample
:-------- | :----------: | :------
math: =,!= | yes | not impl
math: +,-,\*,/ | yes | not impl
math: >,< | yes | not impl
logic: and,or | yes | not impl
timer | no | not impl
update trigger | yes | not impl
state trigger | no | not impl
