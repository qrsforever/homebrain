//******************************************************************
//
// Copyright 2018 letv
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//

#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <set>
#include <assert.h>
#include <unistd.h>

#include <sstream>

#include <pluginServer.h>
#include "logger.h"
#include "ConcurrentIotivityUtils.h"
#include "messageHandler.h"
#include "IotivityWorkItem.h"
#include "iotivity_config.h"
#include "rapidjson.h"
#include "document.h"
#include "oic_string.h"
#include "oic_malloc.h"

#define TAG "KONKE_PLUGIN"

#include "EH_ServerSdk.h"
#include "EH_Client.h"
#include "KonkeInternal.h"
#include "KonkeDevice.h"
#include "KonkeManager.h"
using namespace OC::Bridging;

using namespace std;

static const string MODULE = "Konke";

std::string konkeId("02169");
std::string konkeKey("C3E2C6C75DCC222A1472E67B57F05B83");

KonkeManager *kkmanager;
EH_Client *client;

std::map < std::string, KonkeDevice::Ptr >::iterator itKonke;
std::map < std::string, KonkeDevice::Ptr > addedKonkeDevices;

typedef struct
{
    char mac[MPM_MAX_LENGTH_32];
    char uri[MPM_MAX_URI_LEN];
} KonkeDeviceDetails;

#define KONKE_URI        "/konke/"
#define KONKE_CONFIG        "konke.config"
#define BM   3
const std::string KONKE_SWITCH_RESOURCE_TYPE = "oic.r.switch.binary";
const std::string SWITCH_RELATIVE_URI = "/switch";
const uint BINARY_SWITCH_CALLBACK = 0;

MPMPluginCtx *g_plugin_ctx = NULL;


FILE *sec_file(const char *, const char *mode)
{
    std::string filename = "konke_security";// + path;
    return fopen(filename.c_str(), mode);
}
string readFileIntoString(char * filename)
{
    ifstream ifile(filename);
    ostringstream buf;
    if (ifile.is_open()) {
        char ch;
        while(buf&&ifile.get(ch))
        buf.put(ch);
        ifile.close();
    }else
	    return "";
    return buf.str();
}
void konke_parseconfig(void)
{
    std::string json = readFileIntoString(KONKE_CONFIG);
    if(json == "")
	    return;
    rapidjson::Document doc;
    doc.Parse<0>(json.c_str());
    konkeId = doc["id"].GetString();
    konkeKey = doc["key"].GetString();
    DEBUG_PRINT("id="<< konkeId.c_str()<<" ,key="<< konkeKey.c_str());
}
void konke_start(void)
{
    konke_parseconfig();
    EH_ServerSdk::GetInstance()->Open();
    client = new EH_Client();
    kkmanager = KonkeManager::getInstance();
    kkmanager->clientInit(client, konkeId.c_str(), konkeKey.c_str()); 
}

MPMResult pluginCreate(MPMPluginCtx **pluginSpecificCtx)
{
    MPMPluginCtx *ctx = (MPMPluginCtx *) calloc(1, sizeof(MPMPluginCtx));
    if (ctx == NULL)
    {
        OIC_LOG(ERROR, TAG, "Allocation of plugin context failed");
        return MPM_RESULT_INTERNAL_ERROR;
    }
    *pluginSpecificCtx = ctx;
    g_plugin_ctx = ctx;
    ctx->device_name = "Konke bridge";
    ctx->resource_type = "oic.d.bridge";
    ctx->open = sec_file;

    konke_start();

    return MPM_RESULT_OK;
}

