#ifndef RTN_CODE
#define RTN_CODE

#include <iostream>

namespace rtn {
    int SUCCESS = 1000;
    int RECV_TIMEOUT = 1001;
    int SEND_PACKAGE_ERROR = 1002;
    int RECV_PACKAGE_ERROR = 1003;
    int PACKAGE_INCOMPLETE = 1004;
    int PACKAGE_IDENT_INCONSISTENT = 1005;
    int PACKAGE_CKSUM_INCONSISTENT = 1006;
}

namespace ptype {
    uint16_t CLIENT_CMD_PACKAGE = 1;
    uint16_t SERVER_CMD_PACKAGE = 2;
    uint16_t SERVER_DATA_PACKAGE = 3;
}

#endif // RTN_CODE

