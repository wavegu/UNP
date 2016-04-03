//
// Created by wave on 16/3/10.
//

#ifndef UDP_CLIENT_UTIL_H
#define UDP_CLIENT_UTIL_H
#include <string>
#include <time.h>
#include <setjmp.h>
#include <iostream>
#include<cstdlib>
#include<ctime>

#define random(x)(rand()%x)

std::string get_timestamp();

std::string urlencode(std::string);

bool is_package_missing();


static sigjmp_buf jumpbuf;

static void sig_alarm(int signo) {
    siglongjmp(jumpbuf, 1);
}

#endif //UDP_CLIENT_UTIL_H