MPMResult pluginStart(MPMPluginCtx *ctx)
{
    ctx->stay_in_process_loop = true;
    OIC_LOG(INFO, TAG, "Plugin start called!");

    return MPM_RESULT_OK;
}
void KonkeNotifyObservers(std::string uri) {
    ConcurrentIotivityUtils::queueNotifyObservers(uri + SWITCH_RELATIVE_URI);
}
void echoResponse(MPMPipeMessage *message, std::string type)
{
    std::string s = type + " response echo";
    MPMSendResponse(s.c_str(), s.size(), message->msgType);
}
std::string createuniqueID(KonkeDevice::Ptr pDevice)
{
    std::string uniqueId(pDevice->m_mac);
    std::string token = "";
    std::string delimiter1 = ":";
    std::string delimiter2 = "-";
    size_t pos = 0;

    while ( (pos = uniqueId.find(delimiter1)) != std::string::npos)
    {
        uniqueId.replace(pos, 1, token);
    }
    while ( (pos = uniqueId.find(delimiter2)) != std::string::npos)
    {
        uniqueId.replace(pos, 3, token);
    }
    return KONKE_URI + uniqueId + "/"+ pDevice->m_channel;
}

MPMResult pluginScan(MPMPluginCtx *, MPMPipeMessage *message)
{

    std::string uri, uniqueId;
    std::map < std::string, KonkeDevice::Ptr > kkdevices;
    kkmanager->getDevices(kkdevices);
    for (itKonke = kkdevices.begin(); itKonke != kkdevices.end(); itKonke++) {
	//uri = KONKE_URI + itKonke->first;
	KonkeDevice::Ptr pDevice = itKonke->second;
	uri = createuniqueID(pDevice);
	DEBUG_PRINT("Scan Device URI:" << uri);
	    if(addedKonkeDevices.find(uri) != addedKonkeDevices.end()) {
		OIC_LOG_V(INFO, TAG, "Already Added %s. Ignoring", uri.c_str());
		continue;
	    }
	    OIC_LOG_V(ERROR, TAG, "SendResponse %s", uri.c_str());
	    MPMSendResponse(uri.c_str(), uri.size(), MPM_SCAN);
    }
    return MPM_RESULT_OK;
}

KonkeDevice::Ptr getKonkeDeviceFromOCFResourceUri(std::string resourceUri)
{
    OIC_LOG_V(INFO, TAG, "Request for %s ", resourceUri.c_str());

    for (auto uriToPair : addedKonkeDevices)
    {
        if (resourceUri.find(uriToPair.first) != std::string::npos)
        {
            return uriToPair.second;
        }
    }
    throw "Resource" + resourceUri + "not found";
}
OCRepPayload *getCommonPayload(const char *uri, char *interfaceQuery,
                               std::string resType, OCRepPayload *payload)
{
    if (!OCRepPayloadSetUri(payload, uri))
    {
        throw "Unable to set URI in the payload";
    }

    if (!OCRepPayloadAddResourceType(payload, resType.c_str()))
    {
        throw "Failed to set light resource type" ;
    }
    OIC_LOG_V(INFO, TAG, "Checking against if: %s", interfaceQuery);

    // If the interface filter is explicitly oic.if.baseline, include all properties.
    if (interfaceQuery && std::string(interfaceQuery) == std::string(OC_RSRVD_INTERFACE_DEFAULT))
    {
        if (!OCRepPayloadAddInterface(payload, OC_RSRVD_INTERFACE_ACTUATOR))
        {
            throw "Failed to set light interface";
        }

        if (!OCRepPayloadAddInterface(payload, std::string(OC_RSRVD_INTERFACE_DEFAULT).c_str()))
        {
            throw "Failed to set baseline interface" ;
        }
    }
    return payload;
}

