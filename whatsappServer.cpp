// TODO: Check every system call.


/**
 * @file whatsappServer.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Server.
 */


/*-----=  Includes  =-----*/


#include <iostream>
#include <string>
#include <cassert>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <climits>
#include <cstring>


/*-----=  Definitions  =-----*/


/**
 * @def SUCCESS_STATE 0
 * @brief A Macro that sets the value indicating success state in the server.
 */
#define SUCCESS_STATE 0

/**
 * @def FAILURE_STATE -1
 * @brief A Macro that sets the value indicating failure state in the server.
 */
#define FAILURE_STATE -1

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
 * @def ERROR_MSG_SEPARATOR " "
 * @brief A Macro that sets the error message separator.
 */
#define ERROR_MSG_SEPARATOR " "

/**
 * @def SYSTEM_CALL_ERROR_MSG_PREFIX "ERROR:"
 * @brief A Macro that sets the error message prefix for a system called fail.
 */
#define SYSTEM_CALL_ERROR_MSG_PREFIX "ERROR:"

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


/*-----=  Type Definitions  =-----*/


// TODO: Doxygen.
typedef unsigned short portNumber_t;



static void systemCallError(const std::string callName, const int errorNumber)
{
    // TODO: Check cout or cerr.
    std::cerr << SYSTEM_CALL_ERROR_MSG_PREFIX << ERROR_MSG_SEPARATOR << callName
              << ERROR_MSG_SEPARATOR << errorNumber << std::endl;
}

/**
 * @brief Checks whether the program received the desired number of arguments.
 *        In case of invalid arguments count the function output an error
 *        message specifying the correct usage.
 * @param argc The number of arguments given to the program.
 * @param argv The array of given arguments.
 */
static void checkArguments(int const argc, char * const argv[])
{
    // Check valid number of arguments.
    if (argc != VALID_ARGUMENTS_COUNT)
    {
        // TODO: Check cout or cerr.
        // TODO: Check new line.
        std::cout << USAGE_MSG << std::endl;
        return;
    }

    // Check valid port number.
    std::string portNum = argv[PORT_ARGUMENT_INDEX];
    for (int i = 0; i < portNum.length(); ++i)
    {
        if (!isdigit(portNum[i]))
        {
            std::cout << USAGE_MSG << std::endl;
            return;
        }
    }

    return;
}


static int establish(const portNumber_t portNumber)
{
    // Hostent initialization.
    char hostName[HOST_NAME_MAX + NULL_TERMINATOR_COUNT];
    if (gethostname(hostName, HOST_NAME_MAX))
    {
        systemCallError("gethostname", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }
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
    if (bind(socketID, (sockaddr *) &sa, sizeof(sockaddr_in)))
    {
        if (close(socketID))
        {
            systemCallError("close", errno);  // TODO: Magic Number.
            return FAILURE_STATE;  // TODO: check return value/exit.
        }
        systemCallError("bind", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }

    if (listen(socketID, MAX_PENDING_CONNECTIONS))
    {
        systemCallError("listen", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }

    return socketID;
}


static int getConnection(const int socketID)
{
    int newSocket = accept(socketID, nullptr, nullptr);
    if (newSocket < 0)
    {
        systemCallError("accept", errno);  // TODO: Magic Number.
        return FAILURE_STATE;  // TODO: check return value/exit.
    }

    return newSocket;

}



static int readData(const int socketID, char *buffer, const size_t count)
{
    int totalCount = INITIAL_READ_COUNT;

    while (totalCount < count)
    {
        ssize_t currentCount = read(socketID, buffer, (count - totalCount));
        if (currentCount < 0)
        {
            systemCallError("read", errno);  // TODO: Magic Number.
            return FAILURE_STATE;  // TODO: check return value/exit.
        }
        totalCount += currentCount;
        buffer += currentCount;
    }

    // TODO: Check if we are given a bigger count which reach EOF before finish.

    return totalCount;
}

/**
 * @brief The main function running the program.
 */
int main(int argc, char *argv[])
{
    checkArguments(argc, argv);
    portNumber_t portNumber = (portNumber_t) std::stoi(argv[PORT_ARGUMENT_INDEX]);
    int welcomeSocket = establish(portNumber);
    if (welcomeSocket < 0)
    {
        // TODO: error.
    }
}