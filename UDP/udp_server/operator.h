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
    REQUEST_PACKAGE,        // I'm a request package
    RESPONSE_PACKAGE,       // I'm a response package
    CHECK_REQUEST,          // I've got your request package
    CHECK_RESPONSE,         // I've got your response package
    ASK_FOR_ANSWER,         // for client to ask for an answer after all request packages are checked
    EMPTY_PACKAGE,          //
    END_OF_SESSION,         // In case the last ack from client is lost, the client will send an END_OF_SESSION package if server is resending the last answer package
    ENOUGH_RESENT           // when resending time goes over limit
};


struct Package {
    int             package_num;                    // offset in package group
    int             tot_package_num;                // how many packages there are in current group
    PackageType     package_type;                   // type of the package
    char            timestamp[100];                 // unique group token
    char            content[PACKAGE_CONTENT_LEN+1]; // literal content
};


/* An operator is someone who picks up and handles phone(udp) calls */

class Operator {
private:
    /* socket config */
    int sockfd;
    SA *pcliaddr;
    socklen_t clilen;
    /* to store request packages from different time */
    std::map<std::string, std::string*> timestamp_to_rawstrings;

    void                    add_rawstring(Package);                                 // store the part of raw string a request package carries
    void                    send_answer_packages(std::vector<Package>);             // send answer(url encoded string) in the form of packages
    Package                 block_for_response();                                   // block to wait for a response package
    Package                 string_to_package(std::string, std::string);            // convert a string to an answer package
    Package                 send_package(Package *p_package, bool need_response);   // send a package. block for response and resend when timeout, if 'need_response == true'
    std::string             get_url_string(Package ask_for_answer_package);         // find the corresponding raw string group accroding to timestamp, and return the url encoded string
    std::vector<Package>    get_answer_packages(std::string url_string, std::string timestamp);      // split the answer string to packages with corresponding timestamp

public:
    Operator(int sockfd, SA *pcliaddr, socklen_t clilen);
    ~Operator();
    void run();         // start the operator to handle requests
};

#endif //UDP_SERVER_RESPONSE_H
