
#include "KernalJson.h"

KernalJson::KernalJson()
{

}

KernalJson::~KernalJson()
{

}

Json::Value KernalJson::parseJson( const char *data ) 
{
    Json::Reader reader;
    Json::Value  resp;

    if (!reader.parse(data, resp, false))
    {
        
    }
    return resp;
}

