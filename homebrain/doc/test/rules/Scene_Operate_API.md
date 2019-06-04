---

title: 场景操作API
date: 2018-12-29 17:50:40
tags: [Work]
categories: [Company]

---

<!-- vim-markdown-toc GFM -->

* [场景操作](#场景操作)
    * [环境变量](#环境变量)
    * [场景测试文件](#场景测试文件)
    * [数据库](#数据库)
    * [API测试](#api测试)
        * [创建场景](#创建场景)
        * [更新场景](#更新场景)
        * [删除场景](#删除场景)
        * [查询场景](#查询场景)
        * [查询场景列表](#查询场景列表)
        * [手动执行接口](#手动执行接口)

<!-- vim-markdown-toc -->

<!-- more -->

# 场景操作

## 环境变量

```sh
export ip=192.168.124.16
export port=8899
export token="token:1234567"
```

## 场景测试文件

1. 自动模式的`autotest.json`文件

```json
{
    "sceneId": "-",
    "sceneName": "autotest",
    "description": "当环境传感器流明值小于等于35则打开灯",
    "trigger": {
        "switch": {
            "enabled": "on",
            "notify": "off",
            "timeCondition": "off",
            "deviceCondition": "on",
            "manual": "on"
        },
        "triggerType": "auto|manual"
    },
    "conditions": {
        "conditionLogic": "",
        "deviceCondition": {
            "deviceStatus": [
                {
                    "propValue": "v<=35",
                    "deviceId": "00124b00-12cc-8000-d0d0",
                    "propName": "illuminance"
                }
            ],
            "deviceLogic": ""
        }
    },
    "actions": {
        "deviceControl": [
            {
                "propValue": "1",
                "deviceId": "00124b00-19a8-9215-d0d0",
                "propName": "value1"
            },
            {
                "propValue": "1",
                "deviceId": "00124b00-19a8-9215-d0d0",
                "propName": "value2"
            }
        ]
    }
}
```

2. 手动模式的`manualtest.json`文件

```json
{
    "sceneId": "-",
    "sceneName": "manualtest",
    "description": "",
    "trigger": {
        "switch": {
            "enabled": "on",
            "notify": "off",
            "timeCondition": "off",
            "deviceCondition": "off",
            "manual": "off"
        },
        "triggerType": "manual"
    },
    "actions": {
        "deviceControl": [
            {
                "propValue": "1",
                "deviceId": "00124b00-19a8-9215-d0d0",
                "propName": "value3"
            }
        ]
    }
}
```

## 数据库

1. 删除所有数据: ` adb shell sqlite3 /data/homebrain/engine.db \"delete from rules\" `

2. 查询所有数据: ` adb shell sqlite3 /data/homebrain/engine.db \"select id, name from rules\" `


## API测试

### 创建场景

- 命令: ` ret1SceneId=$(curl -v -X POST http://$ip:$port/api/familyscene/add -H $token -d @autotest.json 2>/dev/null | grep -Po 'sceneId[" :]+\K[^"]+') `

- 命令: ` ret2SceneId=$(curl -v -X POST http://$ip:$port/api/familyscene/add -H $token -d @manualtest.json 2>/dev/null | grep -Po 'sceneId[" :]+\K[^"]+') `

- 命令: ` ret3SceneId=$(curl -v -X POST http://$ip:$port/api/familyscene/add -H $token -d @manualtest.json 2>/dev/null | grep -Po 'sceneId[" :]+\K[^"]+') `

输出:

```json

{
    "requestId": "abcddcba",
    "request": "/api/familyscene/add",
    "result": {
        "resInfo": "规则添加成功",
        "sceneId": "163616",
        "ruleScript": 1
    },
    "status": 1
}

```

**ret[1-3]SceneId**是返回的场景Id


### 更新场景

- 命令: ` cat autotest.json | sed "s/\"-\"/\"$ret1SceneId\"/" | sed "s/=35/\=45/" | tee /tmp/autotest.json `

- 命令: ` curl -v -X POST http://$ip:$port/api/familyscene/modify  -H $token -d @/tmp/autotest.json `

```json
{
    "requestId": "abcddcba",
    "request": "/api/familyscene/modify",
    "result": {
        "resInfo": "规则修改成功",
        "sceneId": "163616",
        "ruleScript": 1
    },
    "status": 1
}
```

### 删除场景

命令: ` curl -v -X POST http://$ip:$port/api/familyscene/delete  -H $token -d "{\"sceneId\":\"$ret3SceneId\"}" `

```json
{
    "requestId": "abcddcba",
    "request": "/api/familyscene/delete",
    "result": {
        "resInfo": "规则删除成功",
        "sceneId": "582111",
        "ruleScript": 1
    },
    "status": 1
}
```

### 查询场景

命令: ` curl -v -X POST http://$ip:$port/api/familyscene/query   -H $token -d "{\"sceneId\":\"$ret1SceneId\"}" `

```json
{
    "requestId": "abcddcba",
    "request": "/api/familyscene/query",
    "result": {
        "resInfo": "规则查询成功",
        "scene": {
            "sceneId": "163616",
            "sceneName": "autotest",
            "description": "当环境传感器监控流明值小于等于35则打开灯",
            "trigger": {
                "switch": {
                    "enabled": "on",
                    "notify": "off",
                    "timeCondition": "off",
                    "deviceCondition": "on",
                    "manual": "on"
                },
                "triggerType": "auto|manual"
            },
            "conditions": {
                "conditionLogic": "",
                "deviceCondition": {
                    "deviceStatus": [
                        {
                            "propValue": "v<=45",
                            "deviceId": "00124b00-12cc-8000-d0d0",
                            "propName": "illuminance"
                        }
                    ],
                    "deviceLogic": ""
                }
            },
            "actions": {
                "deviceControl": [
                    {
                        "propValue": "1",
                        "deviceId": "00124b00-19a8-9215-d0d0",
                        "propName": "value1"
                    },
                    {
                        "propValue": "1",
                        "deviceId": "00124b00-19a8-9215-d0d0",
                        "propName": "value2"
                    }
                ]
            }
        }
    },
    "status": 1
}

```

### 查询场景列表

命令: ` curl -v -X POST http://$ip:$port/api/familyscene/listall -H $token

```json
{
    "requestId": "abcddcba",
    "request": "/api/familyscene/listall",
    "result": {
        "resInfo": "规则列表查询成功",
        "data": [
            {
                "triggerEnabled": "1",
                "manualEnabled": "1",
                "sceneName": "autotest",
                "sceneId": "163616",
                "description": "当环境传感器监控流明值小于等于35则打开灯"
            },
            {
                "triggerEnabled": "1",
                "manualEnabled": "1",
                "sceneName": "manualtest",
                "sceneId": "947828",
                "description": ""
            },
            {
                "triggerEnabled": "1",
                "manualEnabled": "1",
                "sceneName": "manualtest",
                "sceneId": "582111",
                "description": ""
            }
        ]
    },
    "status": 1
}
```

### 手动执行接口

命令: ` curl -v -X POST http://$ip:$port/api/familyscene/execute -H $token -d "{\"sceneId\":\"$ret2SceneId\"}" `

```json
{
    "requestId": "abcddcba",
    "request": "/api/familyscene/execute",
    "result": {
        "resInfo": "规则执行成功",
        "sceneId": "947828",
        "ruleScript": 1
    },
    "status": 1
}
```
