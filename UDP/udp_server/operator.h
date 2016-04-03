//
// Created by wave on 16/4/1.
//

#ifndef UDP_SERVER_RESPONSE_H
#define UDP_SERVER_RESPONSE_H

extern "C"{
#include "unp.h"
}

#include "util.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>

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

class Operator {
private:
    int sockfd;
    SA *pcliaddr;
    socklen_t clilen;
    std::map<std::string, std::string*> timestamp_to_rawstrings;

    void add_rawstring(RequestPackage);
    void send_package(ResponsePackage*);
    void send_response_packages(std::vector<ResponsePackage>);
    ResponsePackage string_to_package(std::string, std::string);
    std::string get_url_string(RequestPackage);
    std::vector<ResponsePackage> get_answer_packages(std::string, std::string);

public:
    Operator(int sockfd, SA *pcliaddr, socklen_t clilen);
    ~Operator();
    void run();
};

#endif //UDP_SERVER_RESPONSE_H