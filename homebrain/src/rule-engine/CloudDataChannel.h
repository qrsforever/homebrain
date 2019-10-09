/***************************************************************************
 *  CloudDataChannel.h - Rule Data Channel
 *
 *  Created: 2018-06-14 15:02:39
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __CloudDataChannel_H__
#define __CloudDataChannel_H__

#include "DataChannel.h"
#include "RulePayload.h"
#include "ScenePayload.h"
#include "ClassPayload.h"
#include "rapidjson/document.h"

#ifdef SIM_SUITE
#include "TempSimulateSuite.h"
#endif

/* #define USE_TIMER_EVENT 1 */

#ifdef USE_TIMER_EVENT
#include "TimerEvent.h"
#endif

#ifdef __cplusplus

#define ELINK_RULE_DOC_VERSION      "0.0.7"
#define ELINK_DEVICE_DOC_VERSION    "0.0.7"

namespace HB {

class RuleEventHandler;

class CloudDataChannel : public DataChannel {
public:
    CloudDataChannel();
    virtual ~CloudDataChannel();

    int init();

    virtual bool send(int action, std::shared_ptr<Payload> payload);

protected:
    RuleEventHandler &mH;
}; /* class CloudDataChannel */

class ElinkCloudDataChannel : public CloudDataChannel {
public:
    ElinkCloudDataChannel();
    ~ElinkCloudDataChannel();

    int init();
    void onSyncRuleProfile(const std::string jsonDoc);
    void onSyncDeviceProfile(const std::string deviceName, const std::string jsonDoc);

    bool send(int action, std::shared_ptr<Payload> payload);

    static bool parseProfile(rapidjson::Value &profile, std::shared_ptr<ClassPayload> payload);
    static bool parseRule(rapidjson::Value &rule, std::shared_ptr<RulePayload> payload);
    static bool parseTrigger(rapidjson::Value &trigger, std::shared_ptr<RulePayload> payload);
    static bool parseConditions(rapidjson::Value &conditions, std::shared_ptr<RulePayload> payload);
    static bool parseActions(rapidjson::Value &actions, std::shared_ptr<RulePayload> payload);
    static bool parseTimeString(const char *timestr, SlotPoint &slotpoint);

    static bool parseScene(rapidjson::Value &rule, std::shared_ptr<ScenePayload> payload);
    static bool parseDeviceControl(rapidjson::Value &controls, ms_on_event_t event, std::shared_ptr<ScenePayload> payload);
    static bool parseNotifyContent(rapidjson::Value &contents, ms_on_event_t event, std::shared_ptr<ScenePayload> payload);
    static bool parseMicroService(rapidjson::Value &services, ms_on_event_t event, std::shared_ptr<ScenePayload> payload);

#ifdef USE_TIMER_EVENT
    static bool parseTimeString(const char *timestr, TimeNode &node);
#endif
    static bool parsePropValue(const char *propval, SlotPoint &slotpoint);
    static bool parsePropValue(const char *propval, Slot &slot);

}; /* class ElinkCloudDataChannel */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __CloudDataChannel_H__ */
