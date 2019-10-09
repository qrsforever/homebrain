---

title: Larfe设备对接

date: 2019-06-26 13:51:19
tags: [LE]
categories: [Comany]

---

# 云端文档

[直联相关接口](http://wiki.letv.cn/pages/viewpage.action?pageId=80580503)

[设备与Elink云接口文档](http://wiki.letv.cn/pages/viewpage.action?pageId=79233222)


# 测试设备

产品名称|ProductKey|节点类型|创建时间
--------|----------|:--------:|--------
拉孚空调    |pk.2f0f787e2d5|  设备  |2019-06-25 17:46:51
HomeBrain   |pk.33ef1b64b64|  网关  |2019-06-26 13:44:35
拉孚灯控面板|pk.8f388580a4c|  设备  |2019-06-26 13:47:40
拉孚门磁    |pk.94509004ba2|  设备  |2019-06-26 13:46:20


# 命令测试

```shell
curl -s -H "Content-Type: application/json" -X POST -d '{"productKey": "pk.33ef1b64b64","deviceName": "HomeBrain","deviceId": "test.123456","manufacture": "test"}' https://elinkplantform-pub.scloud.le.com/device/register
```
