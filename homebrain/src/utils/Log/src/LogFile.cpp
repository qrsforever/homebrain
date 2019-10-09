/***************************************************************************
 *  LogFile.cpp - Log File Impl
 *
 *  Created: 2018-06-04 10:08:34
 *
 *  Copyright QRS
 ****************************************************************************/

#include "LogFile.h"
#include "StringData.h"

#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH "/tmp"
#endif
#define LOG_FILE_NAME "homebrain.log"

namespace UTILS {

LogFile::LogFile(MessageHandler *handler, const char *path, size_t maxSize)
    : mH(handler), mFp(0), mSumSize(0), mMaxSize(maxSize)
{
    snprintf(mPath, 127, "%s", path ? path : LOG_FILE_PATH);
}

LogFile::~LogFile()
{
    if (mFp) {
        fflush(mFp);
        fclose(mFp);
    }

    mFp = 0;
}

bool LogFile::pushBlock(uint8_t* blockHead, uint32_t blockLength)
{
    if (mSumSize > mMaxSize) {
        fclose(mFp);
        mFp = 0;
        if (mH) {
            char filePath[192] = { 0 };
            snprintf(filePath, 191, "%s/%s", mPath, LOG_FILE_NAME);
#ifdef USE_SHARED_PTR
            std::shared_ptr<StringData> data = std::make_shared<StringData>(filePath);
            if (data)
                mH->sendMessage(mH->obtainMessage(MT_LOG, LOG_EVENT_FILE_READY, mSumSize, data));
#else
            StringData *data = new StringData(filePath);
            if (data) {
                mH->sendMessage(mH->obtainMessage(MT_LOG, LOG_EVENT_FILE_READY, mSumSize, data));
                data->safeUnref();
            }
#endif
        }
    }
    if (!mFp) {
        char filePath[192] = { 0 };
        snprintf(filePath, 191, "%s/%s", mPath, LOG_FILE_NAME);
        mFp = fopen(filePath, "w");
        mSumSize = 0;
        if (mH) {
#ifdef USE_SHARED_PTR
            std::shared_ptr<StringData> data = std::make_shared<StringData>(filePath);
            if (data)
                mH->sendMessage(mH->obtainMessage(MT_LOG, LOG_EVENT_FILE_CREATE, mMaxSize, data));
#else
            StringData *data = new StringData(filePath);
            if (data) {
                mH->sendMessage(mH->obtainMessage(MT_LOG, LOG_EVENT_FILE_CREATE, mMaxSize, data));
                data->safeUnref();
            }
#endif
        }
    }
    if (mFp) {
        fwrite(blockHead, blockLength, 1, mFp);
        //fflush(mFp);
    }
    mSumSize += blockLength;
    return false;
}

} /* namespace UTILS */
