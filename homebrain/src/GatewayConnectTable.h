/***************************************************************************
 *  GatewayConnectTable.h -
 *
 *  Created: 2019-02-20 16:32:11
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __GatewayConnectTable_H__
#define __GatewayConnectTable_H__

#include "SQLiteTable.h"
#include "Mutex.h"
#include <vector>

#ifdef __cplusplus

namespace HB {

struct GatewayTableInfo {
    GatewayTableInfo(std::string gid = "")
        : nGid(gid), nDid(""), nType(""), nKey(""), nIp(""), nEnable(-1) {}
    std::string nGid;
    std::string nDid;
    std::string nType;
    std::string nKey;
    std::string nIp;
    int nEnable;
}; /* struct GatewayTableInfo */

class HBDatabase;
class GatewayConnectTable : public ::UTILS::SQLiteTable {
private:
    friend class HBDatabase;
    GatewayConnectTable(::UTILS::SQLiteDatabase &db);
public:
    virtual ~GatewayConnectTable();

    bool updateOrInsert(const GatewayTableInfo &info);
    bool deleteBy(const GatewayTableInfo &info, const std::string &where);
    bool queryBy(std::vector<GatewayTableInfo> &infos, const GatewayTableInfo &filter, const std::string &where);

private:
    ::UTILS::Mutex mMutex; /* make sure: a mutex operation */

}; /* class GatewayConnectTable */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __GatewayConnectTable_H__ */
