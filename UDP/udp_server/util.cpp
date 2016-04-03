//
// Created by wave on 16/3/10.
//

#include "util.h"
using namespace std;

string get_timestamp(){
    time_t timep;
    struct tm *p;

    time(&timep); /*获得time_t结构的时间，UTC时间*/
    p = localtime(&timep); /*转换为struct tm结构的当地时间*/
    string ts = "";
    ts += string(to_string(1900 + p->tm_year) + '/');
    ts += string(to_string(1 + p->tm_mon) + '/');
    ts += string(to_string(p->tm_mday) + '-');
    ts += string(to_string(p->tm_hour) + ':');
    ts += string(to_string(p->tm_min) + ':');
    ts += string(to_string(p->tm_sec));

    return ts;
}

string urlencode(string raw_line){
    raw_line = raw_line.substr(0, raw_line.find('\n'));
    string result = "urlencoding:" + raw_line;
    return result;
}


bool is_package_missing() {
    int dice = random(10);
    if (dice < 3) {
        return true;
    }
    return false;
}