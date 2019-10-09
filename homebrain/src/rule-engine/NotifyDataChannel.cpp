/***************************************************************************
 *  NotifyDataChannel.cpp -
 *
 *  Created: 2019-07-02 20:59:39
 *
 *  Copyright QRS
 ****************************************************************************/

#include "NotifyDataChannel.h"
#include "NotifyPayload.h"

#include "RuleEngineService.h"
#include "RuleEngineTable.h"
#include "HBGlobalTable.h"

#include "Common.h"
#include "Log.h"
#include "HBDatabase.h"

namespace HB {

NotifyDataChannel::NotifyDataChannel()
{
}

NotifyDataChannel::~NotifyDataChannel()
{

}

int NotifyDataChannel::init()
{
    return 0;
}

bool NotifyDataChannel::send(int action, std::shared_ptr<Payload> data)
{
    if (action != PT_NOTIFY_PAYLOAD)
        return false;

    std::shared_ptr<NotifyPayload> payload(std::dynamic_pointer_cast<NotifyPayload>(data));
    if (!payload)
        return false;
    LOGD("payload[%s, %s %s]\n", payload->mID.c_str(), payload->mTitle.c_str(), payload->mContent.c_str());

    const char *id = payload->mID.c_str();
    if (0 == strcmp(NOID_RULE_ENABLE, id)) {
        std::vector<RuleTableInfo> infos;
        mainDB().queryBy(infos, payload->mTitle);
        if (infos.size() != 1) {
            LOGE("not found rule[%s]\n", payload->mTitle.c_str());
            return false;
        }
        infos[0].nEnable = 1;
        mainDB().updateOrInsert(infos[0]);
        ruleEngine().enableRule(infos[0].nRuleId, infos[0].nScriptData, true);
    } else if (0 == strcmp(NOID_RULE_DISABLE, id)) {
        std::vector<RuleTableInfo> infos;
        mainDB().queryBy(infos, payload->mTitle);
        if (infos.size() != 1) {
            LOGE("not found rule[%s]\n", payload->mTitle.c_str());
            return false;
        }
        infos[0].nEnable = 0;
        mainDB().updateOrInsert(infos[0]);
        ruleEngine().disableRule(infos[0].nRuleId, true);
    } else if (0 == strcmp(NOID_SCENE_MODE, id)) {
        SmartHomeInfo info(SCENE_MODE_FILED, payload->mTitle);
        mainDB().updateOrInsert(info);
    }
    return true;
}

}
