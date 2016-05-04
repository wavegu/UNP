#ifndef RTN_CODE
#define RTN_CODE

#include <iostream>

namespace rtn {
    int SUCCESS = 1000;
    int RECV_TIMEOUT = 1001;
    int SEND_PACKAGE_ERROR = 1002;
    int RECV_PACKAGE_ERROR = 1003;
    int PACKAGE_INCOMPLETE = 1004;
    int PACKAGE_TYPE_ERROR = 1005;
    int UNEXPECTED_PACKAGE = 1006;
    int PACKAGE_IDENT_INCONSISTENT = 1007;
    int PACKAGE_CKSUM_INCONSISTENT = 1008;

    int CMD_DONT_EXIST  = 1009;
    int FILE_DONT_EXIST = 1010;
    int CLIENT_QUIT = 1011;
}

namespace ptype {
    uint16_t CLIENT_CMD_PACKAGE = 1;
    uint16_t SERVER_CMD_PACKAGE = 2;
    uint16_t CLIENT_DATA_PACKAGE = 3;
    uint16_t SERVER_DATA_PACKAGE = 4;
    uint16_t SERVER_EOF_PACKAGE = 5;
    uint16_t SERVER_BYE_PACKAGE = 6;
}

#endif // RTN_CODE

