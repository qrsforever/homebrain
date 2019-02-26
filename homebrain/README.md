---

title: HomeBrain工程介绍
date: 2018-05-29 21:02:26
tags: [ IOT ]
categories: [ LOCAL ]

---

<!-- vim-markdown-toc GFM -->

* [编译](#编译)
    * [Scon编译](#scon编译)
    * [ndk-build编译(控客Demo)](#ndk-build编译控客demo)
    * [Android原生平台编译](#android原生平台编译)
* [输出](#输出)
* [代码提交](#代码提交)
* [HomeBrain目录](#homebrain目录)

<!-- vim-markdown-toc -->

编译
====

Scon编译
--------

**如果遇到奇怪的编译问题, 多半是中间产物造成, 请先make clean**

1. (可省) `git clone https://github.com/ARMmbed/mbedtls.git extlibs/mbedtls/mbedtls -b mbedtls-2.4.2`

2. (可省) `git clone https://github.com/01org/tinycbor.git extlibs/tinycbor/tinycbor -b v0.4.1`

3. (可省) `修改homebrain/build/${PLATFORM}/build.sh`

4. make (默认mstar938) 或 make ${PLATFORM}, 其中 `PLATFORM` 已支持linux, mstar938, mstart648

注意:

- *针对mstar648/mstart938平台只需要执行第4步即可, 前3步已经内置脚本中*
- *${TOP_DIR}/briging目录对android系统默认不编译, 需要模块负责人根据实际情况修改*

- 现阶段的编译不含Secure相关模块, 后续根据需要添加
- 不要直接删除out目录重新编译, 不然会遇到不编译so或者sample程序, 最好执行make clean彻底清楚

- 编译linux: 依赖see`TOP_DIR/README-building-and-running-remote-access-sample.txt`
  另外：　`sudo apt-get install libcurl4-gnutils-dev`

ndk-build编译(控客Demo)
-----------------------
1. 在`jni/Application.mk`指定STL, 如:

```:-
   APP_ABI := arm64-v8a
   APP_STL := gnustl_shared
   NDK_TOOLCHAIN_VERSION := 4.9
   APP_CPPFLAGS += -frtti
   APP_CPPFLAGS := -frtti -fexceptions
```

2. 在`jni/Android.mk`指定编译目标(同Android)

3. ndk-build


Android原生平台编译
-------------------

1. 在`jni/Android.mk`指定使用的STL, 目前Android并没有完整的设置

```
  LOCAL_CXX_STL := ndk

  prebuilt_stdcxx_PATH := $(ANDROID_BUILD_TOP)/prebuilts/ndk/current/sources/cxx-stl/gnu-libstdc++
  LOCAL_C_INCLUDES := \
                      $(prebuilt_stdcxx_PATH)/4.9/include \
                      $(prebuilt_stdcxx_PATH)/4.9/libs/$(TARGET_CPU_ABI)/include
  LOCAL_CFLAGS += -fPIE -rdynamic -Wno-error=non-virtual-dtor -D_CTYPE_N=0x04
```

2. mm

注意:

- *原生Android平台使用的c++库, 是android中的libc++.so, 而控客给的库是基于libgnustl_shared的*
- *938平台`NDK_TOOLCHAIN_VERSION`连接的4.7版本, 编译使用的而是4.9, 所以头文件指定的是4.9*
- *在`bionic/libc/include/ctype.h`没有定义`_CTYPE_N`, 所以`LOCAL_CFLAGS`定义.*
- *参考`build/core/cxx_stl_setup.mk`*


编译指定目录
------------

`make DIR=homebrain/src/utils` 默认编译mstart648, 编译其他平台, `make`后面加PLATFROM
如:
  `make linux DIR=homebrain/src/utils`

输出
====

1. 最终输出的东西由各平台的`homebrain/build/${PLATFORM}/postbuild.sh`控制
2. 输出目录规范到`${TOP_DIR}/out/${PLATFORM}/root`

如下是mstar648输出样例:

```
 out/mstar648/
 |-- root
 `-- sysroot_api23_arm64        ------ prebuild.sh脚本将各平台的sysroot解压此处


out/mstar648/root
 `-- usr
     |-- bin                    ------ 目前只打包了example程序, 根据需要修改postbuild.sh
     |   |-- simpleclient
     |   `-- simpleserver
     |-- include                ------ 目前只打包了iotvity的头文件, 根据需要修改postbuild.sh
     |   |-- c_common
     |   |-- resource
     |   `-- service
     |-- iotivity.pc            ------ 编译iotivity之后package配置信息, 如何引用include/lib
     `-- lib64                  ------
         |-- libconnectivity_abstraction.so
         |-- liboc_logger_core.so
         |-- liboc_logger.so
         |-- liboc.so
         `-- liboctbstack.so
```

代码提交
========

1. git commit -s

2. git push origin HEAD:refs/for/1.3.1


HomeBrain目录
=============

```
homebrain
    |-- build
    |   |-- extlibs             ------ 编译iotvity依赖的第三方库压缩包, 包要带版本号, 供脚本处理
    |   `-- mstar648            ------ 应用平台, 目前Demo在mstar648, 里面包含一个sysroot的压缩包
    |-- examples
    |   |-- SConscript
    |   |-- simpleclient.cpp
    |   `-- simpleserver.cpp
    |-- external                ------ HomeBrain依赖的第三方SDK
    |   `-- kk_sdk
    |-- include
    |-- plugins                 ------ 桥相关的插件
    |   `-- kk_plugin
    |-- README.md
    |-- SConscript
    `-- src                     ------ HomeBrain源码
        |-- device-manager
        |-- rule-engine
        |-- SConscript
        |-- smarthome-api
        `-- utils
```

TODO HISTROY
============

1. Iotivity原始的android平台编译下载依赖太多(sdk, ndk, gradle), 且编译JAVA对我们的工程来说没有意思, 固修改为sysroot的形式.
    - `build_commom/SConscript` 添加`ANDROID_BUILD_JAVA`, 我们工程该值为0, 不去编译Java文件
    - `build_common/android/SConscript` 把原始的SDK,NDK等编译模式改为Sysroot

2. 为了加快编译, 使用TEST宏排除了unittest的编译(TEST=1编译), 全局搜索`'1' == evn.get('TEST')`可以修改还原

