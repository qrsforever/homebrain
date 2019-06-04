#!/usr/bin/python3
# -*- coding: utf-8 -*-

# @file unittest_pub.py
# @brief
# @author QRS
# @blog qrsforever.github.io
# @version 1.0
# @date 2019-05-22 13:23:59

import paho.mqtt.client as mqtt
import time, json, sys

HOST = "10.122.67.99"
PORT = 1883

def test_registerReply(client):
    payload = {
            "request_id":"test",
            "result": {
                "deviceId":"deviceId1",
                "deviceSecret":"deviceSecret1",
                "elinkId":"elinkId1"
                },
            "timestamp":1555038123
            }

    topic = "/sys/pk.be25f066ac9/gateway/thing/sub/registerReply"
    print(topic);
    print(payload);
    client.publish(topic, json.dumps(payload, ensure_ascii=False), qos=0)

def test_addReply(client):
    payload = {
            "request_id":"test",
            "elinkId":"elinkId1",
            "result": {
                "code":200,
                "message":"操作成功"
                },
            "timestamp":1554974992
            }

    topic = "/sys/pk.be25f066ac9/gateway/thing/topo/addReply"
    print(topic);
    print(payload);
    client.publish(topic, json.dumps(payload, ensure_ascii=False), qos=0)

def test_delReply(client):
    payload = {
            "request_id":"test",
            "elinkId":"elinkId1",
            "result": {
                "code":200,
                "message":"操作成功"
                },
            "timestamp":1554974992
            }
    topic = "/sys/pk.be25f066ac9/gateway/thing/topo/deleteReply"
    print(topic);
    print(payload);
    client.publish(topic, json.dumps(payload, ensure_ascii=False), qos=0)

def test_set(client):
    payload = {
            "request_id":"test",
            "elinkId":"elinkId1",
            "params": [
                {
                    "Power":1,
                    "Power1":1
                    },
                {
                    "Power2":2
                    }
                ],
            "timestamp":1554974303
            }
    topic = "/sys/property/pk.be25f066ac9/gateway/set"
    print(topic);
    print(payload);
    client.publish(topic, json.dumps(payload, ensure_ascii=False), qos=0)

def test_get(client):
    payload = {
            "request_id":"test",
            "elinkId":"elinkId1",
            "params": [
                "temperature",
                "fan"
                ],
            "timestamp":1554977466
            }
    topic = "/sys/property/pk.be25f066ac9/gateway/get"
    print(topic);
    print(payload);
    client.publish(topic, json.dumps(payload, ensure_ascii=False), qos=0)

unittests = {
        '1': 'test_registerReply',
        '2': 'test_addReply',
        '3': 'test_delReply',
        '4': 'test_set',
        '5': 'test_get',
        }
if __name__ == "__main__":
    if (len(sys.argv) == 1):
        print("Use: ./unittest_pub.py num")
        print(unittests)
        exit(0)
    client = mqtt.Client("unittest")
    client.username_pw_set("test", "test");
    client.connect(HOST, PORT, 3)
    eval(unittests[sys.argv[1]])(client)
