/***************************************************************************
 *  LogFile.h - Log File Header
 *
 *  Created: 2018-06-04 10:06:52
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __LogFile_H__
#define __LogFile_H__

#include "LogFilter.h"
#include "MessageHandler.h"

#include <stdio.h>

#ifdef __cplusplus

namespace UTILS {

class LogFile : public LogFilter {
public:
    LogFile(MessageHandler *handler = 0, const char* path = 0, size_t logMaxSize = 10240000);
    virtual ~LogFile();
    virtual bool pushBlock(uint8_t* blockHead, uint32_t blockLength);
private:
    MessageHandler *mH;
    FILE *mFp;
    size_t mSumSize;
    size_t mMaxSize;
    char mPath[128];
}; /* class LogFile */

} /* namespace UTILS */

#endif /* __cplusplus */

#endif /* __LogFile_H__ */