OCEntityHandlerResult processGetRequest(OCRepPayload *payload, KonkeDevice::Ptr pDevice,
                                        std::string resType)
{
    if (payload == NULL)
    {
        throw "payload is null";
    }

    if (KONKE_SWITCH_RESOURCE_TYPE == resType)
    {
	//std::shared_ptr<KonkeSwitch>	sDev = std::dynamic_pointer_cast<KonkeSwitch>(pDevice);
	string info;
	bool status = false;
	pDevice->get(info);
	if(strcmp(info.c_str(), "ON") == 0) {
	       status = true;
	}
        if (!OCRepPayloadSetPropBool(payload, "value", status ))
        {
            throw "Failed to set 'value' (power) in payload";
        }
        OIC_LOG_V(INFO, TAG, "Device State: %s", status ? "true" : "false");
    }
    else
    {
        throw "Failed due to unkwown resource type";
    }
    return OC_EH_OK;
}
OCEntityHandlerResult processPutRequest(OCEntityHandlerRequest *ehRequest,
                                        KonkeDevice::Ptr pDevice, std::string resourceType,
                                        OCRepPayload *payload)
{

    if (!ehRequest || !ehRequest->payload ||
                                    ehRequest->payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        throw "Incoming payload is NULL or not a representation";
    }

    OCRepPayload *input = reinterpret_cast<OCRepPayload *>(ehRequest->payload);
    if (!input)
    {
        throw "PUT payload is null";
    }

    if (KONKE_SWITCH_RESOURCE_TYPE ==  resourceType)
    {
	bool power;
        if (!OCRepPayloadGetPropBool(input, "value", &power))
        {
            throw "No value (power) in representation" ;
        }
        OIC_LOG_V(INFO, TAG, "PUT/POST value (power):%s", power ? "true" : "false");
        if (!OCRepPayloadSetPropBool(payload, "value", power))
        {
            throw "Failed to set 'value' (power) in payload";
        }
        pDevice->put(power ? "ON" : "OFF");
    }
    else
    {
        throw "Failed due to unkwown resource type" ;
    }
    return OC_EH_OK;
}


OCEntityHandlerResult handleEntityHandlerRequests(
    OCEntityHandlerFlag ,
    OCEntityHandlerRequest *entityHandlerRequest,
    std::string resourceType)
{
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    OCRepPayload *responsePayload = NULL;
    OCRepPayload *payload = OCRepPayloadCreate();
    try
    {
        if (entityHandlerRequest == NULL)
        {
            throw "Entity handler received a null entity request context" ;
        }

        std::string uri = OCGetResourceUri(entityHandlerRequest->resource);
	KonkeDevice::Ptr pDevice = getKonkeDeviceFromOCFResourceUri(uri);
        char *interfaceQuery = NULL;
        char *resourceTypeQuery = NULL;
        char *dupQuery = OICStrdup(entityHandlerRequest->query);
        if (dupQuery)
        {
            MPMExtractFiltersFromQuery(dupQuery, &interfaceQuery, &resourceTypeQuery);
        }

        switch (entityHandlerRequest->method)
        {
            case OC_REST_GET:
                OIC_LOG_V(INFO, TAG, " GET Request for: %s", uri.c_str());
                ehResult = processGetRequest(payload, pDevice, resourceType);
                break;

            case OC_REST_PUT:
            case OC_REST_POST:

                OIC_LOG_V(INFO, TAG, "PUT / POST Request on %s", uri.c_str());
                ehResult = processPutRequest(entityHandlerRequest, pDevice, resourceType, payload);

                //  To include "if" in all payloads.
                interfaceQuery = (char *) OC_RSRVD_INTERFACE_DEFAULT;
                break;

            default:
                OIC_LOG_V(ERROR, TAG, "UnSupported Method [%d] Received ", entityHandlerRequest->method);
                ConcurrentIotivityUtils::respondToRequestWithError(entityHandlerRequest, " Unsupported Method",
                        OC_EH_METHOD_NOT_ALLOWED);
                 return OC_EH_OK;
        }
        responsePayload = getCommonPayload(uri.c_str(),interfaceQuery, resourceType, payload);
        ConcurrentIotivityUtils::respondToRequest(entityHandlerRequest, responsePayload, ehResult);
        OICFree(dupQuery);
    }
    catch (const char *errorMessage)
    {
        OIC_LOG_V(ERROR, TAG, "Error - %s ", errorMessage);
        ConcurrentIotivityUtils::respondToRequestWithError(entityHandlerRequest, errorMessage, OC_EH_ERROR);
        ehResult = OC_EH_OK;
    }

    OCRepPayloadDestroy(responsePayload);
 
    return ehResult;
}


// Entity handler for binary switch
OCEntityHandlerResult entityHandler(OCEntityHandlerFlag flag,
            OCEntityHandlerRequest *entityHandlerRequest, void *callback)
{
    uintptr_t callbackParamResourceType = (uintptr_t)callback;
    std::string resourceType;

    if (callbackParamResourceType == BINARY_SWITCH_CALLBACK)
    {
        resourceType = KONKE_SWITCH_RESOURCE_TYPE;
    }
    return handleEntityHandlerRequests(flag, entityHandlerRequest, resourceType);
}


