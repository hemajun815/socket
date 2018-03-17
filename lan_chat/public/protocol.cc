/*************************************************
  Filename: protocol.cc
  Creator: Hemajun
  Description: Transport protocol.
*************************************************/
#include "protocol.h"
#include "cjson.h"

char * encode_package(const PPackage & data)
{
    cJSON * p_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(p_json, "package_type", (int)data->package_type);
    cJSON_AddStringToObject(p_json, "sender_name", data->sender_name);
    cJSON_AddStringToObject(p_json, "receiver_name", data->receiver_name);
    cJSON_AddStringToObject(p_json, "data", data->data);
    char * str_json = cJSON_PrintUnformatted(p_json);
    cJSON_Delete(p_json);
    return str_json;
}

PPackage decode_package(const char * data)
{
    cJSON * p_json = cJSON_Parse(data);
    PPackage p_package = new Package();
    p_package->package_type = (PackageType)cJSON_GetObjectItem(p_json, "package_type")->valueint;
    p_package->sender_name = cJSON_GetObjectItem(p_json, "sender_name")->valuestring;
    p_package->receiver_name = cJSON_GetObjectItem(p_json, "receiver_name")->valuestring;
    p_package->data = cJSON_GetObjectItem(p_json, "data")->valuestring;
    return p_package;
}