//
// Created by wave on 16/3/10.
//

#ifndef UDP_CLIENT_UDPREQUEST_H
#define UDP_CLIENT_UDPREQUEST_H

extern "C"{
#include "unp.h"
}

#include "util.h"
#include <vector>
#include <iostream>
using namespace std;
#define PACKAGE_CONTENT_LEN 3


enum RequestType {
    REQUEST_PACKAGE,        // I'm a package
    CHECK_RESPONSE,         // I've got your response package
    ASK_FOR_ANSWER
};


enum ResponseType {
    RESPONSE_PACKAGE,       // I'm a package
    CHECK_REQUEST,          // I've got your request package
    EMPTY_PACKAGE
};


struct RequestPackage {
    int             package_num;            // offset in package group
    int             tot_package_num;        // how many packages there are in current group
    RequestType     request_type;
    char            timestamp[100];         // unique group token
    char            content[PACKAGE_CONTENT_LEN+1];
};


struct ResponsePackage {
    int             package_num;            // if ResponseType == PAKAGE: offset in package group
                                            // if ResponseType == CHECK_REQUEST: offset of the request package
    int             tot_package_num;        // as above
    ResponseType    response_type;
    char            timestamp[100];         // always the timestamp of the corresponding request group
    char            content[PACKAGE_CONTENT_LEN+1];
};


class Query {

private:
    int     sockfd;
    string  raw_query_line = "";
    string  timestamp = get_timestamp();
    string  *answers;

    RequestPackage          string_to_package(string);
    vector<RequestPackage>  get_request_packages();
    ResponsePackage         block_for_response();
    ResponsePackage         send_request_package(RequestPackage *p_package, bool need_response);
    int                     send_request_packages(vector<RequestPackage>);

public:
    Query(int _sockfd, string _raw_query_line);
    ~Query() { delete []answers; }
    string                  get_answer();
};


#endif //UDP_CLIENT_UDPREQUEST_H
