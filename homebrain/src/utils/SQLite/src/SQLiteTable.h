/***************************************************************************
 *  SQLiteTable.h - Database Table Base Class Header
 *
 *  Created: 2018-06-26 12:29:15
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __SQLiteTable_H__
#define __SQLiteTable_H__

#include "SQLiteDatabase.h"
#include <string>
#include <vector>
#include <memory>

namespace UTILS {

class SQLiteTable {
public:
    virtual ~SQLiteTable();

    bool init();
    int version();
    const std::string& tableName() const { return mTabName; }

    template <typename T>
    bool updateOrInsert(const T &/*info*/) { return false; }

    virtual void dump() { };

private:
    bool _Update(int currentVersion, int latestVersion);

protected:
    SQLiteTable(SQLiteDatabase &db, const char *tabName);
    SQLiteDatabase& mDB;
	std::vector<std::pair<int, std::string>> mUpdateHistoryList; /* for iterate alter update table */
    std::string mTabName;
}; /* class SQLiteTable */

} /* namespace HB */

#ifdef __cplusplus


#endif /* __cplusplus */

#endif /* __SQLiteTable_H__ */
