---

title: Http-Handler
date: 2018-10-24 10:05:26
tags: [ http, cpp ]
categories: [ project ]

---

[crow](https://github.com/ipkn/crow)

## 设计初稿

MicroHttpHandler : 注册route, 服务监听
MicroMiddleware : 中间信息截获
MicroLogHandler : 日志统一接入HB日志系统


[Json在线](http://tool.oschina.net/codeformat/json)


## Test

```json
{
    "ruleId": "-",
    "description": "当空气净化器进入极速模式，扫地机器人进入沿边清扫模式",
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
                    "propValue": "v==4",
                    "deviceId": "0007A895C8A7",
                    "propName": "WorkMode"
                }
            ],
            "deviceLogic": ""
        }
    },
    "ruleName": "autotest1",
    "actions": {
        "deviceControl": [
            {
                "propValue": "2",
                "deviceId": "04FA8309822A",
                "propName": "CleaningMode"
            }
        ]
    }
}
```

`curl -v -X POST http://0.0.0.0:8899/rule/add -H "token:1234567" -d @xxx/test.json`

`curl -v -X POST http://0.0.0.0:8899/rule/modify -H "token:1234567" -d @xxx/test2.json`

`curl -v -X POST http://0.0.0.0:8899/rule/query -H "token:1234567" -d '{"rule_id":"158679"}'`

`curl -v -X POST http://0.0.0.0:8899/rule/list -H "token:1234567"`

`curl -v -X POST http://0.0.0.0:8899/rule/delete -H "token:1234567" -d '{"rule_id":"158679"}'`

