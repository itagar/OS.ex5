// TODO: Check every system call.
// TODO: Make header file and move shared functions and macros.



/**
 * @file whatsappClient.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Client.
 */


/*-----=  Includes  =-----*/


#include <cstring>
#include <unistd.h>
#include "WhatsApp.h"


// TODO: Doxygen.
static int callSocket(const char *hostName, const portNumber_t portNumber)
{
    // Hostent initialization.
    hostent *pHostent = gethostbyname(hostName);
    if (pHostent == nullptr)
    {
        systemCallError("gethostbyname", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }

    // Socket Address initialization.
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sockaddr_in));
    sa.sin_family = AF_INET;  // TODO: Maybe: sa.sin_family = hp->h_addrtype.
    memcpy(&sa.sin_addr, pHostent->h_addr, (size_t) pHostent->h_length);
    sa.sin_port = htons(portNumber);

    // Create Socket.
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID < 0)
    {
        systemCallError("socket", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }
    if (connect(socketID, (sockaddr *) &sa, sizeof(sockaddr_in)))
    {
        if (close(socketID))
        {
            systemCallError("close", errno);  // TODO: Magic Number.
            return FAILURE_STATE;  // TODO: check return value/exit.
        }
        systemCallError("bind", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }


    return socketID;
}

