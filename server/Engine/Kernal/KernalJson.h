
#ifndef _KERNAL_JSON_H_
#define _KERNAL_JSON_H_

#include <json/json.h>

class KernalJson {
public:
    KernalJson();
    ~KernalJson();

    Json::Value parseJson( const char *data );
private:

};

#endif

