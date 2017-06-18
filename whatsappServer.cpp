// TODO: Check every system call.


/**
 * @file whatsappServer.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Server.
 */


/*-----=  Includes  =-----*/


#include <string>
#include <cassert>
#include <climits>
#include <cstring>
#include <limits.h>
#include "WhatsApp.h"


/*-----=  Definitions  =-----*/


/**
 * @def VALID_ARGUMENTS_COUNT 2
 * @brief A Macro that sets the number for valid arguments count.
 */
#define VALID_ARGUMENTS_COUNT 2

/**
 * @def PORT_ARGUMENT_INDEX 1
 * @brief A Macro that sets the index of the port argument to this program.
 */
#define PORT_ARGUMENT_INDEX 1

/**
 * @def INITIAL_READ_COUNT 0
 * @brief A Macro that sets the initial value of read byte count.
 */
#define INITIAL_READ_COUNT 0

/**
 * @def USAGE_MSG "Usage: whatsappServer portNum"
 * @brief A Macro that sets the error message when the usage is invalid.
 */
#define USAGE_MSG "Usage: whatsappServer portNum"

/**
 * @def NULL_TERMINATOR_COUNT 1
 * @brief A Macro that sets the count for a null terminator in a string.
 */
#define NULL_TERMINATOR_COUNT 1

/**
 * @def MAX_PENDING_CONNECTIONS 10
 * @brief A Macro that sets the maximal number of pending connections.
 */
#define MAX_PENDING_CONNECTIONS 10



/**
 * @brief Checks whether the program received the desired number of arguments.
 *        In case of invalid arguments the function output an error
 *        message specifying the correct usage.
 * @param argc The number of arguments given to the program.
 * @param argv The array of given arguments.
 * @return 0 if the arguments are valid, -1 otherwise.
 */
static int checkServerArguments(int const argc, char * const argv[])
{
    // Check valid number of arguments.
    if (argc != VALID_ARGUMENTS_COUNT)
    {
        // TODO: Check cout or cerr.
        // TODO: Check new line.
        std::cout << USAGE_MSG << std::endl;
        return FAILURE_STATE;
    }

    // Check valid port number.
    std::string portNum = argv[PORT_ARGUMENT_INDEX];
    for (int i = 0; i < portNum.length(); ++i)
    {
        if (!isdigit(portNum[i]))
        {
            std::cout << USAGE_MSG << std::endl;
            return FAILURE_STATE;
        }
    }

    return SUCCESS_STATE;
}

// TODO: Doxygen.
static int establish(const portNumber_t portNumber)
{
    // TODO: check return value in failure (or exit).
    // Hostent initialization.
    char hostName[HOST_NAME_MAX + NULL_TERMINATOR_COUNT] = {NULL};
    if (gethostname(hostName, HOST_NAME_MAX))
    {
        systemCallError(GETHOSTNAME_NAME, errno);
        return FAILURE_STATE;
    }
    hostent *pHostent = gethostbyname(hostName);
    if (pHostent == nullptr)
    {
        systemCallError(GETHOSTBYNAME_NAME, errno);
        return FAILURE_STATE;
    }

    // Socket Address initialization.
    sockaddr_in sa;
    memset(&sa, NULL, sizeof(sockaddr_in));
    sa.sin_family = AF_INET;  // TODO: Maybe: sa.sin_family = hp->h_addrtype.
    memcpy(&sa.sin_addr, pHostent->h_addr, (size_t) pHostent->h_length);
    sa.sin_port = htons(portNumber);

    std::cout << "IP: " << inet_ntoa(*((in_addr *)pHostent->h_addr)) << std::endl;

    // Create Socket.
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID < SOCKET_ID_BOUND)
    {
        systemCallError(SOCKET_NAME, errno);
        return FAILURE_STATE;
    }
    if (bind(socketID, (sockaddr *) &sa, sizeof(sockaddr_in)))
    {
        if (close(socketID))
        {
            systemCallError(CLOSE_NAME, errno);
            return FAILURE_STATE;
        }
        systemCallError(BIND_NAME, errno);
        return FAILURE_STATE;
    }

    // Listen.
    if (listen(socketID, MAX_PENDING_CONNECTIONS))
    {
        systemCallError(LISTEN_NAME, errno);
        return FAILURE_STATE;
    }

    return socketID;
}

// TODO: Doxygen.
static int getConnection(const int socketID)
{
    // TODO: check return value in failure (or exit).
    int newSocket = accept(socketID, NULL, NULL);
    if (newSocket < SOCKET_ID_BOUND)
    {
        systemCallError(ACCEPT_NAME, errno);
        return FAILURE_STATE;
    }
    return newSocket;
}

// TODO: Doxygen.
static int readData(const int socketID, char *buffer, const size_t count)
{
    int totalCount = INITIAL_READ_COUNT;

    while (totalCount < count)
    {
        ssize_t currentCount = read(socketID, buffer, (count - totalCount));
        if (currentCount < 0)
        {
            systemCallError(READ_NAME, errno);
            return FAILURE_STATE;  // TODO: check return value/exit.
        }
        totalCount += currentCount;
        buffer += currentCount;
    }

    // TODO: Check if we are given a bigger count which reach EOF before finish.

    return totalCount;
}

// TODO: Doxygen.
int main(int argc, char *argv[])
{
    // Check the server arguments.
    if (checkServerArguments(argc, argv))
    {
        return FAILURE_STATE;  // TODO: Check this return value.
    }

    // Set the port number and create a welcome socket with that port.
    portNumber_t portNumber = (portNumber_t) std::stoi(argv[PORT_ARGUMENT_INDEX]);
    int welcomeSocket = establish(portNumber);
    if (welcomeSocket < SOCKET_ID_BOUND)
    {
        return FAILURE_STATE;  // TODO: Check this return value.
    }

    std::cout << "Pending..." << std::endl;  // TODO: Delete This.
    int connectionSocket = getConnection(welcomeSocket);
    if (connectionSocket < SOCKET_ID_BOUND)
    {
        return FAILURE_STATE;  // TODO: Check this return value.
    }
    std::cout << "Connected!" << std::endl;  // TODO: Delete This.


    // TODO: EXIT Command.
    // TODO: manage connections using select.

    char clientMessage[2000];
    ssize_t readCount = 0;
    while( (readCount = recv(connectionSocket , clientMessage , 2000 , 0)) > 0 )
    {
        std::cout << clientMessage << std::endl;
    }


}