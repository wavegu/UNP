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
        Package request_package = *((Package*)buffer);
        cout << "request content is " << request_package.content << endl;
        Package response_package;

        switch (request_package.package_type) {

            case REQUEST_PACKAGE: {
                cout << "getting request" << endl;
                add_rawstring(request_package);
                response_package.package_type = CHECK_REQUEST;
                response_package.package_num = request_package.package_num;
                response_package.tot_package_num = request_package.tot_package_num;
                strcpy(response_package.timestamp, request_package.timestamp);
                strcpy(response_package.content, request_package.content);
                // send ack, dont need response.
                // If this package lost, the client can resend the request package, doesn't hurt
                send_package(&response_package, false);
                break;
            }

            case CHECK_RESPONSE:
                break;

            case ASK_FOR_ANSWER: {
                cout << "sending answer now" << endl;
                string url_string = get_url_string(request_package);
                vector<Package> answer_packages = get_answer_packages(url_string, request_package.timestamp);
                send_response_packages(answer_packages);
                break;
            }

            default:
                break;
        }

        if (string(request_package.content) == "bye\n")
            break;
    }
}

/************************************ Private ************************************/

void Operator::add_rawstring(Package request_package) {
    // a new request session
    string timestamp = string(request_package.timestamp);
    if (timestamp_to_rawstrings.find(timestamp) == timestamp_to_rawstrings.end()) {
        timestamp_to_rawstrings[timestamp] = new string[request_package.tot_package_num];
    }
    // add raw string anyway
    cout << "adding raw string: " << timestamp << ' ' << request_package.content << endl;
    timestamp_to_rawstrings[timestamp][request_package.package_num] = string(request_package.content);
}


string Operator::get_url_string(Package ask_for_answer_package) {
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


Package Operator::string_to_package(string s, string timestamp) {

    if (s.size() > PACKAGE_CONTENT_LEN) {
        Package empty_package;
        empty_package.package_type = EMPTY_PACKAGE;
        cout << "error in string_to_package" << endl;
        return empty_package;
    }

    Package request_package;
    request_package.package_num       = -1;
    request_package.tot_package_num   = -1;
    request_package.package_type      = RESPONSE_PACKAGE;
    strcpy(request_package.content,   s.c_str() );
    strcpy(request_package.timestamp, timestamp.c_str());

    return request_package;
}


Package Operator::block_for_response() {
    char recvline[MAXLINE+1];
    bzero(recvline, sizeof(recvline));

    Read(sockfd, recvline, MAXLINE+1);  // block
    Package response_package;
    response_package = *((Package*)recvline);

    return response_package;
}


vector<Package> Operator::get_answer_packages(string url_string, string timestamp) {

    vector<Package> packages;

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


Package Operator::send_package(Package *p_package, bool need_response) {

    Signal(SIGALRM, sig_alarm);

sendpackage:

    Package response_package;
    response_package.package_type = EMPTY_PACKAGE;

    // simulate a package loss
    if (is_package_missing()) {
        cout << "oops, check package missing" << endl;
        return response_package;
    }

    // send the package
    Sendto(sockfd, (char*) p_package, sizeof(*p_package), 0, pcliaddr, clilen);

    // if dont need response, return
    if (! need_response) {
        return response_package;
    }


    alarm(WAIT_TIME);
    if (sigsetjmp(jumpbuf, 1) != 0) {
        cout << "TIME OUT" << endl;
        goto sendpackage;
    }

    // if need response
    response_package = block_for_response();
    alarm(0);
    return response_package;
}


void Operator::send_response_packages(vector<Package> response_packages) {
    int tot_package_num = (int)response_packages.size();

    // Send packages
    int current_package_num = 0;    // The first unchecked package number
    char recvline[MAXLINE+1];

    while (true) {
        // Send the current package
        Package current_response_package = response_packages[current_package_num];

        cout << "sending " << current_response_package.content << endl;

        // need response, and resend if timeout
        Package check_package = send_package(&current_response_package, true);

        printf("[%d] %s\n", check_package.package_type, check_package.content);

        if ((check_package.package_type != CHECK_RESPONSE && check_package.package_type != EMPTY_PACKAGE)
            || check_package.package_num != current_response_package.package_num
            || strcmp(check_package.content, current_response_package.content) != 0){
            cout << "type wrong" << check_package.package_type << ' ' << CHECK_REQUEST << ' ' << EMPTY_PACKAGE << endl;
            continue;
        }

        cout << "ok" << endl;

        if (check_package.package_num == tot_package_num - 1)
            break;

        // move to next package
        current_package_num ++;
    }
}
