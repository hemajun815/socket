/*************************************************
  Filename: protocol.h
  Creator: Hemajun
  Description: Transport protocol.
*************************************************/
#ifndef PROTOCOL_H
#define PROTOCOL_H

enum PackageType
{
    PackageTypeRequestLogin,
    PackageTypeResponceLogin,
    PackageTypeMessage,
    PackageTypeRequestLogout,
    PackageTypeResponceLogout
};

typedef struct Package
{
    PackageType package_type;
    char * sender_name;
    char * receiver_name;
    char * data;
}Package, *PPackage;

char * encode_package(const PPackage & data);
PPackage decode_package(const char * data);

#endif