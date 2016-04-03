//
// Created by wave on 16/3/10.
//

#include "query.h"


/************************************ Public ************************************/

Query::Query(int _sockfd, string _raw_sendline): sockfd(_sockfd), raw_query_line(_raw_sendline) { }


string Query::get_answer() {

    vector<ResponsePackage> response_packages;

    // Split result string into request packages
    vector<RequestPackage> request_packages = get_request_packages();

    // Send them all
    send_request_packages(request_packages);

    // Now all request packages are checked by server, time to ask for the answer
    cout << "All requests checked" << endl;
    RequestPackage ask_for_answer_package;
    ask_for_answer_package.tot_package_num = request_packages.size();
    ask_for_answer_package.request_type = ASK_FOR_ANSWER;
    strcpy(ask_for_answer_package.timestamp, timestamp.c_str());

    // send the ask_for_answer package, wait for the first answer package
    ResponsePackage answer_package;
    strcpy(answer_package.timestamp, string("a weird string").c_str());
    while (string(answer_package.timestamp) != timestamp) {
        answer_package = send_request_package(&ask_for_answer_package, true);
    }

    // now the response_package is the first answer package
    int current_answer_num = answer_package.package_num;
    int tot_answer_package_num = answer_package.tot_package_num;
    answers = new string[answer_package.tot_package_num];


    while (true) {

        current_answer_num = answer_package.package_num;
        answers[current_answer_num] = string(answer_package.content);
        cout << "answers[" << current_answer_num << "] = " << answers[current_answer_num] << endl;

        RequestPackage check_package;
        check_package.tot_package_num = tot_answer_package_num;
        check_package.package_num = answer_package.package_num;
        check_package.request_type = CHECK_RESPONSE;
        strcpy(check_package.content, answer_package.content);
        send_request_package(&check_package, false);

        if (current_answer_num >= tot_answer_package_num - 1) {
            break;
        }

        answer_package = block_for_response();
    }

    // put together the final answer
    string answer = "";
    for (int i = 0; i < tot_answer_package_num; i++) {
        answer += answers[i];
    }

    return answer;
}


/************************************ Private ************************************/

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


ResponsePackage Query::block_for_response() {
    char recvline[MAXLINE+1];
    bzero(recvline, sizeof(recvline));

    Read(sockfd, recvline, MAXLINE+1);  // block
    ResponsePackage response_package;
    response_package = *((ResponsePackage*)recvline);

    return response_package;
}


ResponsePackage Query::send_request_package(RequestPackage *p_package, bool need_response) {

    // send the package
    cout << "sending request package: " << p_package->content << endl;
    Write(sockfd, (char*) p_package, sizeof(*p_package));

    if (! need_response) {
        ResponsePackage empty_package;
        empty_package.response_type = EMPTY_PACKAGE;
        return empty_package;

    }

    // wait for response
    // TODO:timeout, resend
    ResponsePackage response_package = block_for_response();
    
    return response_package;
}


int Query::send_request_packages(vector<RequestPackage> request_packages) {
    int tot_package_num = (int)request_packages.size();

    // Send packages
    int current_package_num = 0;    // The first unchecked package number
    char recvline[MAXLINE+1];

    while (true) {
        // Send the current package
        RequestPackage current_request_package = request_packages[current_package_num];

        ResponsePackage response_package = send_request_package(&current_request_package, true);

        // Check current request package
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

    return 0;
}
