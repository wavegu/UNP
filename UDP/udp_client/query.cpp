//
// Created by wave on 16/3/10.
//

#include "query.h"


Query::Query(int _sockfd, string _raw_sendline): sockfd(_sockfd), raw_query_line(_raw_sendline) { }


string Query::get_answer() {

    string answer = "";
    vector<ResponsePackage> response_packages;

    // Split result string into request packages
    vector<RequestPackage> request_packages = get_request_packages();

    // Send them all
    send_request_packages(request_packages);

    // Now all request packages are checked by server, time to ask for the answer
    RequestPackage ask_for_answer_package;
    ask_for_answer_package.tot_package_num = request_packages.size();
    ask_for_answer_package.request_type = ASK_FOR_ANSWER;
    strcpy(ask_for_answer_package.timestamp, timestamp.c_str());
    cout << int(ask_for_answer_package.request_type) << "Asking for answer" << endl;
    send_package(&ask_for_answer_package);

    return answer;
}


RequestPackage Query::string_to_package(string s) {

    if (s.size() > PACKAGE_CONTENT_LEN) {
        cout << "error in string_to_package" << endl;
        return *((RequestPackage*)NULL);
    }

    RequestPackage request_package;
    request_package.package_num       = -1;
    request_package.tot_package_num   = -1;
    request_package.request_type      = REQUEST_PACKAGE;
    strcpy(request_package.content,   s.c_str() );
    strcpy(request_package.timestamp, timestamp.c_str());

    return request_package;
}


vector<RequestPackage> Query::get_request_packages() {

    vector<RequestPackage> packages;

    int start_pos = 0;
    string tem = raw_query_line.substr(start_pos, PACKAGE_CONTENT_LEN);

    while (tem.size() == PACKAGE_CONTENT_LEN){
        packages.push_back(string_to_package(tem));
        start_pos += PACKAGE_CONTENT_LEN;

        if (start_pos >= raw_query_line.length()) break;

        tem = raw_query_line.substr(start_pos, PACKAGE_CONTENT_LEN);
    }

    if (start_pos < raw_query_line.length() && tem.length() > 0)
        packages.push_back(string_to_package(tem));

    int tot_package_num = packages.size();
    for (int i = 0; i < tot_package_num; i++){
        packages[i].package_num = i;
        packages[i].tot_package_num = tot_package_num;
    }

    return packages;
}


void Query::send_package(RequestPackage *p_package) {
    Write(sockfd, (char*) p_package, sizeof(*p_package));
}


void Query::send_request_packages(vector<RequestPackage> request_packages) {
    int tot_package_num = (int)request_packages.size();

    // Send packages
    int current_package_num = 0;    // The first unchecked package number
    char recvline[MAXLINE+1];

    while (true) {
        // Send the current package
        RequestPackage current_request_package = request_packages[current_package_num];

        cout << "sending " << current_request_package.content << endl;

        send_package(&current_request_package);

        // Get response
        bzero(recvline, sizeof(recvline));
        Read(sockfd, recvline, MAXLINE+1);

        // Timeout?

        // Check current request package
        ResponsePackage response_package = *((ResponsePackage*)recvline);
        printf("[%d] %s\n", response_package.response_type, response_package.content);

        if (response_package.response_type != CHECK_REQUEST
            || response_package.package_num != current_request_package.package_num
            || strcmp(response_package.content, current_request_package.content) != 0){
            cout << "type wrong" << response_package.response_type << ' ' << CHECK_REQUEST << endl;
            continue;
        }

        cout << "ok" << endl;

        if (response_package.package_num == tot_package_num - 1)
            break;

        // move to next package
        current_package_num ++;
    }
}
