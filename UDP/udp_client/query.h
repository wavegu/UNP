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


/* a query with unique timestamp */

class Query {

private:
    /* socket config */
    int     sockfd;
    string  raw_query_line = "";
    /* unique timestamp */
    string  timestamp = get_timestamp();
    string  *answers;

    Package             string_to_package(string);  // convert a string to an answer package
    vector<Package>     get_request_packages();     // from query string construct request packages
    Package             block_for_response();       // block to wait for a response package
    Package             send_package(Package *p_package, bool need_response);   // send a package. block for response and resend when timeout, if 'need_response == true'
    int                 send_request_packages(vector<Package>);                 // send query string in the form of packages

public:
    Query(int _sockfd, string _raw_query_line);
    ~Query() { delete []answers; }
    string                  get_answer();           // public interface, return answer(url encoded) string
};


#endif //UDP_CLIENT_UDPREQUEST_H
