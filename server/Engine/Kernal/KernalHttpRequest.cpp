
#include "KernalHttpRequest.h"

size_t http_data_writer(void *data, size_t size, size_t nmemb, void *content)
{
    long totalSize = size * nmemb;
    std::string *cBuffer = (std::string*)content;
    if(cBuffer)
    {
        cBuffer->append( (char*)data, ((char*)data) + totalSize );
    }
    return totalSize;
}

KernalHttpRequest::KernalHttpRequest()
    :m_pCurl(NULL)
{

}

KernalHttpRequest::~KernalHttpRequest()
{

}


bool KernalHttpRequest::init()
{
    m_pCurl = curl_easy_init();
    // 设置回调函数
    curl_easy_setopt( m_pCurl, CURLOPT_WRITEFUNCTION, http_data_writer );
    curl_easy_setopt( m_pCurl, CURLOPT_WRITEDATA, (void*)&m_httpData );
}

void KernalHttpRequest::uninit()
{
    curl_easy_cleanup( m_pCurl );  
}

std::string KernalHttpRequest::Request( const char *url )
{
    init();

    // 设置URL
    m_code = curl_easy_setopt( m_pCurl, CURLOPT_URL, url );
    // 执行请求
    m_code = curl_easy_perform( m_pCurl );
    
    uninit();

    return m_httpData;
}

