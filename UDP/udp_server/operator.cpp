//
// Created by wave on 16/4/1.
//

#include "operator.h"
using namespace std;

/************************************ Public ************************************/

Operator::Operator(int sockfd, SA *pcliaddr, socklen_t clilen) {
    this->sockfd = sockfd;
    this->pcliaddr = pcliaddr;
    this->clilen = clilen;
}


Operator::~Operator() {
    map<string, string*>::iterator it;
    for (it = timestamp_to_rawstrings.begin(); it != timestamp_to_rawstrings.end(); it++) {
        if (it->second) {
            cout << "deleting key: " << it->first << endl;
            delete []it->second;
        }
    }
}


void Operator::run() {
    socklen_t len;
    char buffer[MAXLINE];

    while (TRUE){
        bzero(buffer, MAXLINE);
        len = clilen;
        cout << "waiting..." << endl;
        Recvfrom(sockfd, buffer, MAXLINE, 0, pcliaddr, &len);
        RequestPackage request_package = *((RequestPackage*)buffer);
        cout << "request content is " << request_package.content << endl;
        ResponsePackage response_package;

        switch (request_package.request_type) {

            case REQUEST_PACKAGE:
                add_rawstring(request_package);
                response_package.response_type = CHECK_REQUEST;
                response_package.package_num = request_package.package_num;
                response_package.tot_package_num = request_package.tot_package_num;
                strcpy(response_package.timestamp, request_package.timestamp);
                strcpy(response_package.content, request_package.content);
                send_package(&response_package);
                break;

            case CHECK_RESPONSE:
                break;

            case ASK_FOR_ANSWER:
                cout << "sending answer now" << endl;
                string url_string = get_url_string(request_package);
                vector<ResponsePackage> answer_packages = get_answer_packages(url_string, request_package.timestamp);
                send_response_packages(answer_packages);
                break;
        }

        if (string(request_package.content) == "bye\n")
            break;
    }
}

/************************************ Private ************************************/

void Operator::add_rawstring(RequestPackage request_package) {
    // a new request session
    string timestamp = string(request_package.timestamp);
    if (timestamp_to_rawstrings.find(timestamp) == timestamp_to_rawstrings.end()) {
        timestamp_to_rawstrings[timestamp] = new string[request_package.tot_package_num];
    }
    // add raw string anyway
    cout << "adding raw string: " << timestamp << ' ' << request_package.content << endl;
    timestamp_to_rawstrings[timestamp][request_package.package_num] = string(request_package.content);
}


string Operator::get_url_string(RequestPackage ask_for_answer_package) {
    string timestamp = string(ask_for_answer_package.timestamp);
    cout << "getting url:" << timestamp << endl;
    int tot_package_num = ask_for_answer_package.tot_package_num;
    cout << "package num = " << tot_package_num << ' ' << timestamp_to_rawstrings[timestamp][0] << endl;
    string raw_string = "";
    for (int i = 0; i < tot_package_num; i++) {
        cout << i << endl;
        raw_string += timestamp_to_rawstrings[timestamp][i];
    }

    return urlencode(raw_string);
}


ResponsePackage Operator::string_to_package(string s, string timestamp) {

    if (s.size() > PACKAGE_CONTENT_LEN) {
        cout << "error in string_to_package" << endl;
        return *((ResponsePackage*)NULL);
    }

    ResponsePackage request_package;
    request_package.package_num       = -1;
    request_package.tot_package_num   = -1;
    request_package.response_type      = RESPONSE_PACKAGE;
    strcpy(request_package.content,   s.c_str() );
    strcpy(request_package.timestamp, timestamp.c_str());

    return request_package;
}


vector<ResponsePackage> Operator::get_answer_packages(string url_string, string timestamp) {

    vector<ResponsePackage> packages;

    int start_pos = 0;
    string tem = url_string.substr(start_pos, PACKAGE_CONTENT_LEN);

    while (tem.size() == PACKAGE_CONTENT_LEN){
        packages.push_back(string_to_package(tem, timestamp));
        start_pos += PACKAGE_CONTENT_LEN;

        if (start_pos >= url_string.length()) break;

        tem = url_string.substr(start_pos, PACKAGE_CONTENT_LEN);
    }

    if (start_pos < url_string.length() && tem.length() > 0)
        packages.push_back(string_to_package(tem, timestamp));

    int tot_package_num = packages.size();
    for (int i = 0; i < tot_package_num; i++){
        packages[i].package_num = i;
        packages[i].tot_package_num = tot_package_num;
    }

    return packages;
}


void Operator::send_package(ResponsePackage *p_package) {
    Sendto(sockfd, (char*) p_package, sizeof(*p_package), 0, pcliaddr, clilen);
}


void Operator::send_response_packages(vector<ResponsePackage> response_packages) {
    int tot_package_num = (int)response_packages.size();

    // Send packages
    int current_package_num = 0;    // The first unchecked package number
    char recvline[MAXLINE+1];

    while (true) {
        // Send the current package
        ResponsePackage current_response_package = response_packages[current_package_num];

        cout << "sending " << current_response_package.content << endl;

        send_package(&current_response_package);

        // Get response
        bzero(recvline, sizeof(recvline));
        Read(sockfd, recvline, MAXLINE+1);

        // Timeout?

        // Check current request package
        RequestPackage check_package = *((RequestPackage*)recvline);
        printf("[%d] %s\n", check_package.request_type, check_package.content);

        if (check_package.request_type != CHECK_RESPONSE
            || check_package.package_num != current_response_package.package_num
            || strcmp(check_package.content, current_response_package.content) != 0){
            cout << "type wrong" << check_package.request_type << ' ' << CHECK_REQUEST << endl;
            break;
        }

        cout << "ok" << endl;

        if (check_package.package_num == tot_package_num - 1)
            break;

        // move to next package
        current_package_num ++;
    }
}
