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
#define WAIT_TIME 2
#define PACKAGE_CONTENT_LEN 3


enum PackageType {
    REQUEST_PACKAGE,        // I'm a package
    RESPONSE_PACKAGE,       // I'm a package
    CHECK_REQUEST,          // I've got your request package
    CHECK_RESPONSE,         // I've got your response package
    ASK_FOR_ANSWER,
    EMPTY_PACKAGE,
    END_OF_SESSION
};


struct Package {
    int             package_num;            // offset in package group
    int             tot_package_num;        // how many packages there are in current group
    PackageType     package_type;
    char            timestamp[100];         // unique group token
    char            content[PACKAGE_CONTENT_LEN+1];
};

class Query {

private:
    int     sockfd;
    string  raw_query_line = "";
    string  timestamp = get_timestamp();
    string  *answers;

    Package             string_to_package(string);
    vector<Package>     get_request_packages();
    Package             block_for_response();
    Package             send_package(Package *p_package, bool need_response);
    int                 send_request_packages(vector<Package>);

public:
    Query(int _sockfd, string _raw_query_line);
    ~Query() { delete []answers; }
    string                  get_answer();
};


#endif //UDP_CLIENT_UDPREQUEST_H
