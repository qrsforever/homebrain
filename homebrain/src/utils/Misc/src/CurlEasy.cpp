/***************************************************************************
 *  CurlEasy.cpp -
 *
 *  Created: 2019-06-26 15:28:52
 *
 *  Copyright QRS
 ****************************************************************************/

#include "CurlEasy.h"

#include "curl/curl.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

namespace UTILS {

CurlEasy::CurlEasy(int type)
    : mType(type), mVerbose(0), mTimeout(0), mConnectTimeout(5)
{
    // curl_global_init(CURL_GLOBAL_ALL);
}

CurlEasy::~CurlEasy()
{

}

size_t CurlEasy::CURL_ReadCallback(void *ptr, size_t size, size_t nmemb, void *arg)
{
    FILE *fp = (FILE*)arg;
    if (ferror(fp))
        return CURL_READFUNC_ABORT;
    return fread(ptr, size, nmemb, fp);
}

size_t CurlEasy::CURL_ReadProgressCallback(void *arg, double /*dltotal*/, double /*dlnow*/, double ult, double uln)
{
    CurlEasy* self = (CurlEasy*)arg;
    if (self)
        return self->sendProgress(ult, uln);
    return -1;
}

size_t CurlEasy::CURL_WriteCallback(void *ptr,size_t size, size_t nmemb, void *arg)
{
    CurlEasy* self = (CurlEasy*)arg;
    if (self)
        return self->receiveData((char*)ptr, size, nmemb);
    return 0;
}

size_t CurlEasy::CURL_WriteProgressCallback(void *arg, double dltotal, double dlnow, double /*ult*/, double /*uln*/)
{
    CurlEasy* self = (CurlEasy*)arg;
    if (self)
        return self->receiveProgress(dltotal, dlnow);
    return -1;
}

int CurlEasy::get(const char* url)
{
    CURLcode res= CURLE_OK;
    CURL* lCurl = 0;

    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if(!lCurl)
        return -1 * CURL_LAST;

    //remove special characters
    int b = strspn(url, "\n\r\f\t ");
    int e = strcspn(url + b, "\n\r\f\t ");
    char turl[256] = { 0 };
    strncpy(turl, url + b, e);

    curl_easy_setopt(lCurl, CURLOPT_URL, turl);

    curl_easy_setopt(lCurl, CURLOPT_VERBOSE, mVerbose);
    curl_easy_setopt(lCurl, CURLOPT_TIMEOUT, mTimeout);
    curl_easy_setopt(lCurl, CURLOPT_CONNECTTIMEOUT, mConnectTimeout);

    if (!strncmp(turl, "https", 5)) {
        curl_easy_setopt(lCurl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(lCurl, CURLOPT_SSL_VERIFYHOST, false);
    }

    curl_easy_setopt(lCurl, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(lCurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(lCurl, CURLOPT_MAXREDIRS, 1);

    curl_easy_setopt(lCurl, CURLOPT_WRITEFUNCTION, CurlEasy::CURL_WriteCallback);
    curl_easy_setopt(lCurl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(lCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(lCurl, CURLOPT_PROGRESSFUNCTION, CurlEasy::CURL_WriteProgressCallback);
    curl_easy_setopt(lCurl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(lCurl, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(lCurl, CURLOPT_NOSIGNAL, 1);

    res = curl_easy_perform(lCurl); /* block */

    curl_easy_cleanup(lCurl);

    if (res != 0)
        printf("Error:%d \n", res);
    // 28 = CURLE_OPERATION_TIMEDOUT
    return res == CURLE_OK ? 0 : (-1 * res);
}

int CurlEasy::post(const char* url, const char* payload, size_t size)
{
    CURLcode res= CURLE_OK;
    CURL* lCurl = 0;
    // struct curl_slist *headers = NULL;
    // headers = curl_slist_append(headers, "Accept: application/json");
    // headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");

    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if(!lCurl)
        return -1 * CURL_LAST;

    //remove special characters
    int b = strspn(url, "\n\r\f\t ");
    int e = strcspn(url + b, "\n\r\f\t ");
    char turl[256] = { 0 };
    strncpy(turl, url + b, e);

    curl_easy_setopt(lCurl, CURLOPT_URL, turl);

    curl_easy_setopt(lCurl, CURLOPT_POST, 1);
    // curl_easy_setopt(lCurl, CURLOPT_HEADER, 1);
    // curl_easy_setopt(lCurl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(lCurl, CURLOPT_POSTFIELDS, payload);
    if (size > 0)
        curl_easy_setopt(lCurl, CURLOPT_POSTFIELDSIZE, size);

    if (!strncmp(turl, "https", 5)) {
        curl_easy_setopt(lCurl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(lCurl, CURLOPT_SSL_VERIFYHOST, false);
    }

    curl_easy_setopt(lCurl, CURLOPT_VERBOSE, mVerbose);
    curl_easy_setopt(lCurl, CURLOPT_TIMEOUT, mTimeout);
    curl_easy_setopt(lCurl, CURLOPT_CONNECTTIMEOUT, mConnectTimeout);

    curl_easy_setopt(lCurl, CURLOPT_AUTOREFERER, 1);
    curl_easy_setopt(lCurl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(lCurl, CURLOPT_MAXREDIRS, 1);

    curl_easy_setopt(lCurl, CURLOPT_WRITEFUNCTION, CurlEasy::CURL_WriteCallback);
    curl_easy_setopt(lCurl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(lCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(lCurl, CURLOPT_PROGRESSFUNCTION, CurlEasy::CURL_WriteProgressCallback);
    curl_easy_setopt(lCurl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(lCurl, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(lCurl, CURLOPT_NOSIGNAL, 1);

    res = curl_easy_perform(lCurl); /* block */

    // curl_slist_free_all(headers);
    curl_easy_cleanup(lCurl);

    if (res != 0)
        printf("Error:%d \n", res);
    // 28 = CURLE_OPERATION_TIMEDOUT
    return res == CURLE_OK ? 0 : (-1 * res);
}

int CurlEasy::put(const char* remote_url, const char* local_file, const char* user, const char* passwd)
{
    FILE *fp = 0;
    struct stat file_info;
    curl_off_t fsize;
    CURLcode res= CURLE_OK;
    CURL* lCurl = 0;

    if (stat(local_file, &file_info)) {
        printf("Couldn't open '%s': %s\n", local_file, strerror(errno));
        return -1;
    }
    fsize = (curl_off_t)file_info.st_size;
    fp = fopen(local_file, "rb");

    lCurl = curl_easy_init(); /* start a libcurl easy session */
    if(!lCurl)
        return -1 * CURL_LAST;

    char user_key[1024] = {0};
    sprintf(user_key, "%s:%s", user, passwd);

    curl_easy_setopt(lCurl, CURLOPT_URL, remote_url);
    curl_easy_setopt(lCurl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(lCurl, CURLOPT_USERPWD, user_key);
    curl_easy_setopt(lCurl, CURLOPT_VERBOSE, mVerbose);
    curl_easy_setopt(lCurl, CURLOPT_CONNECTTIMEOUT, mConnectTimeout);
    curl_easy_setopt(lCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, mTimeout);
    curl_easy_setopt(lCurl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1);
    curl_easy_setopt(lCurl, CURLOPT_READFUNCTION, CurlEasy::CURL_ReadCallback);
    curl_easy_setopt(lCurl, CURLOPT_READDATA, fp);
    curl_easy_setopt(lCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
    curl_easy_setopt(lCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(lCurl, CURLOPT_PROGRESSFUNCTION, CurlEasy::CURL_ReadProgressCallback);
    curl_easy_setopt(lCurl, CURLOPT_PROGRESSDATA, this);

    res = curl_easy_perform(lCurl);
    curl_easy_cleanup(lCurl);
    if (res != 0)
        printf("Error:%d \n", res);
    /* 25 - failed upload "command" */

    fclose(fp);
    return res == CURLE_OK ? 0 : (-1 * res);
}

class EasyRequest: public CurlEasy {
public:
    EasyRequest(): CurlEasy(eURL_HTTP), mResponse("") {}
    int receiveData(char *buf, size_t size, size_t nmemb);
    int ftpPut(const char* remote, const char* local, const char* user, const char* passwd);
    void run();
    std::string mResponse;
};

int EasyRequest::receiveData(char *buf, size_t size, size_t nmemb)
{
    size_t len = size * nmemb;
    mResponse.assign(buf, len);
    return len;
}

std::string easyGet(const char *url)
{
    EasyRequest req;
    if (0 != req.get(url))
        return std::string();
    return req.mResponse;
}

std::string easyPost(const char *url, const char *payload, size_t size)
{
    EasyRequest req;
    if (0 != req.post(url, payload, size))
        return std::string();
    return req.mResponse;
}

int easyPut(const char *remote, const char *local_file, const char *user, const char *passwd)
{
    // warning block
    EasyRequest req;
    req.setTimeout(30L);
    // req.setVerbose(true);

    const char* bsname = strrchr(local_file, '/');
    if (bsname )
        bsname = bsname  + 1;
    else
        bsname = local_file;

    char url[256] = { 0 };

    snprintf(url, 255, "%s/%s", remote, bsname);

    return req.put(url, local_file, user, passwd);
}

}

