#ifndef PACKAGE_H
#define PACKAGE_H

#include<iostream>

#define VERSION 0
#define TIMEOUT 1
#define IDENT_RQ_BIT 0x8000
#define PACKAGE_HEAD_SIZE 8
#define PACKAGE_DATA_SIZE 1024


struct Package
{
    uint8_t version;
    uint8_t auth_protocol;
    uint16_t ident;
    uint16_t checksum;
    uint16_t package_type;
    char data[PACKAGE_DATA_SIZE];
};



#endif
