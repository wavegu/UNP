#include <iostream>
using namespace std;

uint16_t calc_checksum(uint8_t* data, int len)
{
    uint16_t sum = 0;
    int i;
    for(i=0; i<len; i++){
        sum += data[i];
    }
    return sum;
}
