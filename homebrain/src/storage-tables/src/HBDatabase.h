/***************************************************************************
 *  HBDatabase.h - HomeBrain Database
 *
 *  Created: 2018-11-12 14:11:38
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __HBDatabase_H__
#define __HBDatabase_H__

#include "SQLiteDatabase.h"
#include "Object.h"

#include <string>
#include <vector>

#ifdef __cplusplus

namespace HB {

class HBDatabase : public ::UTILS::Object {
public:
    HBDatabase();
    ~HBDatabase();

    void init(const std::string &path, int interval);

    bool isOpen() { return mDB != 0; }
    bool open();
    bool close();

    /* Add and Mod */
    template <typename T> bool updateOrInsert(const T &info);

    /* Del */
    template <typename T> bool deleteBy(const T &info);

    /* Query */
    template <typename T> bool queryBy(std::vector<T> &infos, const T &filter);
    template <typename T> bool queryBy(std::vector<T> &infos, const std::string &key);

private:
    std::string mDBPath;
    int mAutoCloseInterval;
    UTILS::SQLiteDatabase *mDB;
}; /* class HBDatabase */

HBDatabase& mainDB();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __HBDatabase_H__ */
