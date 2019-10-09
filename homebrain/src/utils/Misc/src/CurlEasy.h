/***************************************************************************
 *  CurlEasy.h -
 *
 *  Created: 2019-06-26 15:28:45
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __CurlEasy_H__
#define __CurlEasy_H__

#ifdef __cplusplus

#include <stdio.h>
#include <string>

namespace UTILS {

enum {
    eURL_HTTP = 1,
    eURL_FTP,
};

class CurlEasy {
public:
    virtual ~CurlEasy();

    void setVerbose(int verbose) { mVerbose = verbose; }
    void setTimeout(int timeout) { mTimeout = timeout; }
    void setConnectTimeout(int timeout) { mConnectTimeout = timeout; }

    static size_t CURL_ReadCallback(void *ptr, size_t size, size_t nmemb, void *arg);
    static size_t CURL_WriteCallback(void  *ptr, size_t size, size_t nmemb, void *arg);
    static size_t CURL_ReadProgressCallback(void *arg, double dltotal, double dlnow, double ult, double uln);
    static size_t CURL_WriteProgressCallback(void *arg, double dltotal, double dlnow, double ult, double uln);

    int get(const char* url);
    int post(const char* url, const char* payload, size_t size);

    int put(const char* remote, const char* local, const char* user, const char* passwd);

protected:
    CurlEasy(int type);
    virtual int receiveData(char * /*buf*/, size_t /*size*/, size_t /*nmemb*/) { return 0; }
    virtual int receiveProgress(double /*total*/, double /*now*/) { return 0; }

    virtual int sendData(char * /*buf*/, size_t /*size*/, size_t /*nmemb*/) { return 0; }
    virtual int sendProgress(double /*total*/, double /*now*/) { return 0; }

private:
    int mType;
    int mVerbose;
    int mTimeout;
    int mConnectTimeout;
};

std::string easyGet(const char *url);
std::string easyPost(const char *url, const char *payload, size_t size);
int easyPut(const char *remote, const char *local_file, const char *user, const char *passwd);

}

#endif /* __cplusplus */

#endif /* __CurlEasy_H__ */

