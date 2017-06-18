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
#include <vector>
#include <stdlib.h>
#include <libltdl/lt_system.h>
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


// TODO: INIT ALL GLOBAL VARIABLES.
std::vector<Client> clients;
int activeClients = 0;  // TODO: Magic Number.


enum ConnectionState {SUCCESS, FAILURE};

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
    sa.sin_family = (sa_family_t) pHostent->h_addrtype;
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

static int getMaxSocketID(int const welcomeSocketID)
{
    int maxID = welcomeSocketID;
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        if (maxID < i->socket)
        {
            maxID = i->socket;
        }
    }
    return maxID;
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

    fd_set readFDs;
    FD_ZERO(&readFDs);
    FD_SET(STDIN_FILENO, &readFDs);
    FD_SET(welcomeSocket, &readFDs);

    while (true)
    {

        fd_set currentFDs = readFDs;

        // Get the max socket ID for the select function.
        int maxSocketID = getMaxSocketID(welcomeSocket);
        // Select.
        int readyFD = select(maxSocketID + 1, &currentFDs, NULL, NULL, NULL);

        if (readyFD < 0)
        {
            // TODO: Error.
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &currentFDs))
        {
            std::cout << "INPUT" << std::endl;
            std::string currentInput;
            std::getline(std::cin, currentInput);

            if (currentInput.compare("EXIT") == 0)  // TODO: Magic Number.
            {
                // TODO: EXIT
                exit(EXIT_SUCCESS);
            }

        }

        else if (FD_ISSET(welcomeSocket, &currentFDs))
        {
            std::cout << "Welcome" << std::endl;
            int connectionSocket = getConnection(welcomeSocket);
            if (connectionSocket < SOCKET_ID_BOUND)
            {
                return FAILURE_STATE;  // TODO: Check this return value.
            }

            // TODO: Check name.

            bool validClient = true;
            char clientName[55] = {NULL};
            ConnectionState connectionState;
            read(connectionSocket, clientName, 55);
            std::cout << "CLIENT NAME: " << clientName << std::endl;

            // TODO: Make more efficient using find.
            // Check if the name is available.
            for (auto i = clients.begin(); i != clients.end(); ++i)
            {
                if (i->name.compare(clientName) == 0)
                {
                    validClient = false;
                    std::cout << "CLIENT TAKEN" << std::endl;
                    connectionState = FAILURE;
                    write(connectionSocket, (const void *) connectionState, 1);
                }
            }

            if (validClient)
            {
                Client client;
                client.name = clientName;
                client.socket = connectionSocket;
                clients.push_back(client);
                FD_SET(client.socket, &readFDs);
                connectionState = SUCCESS;
                write(connectionSocket, (const void *) connectionState, 1);
            }
            else
            {
                close(connectionSocket);
            }


        }
        else
        {
            std::cout << "ELSE" << std::endl;
            for (auto i = clients.begin(); i != clients.end(); ++i)
            {
                if (FD_ISSET(i->socket, &currentFDs))
                {
                    char clientMessage[1024] = {NULL};
                    read(i->socket, clientMessage, 1024);
                    std::cout << clientMessage << std::endl;
                }
            }
        }


    }
//
//    ssize_t readCount = 0;
//    while( (readCount = recv(connectionSocket , clientMessage , 2000 , 0)) > 0 )
//    {
//        std::cout << clientMessage << std::endl;
//    }

}