void createOCFResources(std::string uri, std::string type)
{
    uint8_t resourceProperties = (OC_OBSERVABLE | OC_DISCOVERABLE);
#if 0
    if (isSecureEnvSet())
    {
        resourceProperties |= OC_SECURE;
    }
#endif
    DEBUG_PRINT("createOCFResources: " << uri);
    if (type.compare(KONKE_SWITCH_RESOURCE_TYPE) == 0 ) {
    ConcurrentIotivityUtils::queueCreateResource(uri + SWITCH_RELATIVE_URI,
                    KONKE_SWITCH_RESOURCE_TYPE.c_str(), OC_RSRVD_INTERFACE_ACTUATOR,
                    entityHandler, (void *) BINARY_SWITCH_CALLBACK, resourceProperties);
    }
}
MPMResult createPayloadForMetaData(MPMResourceList **list, const std::string &configURI,
                              const std::string rt, const std::string res_if)
{
    MPMResourceList *temp = (MPMResourceList *)OICCalloc(1, sizeof(MPMResourceList));
    if (temp == NULL)
    {
        OIC_LOG_V(ERROR, TAG, "Calloc failed for createPayloadForMetaData %s", strerror(errno));
        return MPM_RESULT_OUT_OF_MEMORY;
    }

    OICStrcpy(temp->rt, MPM_MAX_LENGTH_64, rt.c_str());
    OICStrcpy(temp->href, MPM_MAX_URI_LEN, configURI.c_str());
    OICStrcpy(temp->interfaces, MPM_MAX_LENGTH_64, res_if.c_str());
    temp->bitmap = BM;

    temp->next = *list;
    *list = temp;
    return MPM_RESULT_OK;
}
KonkeDevice::Ptr getDeviceByURI(std::string uri)
{
    std::map < std::string, KonkeDevice::Ptr > kkdevices;
    std::string uriIt;
    kkmanager->getDevices(kkdevices);
    for (itKonke = kkdevices.begin(); itKonke != kkdevices.end(); itKonke++) {
	KonkeDevice::Ptr pDevice = itKonke->second;
	uriIt = createuniqueID(pDevice);
	//uriIt = pDevice->m_uri;
	if(!uriIt.compare(uri))
		return pDevice;
    }
    return NULL;
}

