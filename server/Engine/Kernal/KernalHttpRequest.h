
#ifndef _KERNAL_HTTP_REQUEST_H_
#define _KERNAL_HTTP_REQUEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <curl/curl.h>

class KernalHttpRequest {
public:
    KernalHttpRequest();
    ~KernalHttpRequest();
    
    std::string Request(char *url);
private:
    bool init();
    void uninit();
private:
    CURL        *m_pCurl;
    CURLcode     m_code;
    std::string  m_httpData;
};

#endif
