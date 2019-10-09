/***************************************************************************
 *  LFDatabase.h -
 *
 *  Created: 2019-07-11 11:04:47
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __LFDatabase_H__
#define __LFDatabase_H__

#include "SQLiteDatabase.h"
#include "Object.h"

#include <string>
#include <vector>

#ifdef __cplusplus

namespace HB {

class LFDatabase : public ::UTILS::Object {
public:
    LFDatabase();
    ~LFDatabase();

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

    /* Dump */
    template <typename T> void dump(const T &info);

private:
    std::string mDBPath;
    int mAutoCloseInterval;
    UTILS::SQLiteDatabase *mDB;
}; /* class LFDatabase */

LFDatabase& mainDB();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __LFDatabase_H__ */
