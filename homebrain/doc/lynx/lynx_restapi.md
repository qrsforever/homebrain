---

title: Lynx REST API
date: 2018-10-10 17:16:03
tags: [ HomeBrain ]
categories: [ IOT ]

---

<!-- vim-markdown-toc GFM -->

* [URLEncode:](#urlencode)
* [Others](#others)
    * [status](#status)
    * [userinfo](#userinfo)
    * [nobody_at_home](#nobody_at_home)
* [List](#list)
    * [list](#list-1)
    * [list_scenes](#list_scenes)
    * [list_collections](#list_collections)
    * [list_rules](#list_rules)
    * [list_triggers](#list_triggers)
    * [list_conditions](#list_conditions)
    * [list_actions](#list_actions)
* [Add/Remove](#addremove)
    * [add_scene](#add_scene)
    * [add_to_scene](#add_to_scene)
    * [remove_scene](#remove_scene)
    * [remove_from_scene](#remove_from_scene)
* [Control](#control)
    * [light](#light)
* [Post](#post)
    * [rules](#rules)

<!-- vim-markdown-toc -->

# URLEncode:
  %2F - /
  %2C - ,
  %3D - =

# Others

## status

引擎自身的状态信息

请求：
    https://192.168.124.19:443/lynx/status
返回：
    {
      "version": "v1.0.1",
      "API_version": "v2",
      "OC_IPV4": true,
      "OC_SECURITY": true,
      "lynx_commit": "e15b3e7",
      "iot_commit": "7c69843",
      "buildtime": "2018-10-04_15:52:17",
      "platform": "x86-64_gcc520_glibc222_2__x86-64-glibc-2.22-dynmem",
      "di": "b44a1f0f-1a24-4448-7037-d215639c1857",
      "appdata": "/var/lynxmind/",
      "config": "/var/lynxmind//lynxmind.ini",
      "datfile": "/var/lynxmind/engine.dat",
      "custom_file": "/var/lynxmind//custom.dat",
      "external_url": "https://${IP}:443/lynx/",
      "oic.wk.col": 0,
      "oic.wk.sceneList": 2,
      "oic.wk.introspection": 0,
      "oic.wk.p": 4,
      "oic.wk.d": 4,
      "oic.wk.d (offline)": 0,
      "oic.wk.con": 3,
      "oic.wk.res": 0,
      "oic.r.doxm": 0,
      "oic.r.pstat": 0,
      "oic.r.cred": 0,
      "oic.r.acl2": 0,
      "oic.d.<x>": 3,
      "oic.r.<x>": 6,
      "total": "16 of 1500"
    }


## userinfo

请求:
    https://192.168.124.19:443/lynx/userinfo

返回:
    {
     "user": "lidong8@le.com",
     "logged_in": false
    }

## nobody_at_home

请求:
    https://192.168.124.19:443/lynx/nobody_at_home?false

返回:
    { "nobody_at_home": false }

# List

## list

陈列所有设备(scama)

请求：
    https://192.168.124.19:443/lynx/list
返回：
     [
      {
        "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
        "online": true,
        "secure": true,
        "owned": true,
        "iconid": "ic_default_light",
        "name": "Hue lightstrip plus 1",
        "rt": "oic.d.light",
        "resources": [
          {
            "path": "/light/8",
            "iconid": "ic_default_switch_binary",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.switch.binary",
            "properties": {
              "value": {
                "type": "boolean",
                "value": true
              }
            }
          },
          {
            "path": "/light/8~1",
            "iconid": "ic_default_light_brightness",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.light.brightness",
            "properties": {
              "brightness": {
                "type": "integer",
                "value": 100,
                "units": "%",
                "range": [0,100]
              }
            }
          },
          {
            "path": "/light/8~2",
            "iconid": "ic_default_colour_chroma",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.colour.chroma",
            "properties": {
              "hue": {
                "type": "float",
                "value": 356.000916,
                "range": [0.0,360.0]
              },
              "saturation": {
                "type": "integer",
                "value": 220
              },
              "ct": {
                "type": "integer",
                "value": 360,
                "units": "mired"
              }
            }
          }
        ]
      },
      {
        "di": "29f16dfe-0bdb-4489-6b76-bd952f4a7401",
        "online": true,
        "secure": true,
        "owned": true,
        "iconid": "ic_default_light",
        "name": "Hue white floor 1",
        "rt": "oic.d.light",
        "resources": [
          {
            "path": "/light/7",
            "iconid": "ic_default_switch_binary",
            "name": "Hue white floor 1",
            "rt": "oic.r.switch.binary",
            "properties": {
              "value": {
                "type": "boolean",
                "value": true
              }
            }
          },
          {
            "path": "/light/7~1",
            "iconid": "ic_default_light_brightness",
            "name": "Hue white floor 1",
            "rt": "oic.r.light.brightness",
            "properties": {
              "brightness": {
                "type": "integer",
                "value": 100,
                "units": "%",
                "range": [0,100]
              }
            }
          }
        ]
      }
    ]


## list_scenes

列出所有场景

请求:
    https://192.168.124.19:443/lynx/list_scenes

返回:
    [
      {
        "path": "/3464a92d1340_7582",
        "di": "bdf5eb6b-c067-4bf9-43d3-3464a92d1340",
        "id": "3464a92d1340_7582",
        "online": true,
        "secure": false,
        "owned": false,
        "iconid": "ic_default_scene",
        "name": "scene_come_home",
        "rt": "oic.wk.sceneList",
        "devices": [
          {
            "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
            "online": true,
            "secure": true,
            "owned": true,
            "iconid": "ic_default_light",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.d.light",
            "resources": [
              {
                "path": "/light/8",
                "rt": "oic.r.switch.binary",
                "iconid": "ic_default_switch_binary",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "value": {
                    "type": "boolean",
                    "value": true,
                    "targetvalue": true
                  }
                }
              },
              {
                "path": "/light/8~1",
                "rt": "oic.r.light.brightness",
                "iconid": "ic_default_light_brightness",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "brightness": {
                    "type": "integer",
                    "value": 100,
                    "targetvalue": 100,
                    "units": "%",
                    "range": [0,100]
                  }
                }
              },
              {
                "path": "/light/8~2",
                "rt": "oic.r.colour.chroma",
                "iconid": "ic_default_colour_chroma",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "hue": {
                    "type": "float",
                    "value": 356.000916,
                    "targetvalue": 356.000000,
                    "range": [0.0,360.0]
                  },
                  "saturation": {
                    "type": "integer",
                    "value": 220,
                    "targetvalue": 220
                  },
                  "ct": {
                    "type": "integer",
                    "value": 360,
                    "units": "mired"
                  }
                }
              }
            ]
          },
          {
            "di": "29f16dfe-0bdb-4489-6b76-bd952f4a7401",
            "online": true,
            "secure": true,
            "owned": true,
            "iconid": "ic_default_light",
            "name": "Hue white floor 1",
            "rt": "oic.d.light",
            "resources": [
              {
                "path": "/light/7",
                "rt": "oic.r.switch.binary",
                "iconid": "ic_default_switch_binary",
                "name": "Hue white floor 1",
                "properties":{
                  "value": {
                    "type": "boolean",
                    "value": true,
                    "targetvalue": true
                  }
                }
              },
              {
                "path": "/light/7~1",
                "rt": "oic.r.light.brightness",
                "iconid": "ic_default_light_brightness",
                "name": "Hue white floor 1",
                "properties":{
                  "brightness": {
                    "type": "integer",
                    "value": 100,
                    "targetvalue": 100,
                    "units": "%",
                    "range": [0,100]
                  }
                }
              }
            ]
          }
        ]
      },
      {
        "path": "/3464a92d1340_2507",
        "di": "bdf5eb6b-c067-4bf9-43d3-3464a92d1340",
        "id": "3464a92d1340_2507",
        "online": true,
        "secure": false,
        "owned": false,
        "iconid": "ic_default_scene",
        "name": "scene_leave_home",
        "rt": "oic.wk.sceneList",
        "devices": [
          {
            "di": "29f16dfe-0bdb-4489-6b76-bd952f4a7401",
            "online": true,
            "secure": true,
            "owned": true,
            "iconid": "ic_default_light",
            "name": "Hue white floor 1",
            "rt": "oic.d.light",
            "resources": [
              {
                "path": "/light/7",
                "rt": "oic.r.switch.binary",
                "iconid": "ic_default_switch_binary",
                "name": "Hue white floor 1",
                "properties":{
                  "value": {
                    "type": "boolean",
                    "value": true,
                    "targetvalue": false
                  }
                }
              },
              {
                "path": "/light/7~1",
                "rt": "oic.r.light.brightness",
                "iconid": "ic_default_light_brightness",
                "name": "Hue white floor 1",
                "properties":{
                  "brightness": {
                    "type": "integer",
                    "value": 100,
                    "targetvalue": 95,
                    "units": "%",
                    "range": [0,100]
                  }
                }
              }
            ]
          },
          {
            "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
            "online": true,
            "secure": true,
            "owned": true,
            "iconid": "ic_default_light",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.d.light",
            "resources": [
              {
                "path": "/light/8",
                "rt": "oic.r.switch.binary",
                "iconid": "ic_default_switch_binary",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "value": {
                    "type": "boolean",
                    "value": true,
                    "targetvalue": false
                  }
                }
              },
              {
                "path": "/light/8~1",
                "rt": "oic.r.light.brightness",
                "iconid": "ic_default_light_brightness",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "brightness": {
                    "type": "integer",
                    "value": 100,
                    "targetvalue": 88,
                    "units": "%",
                    "range": [0,100]
                  }
                }
              },
              {
                "path": "/light/8~2",
                "rt": "oic.r.colour.chroma",
                "iconid": "ic_default_colour_chroma",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "hue": {
                    "type": "float",
                    "value": 356.000916,
                    "targetvalue": 300.002747,
                    "range": [0.0,360.0]
                  },
                  "saturation": {
                    "type": "integer",
                    "value": 220,
                    "targetvalue": 134
                  },
                  "ct": {
                    "type": "integer",
                    "value": 360,
                    "targetvalue": 360,
                    "units": "mired"
                  }
                }
              }
            ]
          }
        ]
      }
    ]


## list_collections

列出设备集合

请求:
    https://192.168.124.19:443/lynx/list_collections

返回:
    [
      {
        "path": "/3464a92d1340_0725",
        "di": "bdf5eb6b-c067-4bf9-43d3-3464a92d1340",
        "id": "3464a92d1340_0725",
        "online": true,
        "secure": false,
        "owned": false,
        "iconid": "ic_default_collection",
        "name": "collection_room1",
        "rt": "oic.wk.col",
        "devices": [
          {
            "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
            "online": true,
            "secure": true,
            "owned": true,
            "iconid": "ic_default_light",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.d.light",
            "resources": [
              {
                "path": "/light/8",
                "rt": "oic.r.switch.binary",
                "iconid": "ic_default_switch_binary",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "value": {
                    "type": "boolean",
                    "value": true
                  }
                }
              },
              {
                "path": "/light/8~1",
                "rt": "oic.r.light.brightness",
                "iconid": "ic_default_light_brightness",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "brightness": {
                    "type": "integer",
                    "value": 100,
                    "units": "%",
                    "range": [0,100]
                  }
                }
              },
              {
                "path": "/light/8~2",
                "rt": "oic.r.colour.chroma",
                "iconid": "ic_default_colour_chroma",
                "name": "Hue lightstrip plus 1",
                "properties":{
                  "hue": {
                    "type": "float",
                    "value": 154.997482,
                    "range": [0.0,360.0]
                  },
                  "saturation": {
                    "type": "integer",
                    "value": 130
                  },
                  "ct": {
                    "type": "integer",
                    "value": 360,
                    "units": "mired"
                  }
                }
              }
            ]
          },
          {
            "di": "29f16dfe-0bdb-4489-6b76-bd952f4a7401",
            "online": true,
            "secure": true,
            "owned": true,
            "iconid": "ic_default_light",
            "name": "Hue white floor 1",
            "rt": "oic.d.light",
            "resources": [
              {
                "path": "/light/7",
                "rt": "oic.r.switch.binary",
                "iconid": "ic_default_switch_binary",
                "name": "Hue white floor 1",
                "properties":{
                  "value": {
                    "type": "boolean",
                    "value": true
                  }
                }
              },
              {
                "path": "/light/7~1",
                "rt": "oic.r.light.brightness",
                "iconid": "ic_default_light_brightness",
                "name": "Hue white floor 1",
                "properties":{
                  "brightness": {
                    "type": "integer",
                    "value": 100,
                    "units": "%",
                    "range": [0,100]
                  }
                }
              }
            ]
          }
        ]
      }
    ]


## list_rules

列出所有规则

请求:
    https://192.168.124.19:443/lynx/list_rules

返回:
    [
      {
        "id": "32800",
        "enabled": true,
        "n": "rule_come_home",
        "event": {
          "iconid": "ic_icons_home_nobody_home",
          "trigger": "nobodyathome"
        },
        "conditions": [
          {
            "condition": [
              {
                "trigger": true,
                "iconid": "ic_icons_home_nobody_home",
                "property": "nobodyathome",
                "operator": "eq",
                "value": false
              }
            ]
          }
        ],
        "actions": [
          {
            "action": [
              {
                "iconid": "ic_default_scene",
                "property": "scene",
                "value": "3464a92d1340_7582"
              }
            ]
          }
        ]
      },
      {
        "id": "35750",
        "enabled": true,
        "n": "rule_leave_home",
        "event": {
          "iconid": "ic_icons_home_nobody_home",
          "trigger": "nobodyathome"
        },
        "conditions": [
          {
            "condition": [
              {
                "trigger": true,
                "iconid": "ic_icons_home_nobody_home",
                "property": "nobodyathome",
                "operator": "eq",
                "value": true
              }
            ]
          }
        ],
        "actions": [
          {
            "action": [
              {
                "iconid": "ic_default_scene",
                "property": "scene",
                "value": "3464a92d1340_2507"
              }
            ]
          }
        ]
      }
    ]


## list_triggers

请求:
    https://192.168.124.19:443/lynx/list_triggers

返回:
    [
      {
        "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
        "online": true,
        "secure": true,
        "owned": true,
        "iconid": "ic_default_light",
        "name": "Hue lightstrip plus 1",
        "rt": "oic.d.light",
        "resources": [
          {
            "path": "/light/8",
            "iconid": "ic_default_switch_binary",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.switch.binary",
            "properties": {
              "value": {
                "type": "boolean",
                "value": true
              }
            }
          },
          {
            "path": "/light/8~1",
            "iconid": "ic_default_light_brightness",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.light.brightness",
            "properties": {
              "brightness": {
                "type": "integer",
                "value": 100,
                "units": "%",
                "range": [0,100]
              }
            }
          },
          {
            "path": "/light/8~2",
            "iconid": "ic_default_colour_chroma",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.colour.chroma",
            "properties": {
              "hue": {
                "type": "float",
                "value": 356.000916,
                "range": [0.0,360.0]
              },
              "saturation": {
                "type": "integer",
                "value": 220
              }
            }
          }
        ]
      },
      {
        "di": "29f16dfe-0bdb-4489-6b76-bd952f4a7401",
        "online": true,
        "secure": true,
        "owned": true,
        "iconid": "ic_default_light",
        "name": "Hue white floor 1",
        "rt": "oic.d.light",
        "resources": [
          {
            "path": "/light/7",
            "iconid": "ic_default_switch_binary",
            "name": "Hue white floor 1",
            "rt": "oic.r.switch.binary",
            "properties": {
              "value": {
                "type": "boolean",
                "value": true
              }
            }
          },
          {
            "path": "/light/7~1",
            "iconid": "ic_default_light_brightness",
            "name": "Hue white floor 1",
            "rt": "oic.r.light.brightness",
            "properties": {
              "brightness": {
                "type": "integer",
                "value": 100,
                "units": "%",
                "range": [0,100]
              }
            }
          }
        ]
      },
      {
        "resources": [
          {
            "iconid": "ic_icons_home_nobody_home",
            "property": "nobodyathome",
            "value": {
              "type": "boolean",
              "value": false
            }
          },
          {
            "iconid": "ic_icons_timer",
            "property": "currenttime",
            "value": {
              "type": "iso8601",
              "value": "2018-10-10T08:58:30+00:00"
            }
          }
        ]
      }
    ]

## list_conditions

请求:
    https://192.168.124.19:443/lynx/list_conditions

返回:
    [
      {
        "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
        "online": true,
        "secure": true,
        "owned": true,
        "iconid": "ic_default_light",
        "name": "Hue lightstrip plus 1",
        "rt": "oic.d.light",
        "resources": [
          {
            "path": "/light/8",
            "iconid": "ic_default_switch_binary",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.switch.binary",
            "properties": {
              "value": {
                "type": "boolean",
                "value": true
              }
            }
          },
          {
            "path": "/light/8~1",
            "iconid": "ic_default_light_brightness",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.light.brightness",
            "properties": {
              "brightness": {
                "type": "integer",
                "value": 100,
                "units": "%",
                "range": [0,100]
              }
            }
          },
          {
            "path": "/light/8~2",
            "iconid": "ic_default_colour_chroma",
            "name": "Hue lightstrip plus 1",
            "rt": "oic.r.colour.chroma",
            "properties": {
              "hue": {
                "type": "float",
                "value": 356.000916,
                "range": [0.0,360.0]
              },
              "saturation": {
                "type": "integer",
                "value": 220
              }
            }
          }
        ]
      },
      {
        "di": "29f16dfe-0bdb-4489-6b76-bd952f4a7401",
        "online": true,
        "secure": true,
        "owned": true,
        "iconid": "ic_default_light",
        "name": "Hue white floor 1",
        "rt": "oic.d.light",
        "resources": [
          {
            "path": "/light/7",
            "iconid": "ic_default_switch_binary",
            "name": "Hue white floor 1",
            "rt": "oic.r.switch.binary",
            "properties": {
              "value": {
                "type": "boolean",
                "value": true
              }
            }
          },
          {
            "path": "/light/7~1",
            "iconid": "ic_default_light_brightness",
            "name": "Hue white floor 1",
            "rt": "oic.r.light.brightness",
            "properties": {
              "brightness": {
                "type": "integer",
                "value": 100,
                "units": "%",
                "range": [0,100]
              }
            }
          }
        ]
      },
      {
        "resources": [
          {
            "iconid": "ic_icons_calendar",
            "property": "weekday",
            "value": {
              "type": "integer",
              "range": [0,6],
              "value": 3
            }
          },
          {
            "iconid": "ic_icons_home_nobody_home",
            "property": "nobodyathome",
            "value": {
              "type": "boolean",
              "value": false
            }
          },
          {
            "iconid": "ic_icons_timer",
            "property": "currenttime",
            "value": {
              "type": "iso8601",
              "value": "2018-10-10T08:58:30+00:00"
            }
          }
        ]
      }
    ]


## list_actions

请求:
    https://192.168.124.19:443/lynx/list_actions

返回:
    [
      {
        "resources": [
          {
            "iconid": "ic_icons_home_nobody_home",
            "property": "nobodyathome",
            "value": {
              "type": "boolean",
              "value": false
            }
          },
          {
            "iconid": "ic_icons_message_info",
            "property": "send_info",
            "value": {
              "type": "function"
            }
          },
          {
            "iconid": "ic_icons_message_alert",
            "property": "send_alert",
            "value": {
              "type": "function"
            }
          },
          {
            "iconid": "ic_default_scene",
            "property": "activatescene",
            "value": {
              "type": "scene",
              "value": "3464a92d1340_7582",
              "name": "scene_come_home"
            }
          },
          {
            "iconid": "ic_default_scene",
            "property": "activatescene",
            "value": {
              "type": "scene",
              "value": "3464a92d1340_2507",
              "name": "scene_leave_home"
            }
          }
        ]
      }
    ]


# Add/Remove

## add_scene

请求:
    https://192.168.124.19:443/lynx/add_scene?n=scene_test1

返回:
    {

      "path": "/3464a92d1340_9800",
      "di": "bdf5eb6b-c067-4bf9-43d3-3464a92d1340",
      "id": "3464a92d1340_9800",
      "online": true,
      "secure": false,
      "owned": false,
      "iconid": "ic_default_scene",
      "name": "scene_test1",
      "rt": "oic.wk.sceneList",
      "devices": [

      ]
    }

## add_to_scene

请求:
     https://192.168.124.19:443/lynx/add_to_scene?参数

参数:
     id=3464a92d1340_9800
     &values=29f2db8c-2f5b-4a62-761b-a021ccb41508%2Flight%2F8%2Cvalue%3Dtrue
     &values=29f2db8c-2f5b-4a62-761b-a021ccb41508%2Flight%2F8~1%2Cbrightness%3D100
     &values=29f2db8c-2f5b-4a62-761b-a021ccb41508%2Flight%2F8~2%2Chue%3D154.997482%2Csaturation%3D130%2Cct%3D360

返回:
    {
      "path": "/3464a92d1340_9800",
      "di": "bdf5eb6b-c067-4bf9-43d3-3464a92d1340",
      "id": "3464a92d1340_9800",
      "online": true,
      "secure": false,
      "owned": false,
      "iconid": "ic_default_scene",
      "name": "scene_test1",
      "rt": "oic.wk.sceneList",
      "devices": [
        {
          "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
          "online": true,
          "secure": true,
          "owned": true,
          "iconid": "ic_default_light",
          "name": "Hue lightstrip plus 1",
          "rt": "oic.d.light",
          "resources": [
            {
              "path": "/light/8",
              "rt": "oic.r.switch.binary",
              "iconid": "ic_default_switch_binary",
              "name": "Hue lightstrip plus 1",
              "properties":{
                "value": {
                  "type": "boolean",
                  "value": true,
                  "targetvalue": true
                }
              }
            },
            {
              "path": "/light/8~1",
              "rt": "oic.r.light.brightness",
              "iconid": "ic_default_light_brightness",
              "name": "Hue lightstrip plus 1",
              "properties":{
                "brightness": {
                  "type": "integer",
                  "value": 100,
                  "targetvalue": 100,
                  "units": "%",
                  "range": [0,100]
                }
              }
            },
            {
              "path": "/light/8~2",
              "rt": "oic.r.colour.chroma",
              "iconid": "ic_default_colour_chroma",
              "name": "Hue lightstrip plus 1",
              "properties":{
                "hue": {
                  "type": "float",
                  "value": 154.997482,
                  "targetvalue": 154.997482,
                  "range": [0.0,360.0]
                },
                "saturation": {
                  "type": "integer",
                  "value": 130,
                  "targetvalue": 130
                },
                "ct": {
                  "type": "integer",
                  "value": 360,
                  "targetvalue": 360,
                  "units": "mired"
                }
              }
            }
          ]
        }
      ]
    }


## remove_scene

请求:
    https://192.168.124.19:443/lynx/remove_scene?id=3464a92d1340_5302

返回:
    {}


## remove_from_scene

请求:
    https://192.168.124.19:443/lynx/remove_from_scene?id=3464a92d1340_2507&links=29f16dfe-0bdb-4489-6b76-bd952f4a7401

返回:
    {
      "path": "/3464a92d1340_2507",
      "di": "bdf5eb6b-c067-4bf9-43d3-3464a92d1340",
      "id": "3464a92d1340_2507",
      "online": true,
      "secure": false,
      "owned": false,
      "iconid": "ic_default_scene",
      "name": "scene_leave_home",
      "rt": "oic.wk.sceneList",
      "devices": [
        {
          "di": "29f2db8c-2f5b-4a62-761b-a021ccb41508",
          "online": true,
          "secure": true,
          "owned": true,
          "iconid": "ic_default_light",
          "name": "Hue lightstrip plus 1",
          "rt": "oic.d.light",
          "resources": [
            {
              "path": "/light/8",
              "rt": "oic.r.switch.binary",
              "iconid": "ic_default_switch_binary",
              "name": "Hue lightstrip plus 1",
              "properties":{
                "value": {
                  "type": "boolean",
                  "value": true,
                  "targetvalue": true
                }
              }
            },
            {
              "path": "/light/8~1",
              "rt": "oic.r.light.brightness",
              "iconid": "ic_default_light_brightness",
              "name": "Hue lightstrip plus 1",
              "properties":{
                "brightness": {
                  "type": "integer",
                  "value": 88,
                  "targetvalue": 88,
                  "units": "%",
                  "range": [0,100]
                }
              }
            },
            {
              "path": "/light/8~2",
              "rt": "oic.r.colour.chroma",
              "iconid": "ic_default_colour_chroma",
              "name": "Hue lightstrip plus 1",
              "properties":{
                "hue": {
                  "type": "float",
                  "value": 300.002747,
                  "targetvalue": 300.002747,
                  "range": [0.0,360.0]
                },
                "saturation": {
                  "type": "integer",
                  "value": 134,
                  "targetvalue": 134
                },
                "ct": {
                  "type": "integer",
                  "value": 360,
                  "targetvalue": 360,
                  "units": "mired"
                }
              }
            }
          ]
        }
      ]
    }


# Control

## light

请求:
    https://192.168.124.19:443/lynx/29f2db8c-2f5b-4a62-761b-a021ccb41508/light/8?value=false

返回:
    {
      "path": "/light/8",
      "iconid": "ic_default_switch_binary",
      "name": "Hue lightstrip plus 1",
      "rt": "oic.r.switch.binary",
      "properties": {
        "value": {
          "type": "boolean",
          "value": false
        }
      }
    }


# Post

## rules

请求:
    https://192.168.124.19:443/lynx/rules

参数:
    {
	    "enabled": true,
	    "n": "rule_come_home",
	    "event": {
	    	"trigger": "nobodyathome"
	    },
	    "conditions": [{
	    	"condition": [{
	    		"value": "false",
	    		"operator": "eq",
	    		"property": "nobodyathome",
	    		"trigger": true
	    	}]
	    }],
	    "actions": [{
	    	"action": [{
	    		"value": "3464a92d1340_7582",
	    		"property": "scene",
	    		"delay": 0
	    	}]
	    }]
    }

返回:
    {
      "id": "32800",
      "enabled": true,
      "n": "rule_come_home",
      "event": {
        "iconid": "ic_icons_home_nobody_home",
        "trigger": "nobodyathome"
      },
      "conditions": [
        {
          "condition": [
            {
              "trigger": true,
              "iconid": "ic_icons_home_nobody_home",
              "property": "nobodyathome",
              "operator": "eq",
              "value": false
            }
          ]
        }
      ],
      "actions": [
        {
          "action": [
            {
              "iconid": "ic_default_scene",
              "property": "scene",
              "value": "3464a92d1340_7582"
            }
          ]
        }
      ]
    }
