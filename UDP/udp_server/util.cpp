//
// Created by wave on 16/3/10.
//

#include "util.h"
#include <map>
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

//string urlencode(string raw_line){
//    map<char, string> url_map;
//    url_map['!'] = "%21";
//    url_map['#'] = "%23";
//    url_map['$'] = "%24";
//    url_map['&'] = "%26";
//    url_map['\''] = "%27";
//    url_map['('] = "%28";
//    url_map[')'] = "%29";
//    url_map['*'] = "%2A";
//    url_map['+'] = "%2B";
//    url_map[','] = "%2C";
//    url_map['/'] = "%2F";
//    url_map[':'] = "%3A";
//    url_map[';'] = "%3B";
//    url_map['='] = "%3D";
//    url_map['?'] = "%3F";
//    url_map['@'] = "%40";
//    url_map['['] = "%5B";
//    url_map[']'] = "%5D";
//    string result = "";
//    int len = raw_line.length();
//    for (int i = 0; i < len; i++) {
//        if (url_map.find(raw_line[i]) != url_map.end()) {
//            result += url_map[raw_line[i]];
//        } else {
//            result.append(1, raw_line[i]);
//        }
//    }
//    return result;
//}


bool is_package_missing() {
    int dice = random(10);
    if (dice < 3) {
        return true;
    }
    return false;
}

unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

string urlencode(string str)
{
    string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}