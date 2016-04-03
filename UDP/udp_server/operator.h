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

#define WAIT_TIME 2
#define PACKAGE_CONTENT_LEN 3


enum PackageType {
    REQUEST_PACKAGE,        // I'm a package
    RESPONSE_PACKAGE,       // I'm a package
    CHECK_REQUEST,          // I've got your request package
    CHECK_RESPONSE,         // I've got your response package
    ASK_FOR_ANSWER,
    EMPTY_PACKAGE,
    END_OF_SESSION,
    ENOUGH_RESENT
};


struct Package {
    int             package_num;            // offset in package group
    int             tot_package_num;        // how many packages there are in current group
    PackageType     package_type;
    char            timestamp[100];         // unique group token
    char            content[PACKAGE_CONTENT_LEN+1];
};


class Operator {
private:
    int sockfd;
    SA *pcliaddr;
    socklen_t clilen;
    std::map<std::string, std::string*> timestamp_to_rawstrings;

    void                    add_rawstring(Package);
    void                    send_response_packages(std::vector<Package>);
    Package                 block_for_response();
    Package                 string_to_package(std::string, std::string);
    Package                 send_package(Package *p_package, bool need_response);
    std::string             get_url_string(Package);
    std::vector<Package>    get_answer_packages(std::string, std::string);

public:
    Operator(int sockfd, SA *pcliaddr, socklen_t clilen);
    ~Operator();
    void run();
};

#endif //UDP_SERVER_RESPONSE_H