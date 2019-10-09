/* ****************************************************************
 *
 * Copyright 2018 Letv All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/**
 * This file contains the declaration of reserved variables.
 */


#ifndef DEVICE_MANAGER_COMMONAPI_H_
#define DEVICE_MANAGER_COMMONAPI_H_

#include <string>

namespace OIC
{
    namespace Service
    {
        namespace HB
        {
            /**
             * Information about the reserved uri.
             */
            namespace RESERVED_URI
            {
                const std::string DEVICE = "/oic/d";
                const std::string PLATFORM = "/oic/p";
            }

            /**
             * Information about the device type.
             */
            namespace DEVICE_TYPE
            {
                const std::string UNKNOWN = "oic.wk.d";
                const std::string BRIDGE = "oic.d.bridge";
                const std::string HUE_BRI_LIGHT = "oic.d.hue_bri_light";
                const std::string HUE_COLOR_LIGHT = "oic.d.hue_color_light";
                const std::string HUE_CT_LIGHT = "oic.d.hue_ct_light";
                const std::string HUE_COLOR_CT_LIGHT = "oic.d.hue_color_ct_light";
                const std::string KK_LIGHTCTRL = "oic.d.kk_lightctrl";
                const std::string KK_SOSALARM = "oic.d.kk_sosalarm";
                const std::string KK_DOORCONTACT = "oic.d.kk_doorcontact";
                const std::string KK_ENVSENSOR = "oic.d.kk_envsensor";
                const std::string KK_SMART_PLUG = "oic.d.kk_smartplug";
                const std::string KK_CURTAINCTRL = "oic.d.kk_curtainctrl";
                const std::string KK_CURTAINMOTOR = "oic.d.kk_curtainmotor";
                const std::string KK_SCENECTRL = "oic.d.kk_scenectrl";
                const std::string KK_SCENECTRL1 = "oic.d.kk_scenectrl1";
                const std::string KK_BOOLSENSOR = "oic.d.kk_boolsensor";
                const std::string KK_ALARMER = "oic.d.kk_alarmer";
                const std::string LE_TV = "oic.d.letv";
            }

            /**
             * Information about the resource type.
             */
            namespace RESOURCE_TYPE
            {
                const std::string KONKE_DEVICE = "oic.r.konke.device";
                const std::string BINARYSWITCH = "oic.r.switch.binary";
                const std::string CONTACT_STATUS = "oic.r.sensor.contact";
                const std::string TEMPERATURE = "oic.r.temperature";
                const std::string HUMIDITY = "oic.r.humidity";
                const std::string ILLUMINANCE = "oic.r.sensor.illuminance";
                const std::string CURTAIN_STATUS = "oic.r.curtain.control";
                const std::string SCENE_STATUS = "oic.r.scene.control";
                const std::string BOOL_SENSOR = "oic.r.sensor.bool";
                const std::string ALARMER = "oic.r.alarmer";
                const std::string GATEWAY = "oic.r.gateway";
            }

            /**
             * Information about the resource interface.
             */
            namespace INTERFACE
            {
                const std::string BASELINE = "oic.if.baseline";
                const std::string LINKS_LIST = "oic.if.ll";
                const std::string BATCH = "oic.if.b";
                const std::string READ_ONLY = "oic.if.r";
                const std::string READ_WRITE = "oic.if.rw";
                const std::string ACTUATOR = "oic.if.a";
                const std::string SENSOR = "oic.if.s";
            }

            /**
             * Resource Properties.
             *
             * The value of a policy property is defined as bitmap.
             * The LSB represents OC_DISCOVERABLE and Second LSB bit represents OC_OBSERVABLE and
             * so on. Not including the policy property is equivalent to zero.
             */
            namespace RESOURCE_PROPERTY
            {
                enum
                {
                    /** When none of the bits are set, the resource is non-discoverable &
                     *  non-observable by the client.*/
                    NONE = (0),

                    /** When this bit is set, the resource is allowed to be discovered by clients.*/
                    DISCOVERABLE = (1 << 0),

                    /** When this bit is set, the resource is allowed to be observed by clients.*/
                    OBSERVABLE = (1 << 1),
                };
            }
        }
    }
}
#endif /* */
