//
// Created by wave on 16/3/10.
//

#include "query.h"
#define MAX_RESEND_TIMES 10


/************************************ Public ************************************/

Query::Query(int _sockfd, string _raw_sendline): sockfd(_sockfd), raw_query_line(_raw_sendline) { }


string Query::get_answer() {

    vector<Package> response_packages;

    // Split result string into request packages
    vector<Package> request_packages = get_request_packages();

    // Send them all
    send_request_packages(request_packages);

    // Now all request packages are checked by server, time to ask for the answer
    cout << get_timestamp() << " | " << "All requests checked" << endl;
    Package ask_for_answer_package;
    ask_for_answer_package.tot_package_num = request_packages.size();
    ask_for_answer_package.package_type = ASK_FOR_ANSWER;
    strcpy(ask_for_answer_package.timestamp, timestamp.c_str());

    // send the ask_for_answer package, wait for the first answer package
    Package answer_package;
    strcpy(answer_package.timestamp, string("a weird string").c_str());
    while (string(answer_package.timestamp) != timestamp) {
        answer_package = send_package(&ask_for_answer_package, true);
    }

    // now the response_package is the first answer package
    int current_answer_num = answer_package.package_num;
    int tot_answer_package_num = answer_package.tot_package_num;
    answers = new string[answer_package.tot_package_num];


    while (true) {

        current_answer_num = answer_package.package_num;
        answers[current_answer_num] = string(answer_package.content);
        cout << get_timestamp() << " | " << "answers[" << current_answer_num << "] = " << answers[current_answer_num] << endl;

        Package check_package;
        check_package.tot_package_num = tot_answer_package_num;
        check_package.package_num = answer_package.package_num;
        check_package.package_type = CHECK_RESPONSE;
        strcpy(check_package.content, answer_package.content);
        send_package(&check_package, false);

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

Package Query::string_to_package(string s) {

    if (s.size() > PACKAGE_CONTENT_LEN) {
        cout << get_timestamp() << " | " << "error in string_to_package" << endl;
        return *((Package*)NULL);
    }

    Package request_package;
    request_package.package_num       = -1;
    request_package.tot_package_num   = -1;
    request_package.package_type      = REQUEST_PACKAGE;
    strcpy(request_package.content,   s.c_str() );
    strcpy(request_package.timestamp, timestamp.c_str());

    return request_package;
}


vector<Package> Query::get_request_packages() {

    vector<Package> packages;

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


Package Query::block_for_response() {
    char recvline[MAXLINE+1];
    bzero(recvline, sizeof(recvline));

    Read(sockfd, recvline, MAXLINE+1);  // block
    Package response_package;
    response_package = *((Package*)recvline);

    return response_package;
}


Package Query::send_package(Package *p_package, bool need_response) {

    Signal(SIGALRM, sig_alarm);

    int resend_times = 0;

sendpackage:

    Package response_package;
    response_package.package_type = CHECK_RESPONSE;

    resend_times++;
    if (resend_times > MAX_RESEND_TIMES) {
        response_package.package_type = ENOUGH_RESENT;
        return response_package;
    }

    // simulate a package loss
    bool missing_package = false;
    if (is_package_missing()) {
        cout << get_timestamp() << " | " << "oops, check package missing" << endl;
        missing_package = true;
    }

    // send the package
    cout << get_timestamp() << " | " << "sending request package: " << p_package->content << endl;

    if (! missing_package)
        Write(sockfd, (char*) p_package, sizeof(*p_package));

    if (! need_response) {
        return response_package;
    }

    // wait for response
    // TODO:timeout, resend
    alarm(WAIT_TIME);
    if (sigsetjmp(jumpbuf, 1) != 0) {
        cout << get_timestamp() << " | " << "TIME OUT" << endl;
        goto sendpackage;
    }

    response_package = block_for_response();
    alarm(0);
    return response_package;
}


int Query::send_request_packages(vector<Package> request_packages) {
    int tot_package_num = (int)request_packages.size();

    // Send packages
    int current_package_num = 0;    // The first unchecked package number
    char recvline[MAXLINE+1];

    while (true) {
        // Send the current package
        Package current_request_package = request_packages[current_package_num];

        Package response_package = send_package(&current_request_package, true);

        if (response_package.package_type == ENOUGH_RESENT) {
            break;
        }

        // if response package is from another timestamp
        if (string(response_package.timestamp) != timestamp) {
            cout << "sending ENDOFSESSION" << endl;
            Package ack;
            ack.package_type = END_OF_SESSION;
            ack.package_num = response_package.package_num;
            ack.tot_package_num = response_package.tot_package_num;
            strcpy(ack.content, response_package.content);
            strcpy(ack.timestamp, response_package.timestamp);
            send_package(&ack, false);
            continue;
        }

        // Check current request package
        if (response_package.package_type != CHECK_REQUEST
            || response_package.package_num != current_request_package.package_num
            || strcmp(response_package.content, current_request_package.content) != 0){
            cout << get_timestamp() << " | " << "type wrong" << response_package.package_type << ' ' << CHECK_REQUEST << endl;
            continue;
        }

        cout << get_timestamp() << " | " << "ok" << endl;

        if (response_package.package_num == tot_package_num - 1)
            break;

        // move to next package
        current_package_num ++;
    }

    return 0;
}















