MPMResult pluginAdd(MPMPluginCtx *, MPMPipeMessage *message)
{
    if (message->payloadSize <= 0 && message->payload == NULL)
    {
        OIC_LOG(ERROR, TAG, "No payload received, failed to add device");
        return MPM_RESULT_INTERNAL_ERROR;
    }

    MPMResourceList *list  = NULL;
    MPMResult result = MPM_RESULT_INTERNAL_ERROR;

    std::string uri = reinterpret_cast<const char *>(message->payload);

    if (addedKonkeDevices.find(uri) != addedKonkeDevices.end())
    {
        OIC_LOG_V(ERROR, TAG, "%s already added", uri.c_str());
        return MPM_RESULT_ALREADY_CREATED;
    }

    std::shared_ptr<KonkeDevice> pDevice=getDeviceByURI(uri);
    if(pDevice == NULL)
	    return result;
    addedKonkeDevices[uri] = pDevice;

    uint8_t *buff = (uint8_t *)OICCalloc(1, MPM_MAX_METADATA_LEN);
    if (buff == NULL)
    {
        OIC_LOG_V(ERROR, TAG, "Calloc failed %s", strerror(errno));
        return MPM_RESULT_OUT_OF_MEMORY;
    }
    size_t size = MPM_MAX_METADATA_LEN;
    KonkeDeviceDetails deviceDetails;

    MPMDeviceSpecificData deviceConfiguration;
    memset(&deviceDetails, 0, sizeof(KonkeDeviceDetails));
    memset(&deviceConfiguration, 0, sizeof(MPMDeviceSpecificData));

    // Create Resources and form metadata for RECONNECT

    createOCFResources(uri, KONKE_SWITCH_RESOURCE_TYPE);
    result = createPayloadForMetaData(&list, uri+SWITCH_RELATIVE_URI,
                KONKE_SWITCH_RESOURCE_TYPE.c_str(), OC_RSRVD_INTERFACE_ACTUATOR);

#if 0
    result= createPayloadForMetaData(&list, uri + BRIGHTNESS_RELATIVE_URI,
                HUE_BRIGHTNESS_RESOURCE_TYPE.c_str(),OC_RSRVD_INTERFACE_ACTUATOR);

    result = createPayloadForMetaData(&list, uri + CHROMA_RELATIVE_URI,
                HUE_CHROMA_RESOURCE_TYPE.c_str(), OC_RSRVD_INTERFACE_ACTUATOR);
#endif
    if(result != MPM_RESULT_OK)
    {
        OIC_LOG_V(ERROR, TAG, " Failed creating payload for metadata");
        return result;
    }

    OICStrcpy(deviceDetails.mac, MPM_MAX_LENGTH_32, pDevice->m_mac.c_str());
    OICStrcpy(deviceDetails.uri, MPM_MAX_URI_LEN, uri.c_str());


    OICStrcpy(deviceConfiguration.devName, MPM_MAX_LENGTH_64, "Konke");
    OICStrcpy(deviceConfiguration.devType, MPM_MAX_LENGTH_64, "Switch");
    OICStrcpy(deviceConfiguration.manufacturerName, MPM_MAX_LENGTH_256, "Konke");
    MPMFormMetaData(list, &deviceConfiguration, buff, size, &deviceDetails, sizeof(deviceDetails));

    MPMAddResponse response;
    memset(&response, 0, sizeof(MPMAddResponse));
    OICStrcpy(response.uri, MPM_MAX_URI_LEN, uri.c_str());
    memcpy(response.metadata, buff, MPM_MAX_METADATA_LEN);
    size_t response_size = sizeof(MPMAddResponse);

    MPMSendResponse(&response, response_size, MPM_ADD);

    OICFree(buff);

    return MPM_RESULT_OK;

}

MPMResult pluginRemove(MPMPluginCtx *, MPMPipeMessage *message)
{
    OIC_LOG(INFO, TAG, "Remove called! Remove iotivity resources here based on what the client says");
    echoResponse(message, "REMOVE");
#if 0
// power on/off for the device(URI) by MPM client
    static bool power=0;
    std::string uri = reinterpret_cast<const char *>(message->payload);
    if(strcmp(uri.c_str(), "startnetwork") == 0)
    {
	  kkmanager->openNetChannel();
          return MPM_RESULT_OK;
    }

    for (auto uriToPair : addedKonkeDevices) {
        if (uri.find(uriToPair.first) != std::string::npos)
        {
            std::shared_ptr<KonkeDevice> pDevice = uriToPair.second;
            pDevice->put(power?"ON":"OFF");
        }
    }
    power= power?0:1;
#endif
    return MPM_RESULT_OK;
}

MPMResult pluginReconnect(MPMPluginCtx *, MPMPipeMessage *message)
{
    OIC_LOG(INFO, TAG,
            "Reconnect called! Reconnect to devices, create resources from the message/cloud/db/file.");
    echoResponse(message, "ADD");
    return MPM_RESULT_OK;
}

MPMResult pluginStop(MPMPluginCtx *)
{
    OIC_LOG(INFO, TAG, "Stop called !");
    return MPM_RESULT_OK;
}


MPMResult pluginDestroy(MPMPluginCtx *pluginSpecificCtx)
{
    OIC_LOG(INFO, TAG, "Destroy called");
    if (!pluginSpecificCtx)
    {
        assert(g_plugin_ctx);

        if (pluginSpecificCtx->started)
        {
            pluginStop(pluginSpecificCtx);
        }
        // freeing the resource allocated in create
        free(pluginSpecificCtx);
        g_plugin_ctx = NULL;
    }

    return (MPM_RESULT_OK);
}
