// TODO: Check every system call.


/**
 * @file whatsappClient.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Client.
 */


/*-----=  Includes  =-----*/


#include <cstring>
#include <stdlib.h>
#include "WhatsApp.h"


/*-----=  Definitions  =-----*/


/**
 * @def VALID_ARGUMENTS_COUNT 4
 * @brief A Macro that sets the number for valid arguments count.
 */
#define VALID_ARGUMENTS_COUNT 4

/**
 * @def PORT_ARGUMENT_INDEX 1
 * @brief A Macro that sets the index of client name argument to this program.
 */
#define CLIENT_ARGUMENT_INDEX 1

/**
 * @def PORT_ARGUMENT_INDEX 1
 * @brief A Macro that sets index of server address argument to this program.
 */
#define SERVER_ARGUMENT_INDEX 2

/**
 * @def PORT_ARGUMENT_INDEX 3
 * @brief A Macro that sets the index of the port argument to this program.
 */
#define PORT_ARGUMENT_INDEX 3

/**
 * @def USAGE_MSG "Usage: whatsappClient clientName serverAddress serverPort"
 * @brief A Macro that sets the error message when the usage is invalid.
 */
#define USAGE_MSG "Usage: whatsappClient clientName serverAddress serverPort"

/**
 * @def ADDRESS_DELIMITER '.'
 * @brief A Macro that sets the server address delimiter.
 */
#define ADDRESS_DELIMITER '.'

/**
 * @def CONNECT_SUCCESS_MSG "Connected Successfully."
 * @brief A Macro that sets the message upon a successful connection.
 */
#define CONNECT_SUCCESS_MSG "Connected Successfully."

/**
 * @def CONNECT_FAILURE_MSG "Failed to connect the server"
 * @brief A Macro that sets the message upon a failure in connection.
 */
#define CONNECT_FAILURE_MSG "Failed to connect the server"



/**
 * @brief Checks whether the program received the desired number of arguments.
 *        In case of invalid arguments the function output an error
 *        message specifying the correct usage.
 * @param argc The number of arguments given to the program.
 * @param argv The array of given arguments.
 * @return 0 if the arguments are valid, -1 otherwise.
 */
static int checkClientArguments(int const argc, char * const argv[])
{
    // TODO: Check this function. when to print usage message.

    // Check valid number of arguments.
    if (argc != VALID_ARGUMENTS_COUNT)
    {
        // TODO: Check new line.
        std::cout << USAGE_MSG << std::endl;
        return FAILURE_STATE;
    }

    // Check valid client name.
    clientName_t clientName = argv[CLIENT_ARGUMENT_INDEX];
    for (int i = 0; i < clientName.length(); ++i)
    {
        if (!isalnum(clientName[i]))
        {
            std::cout << USAGE_MSG << std::endl;
            return FAILURE_STATE;
        }
    }

    // Check valid client name.
    std::string serverAddress = argv[SERVER_ARGUMENT_INDEX];
    for (int i = 0; i < serverAddress.length(); ++i)
    {
        if (!isdigit(serverAddress[i]) && serverAddress[i] != ADDRESS_DELIMITER)
        {
            std::cout << USAGE_MSG << std::endl;
            return FAILURE_STATE;
        }
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



static int createClientRequest(const int socket, const clientName_t clientName)
{
    if (write(socket, clientName.c_str(), clientName.length()) < 0)
    {
        systemCallError("write", errno);
        exit(EXIT_FAILURE);
    }

    fd_set readFDs;
    FD_ZERO(&readFDs);
    FD_SET(socket, &readFDs);
    int readyFD = select(socket + 1, &readFDs, NULL, NULL, NULL);

    if (readyFD < 0)
    {
        systemCallError("select", errno);
        exit(EXIT_FAILURE);
    }

    if (FD_ISSET(socket, &readFDs))
    {
        char connectionState[2] = {NULL};
        read(socket, connectionState, 1);
        if (connectionState[0] == '1')
        {
            return SUCCESS_STATE;
        }
        else
        {
            return FAILURE_STATE;
        }
    }

    return FAILURE_STATE;
}


// TODO: Doxygen.
static int callSocket(const char *hostName, const portNumber_t portNumber,
                      const clientName_t clientName)
{
    // TODO: check return value in failure (or exit).
    // Hostent initialization.
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

    // Create Socket.
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID < SOCKET_ID_BOUND)
    {
        systemCallError(SOCKET_NAME, errno);
        return FAILURE_STATE;
    }
    if (connect(socketID, (sockaddr *) &sa, sizeof(sockaddr_in)))
    {
        if (close(socketID))
        {
            systemCallError(CLOSE_NAME, errno);
            return FAILURE_STATE;
        }
        systemCallError(CONNECT_NAME, errno);
        return FAILURE_STATE;
    }

    if (createClientRequest(socketID, clientName))
    {
        return FAILURE_STATE;
    }

    return socketID;
}

// TODO: Doxygen.
static int validateClientName(clientName_t const clientName)
{
    for (int i = 0; i < clientName.length(); ++i)
    {
        if (!isalnum(clientName[i]))
        {
            return FAILURE_STATE;
        }
    }
    return SUCCESS_STATE;
}


// TODO: Doxygen.
int main(int argc, char *argv[])
{
    // Check the client arguments.
    if (checkClientArguments(argc, argv))
    {
        return FAILURE_STATE;  // TODO: Check this return value.
    }

    std::string clientName = argv[CLIENT_ARGUMENT_INDEX];
    const char *serverAddress = argv[SERVER_ARGUMENT_INDEX];
    portNumber_t portNumber = (portNumber_t) std::stoi(argv[PORT_ARGUMENT_INDEX]);

    // Attempt to connect to the server.
    int clientSocket = callSocket(serverAddress, portNumber, clientName);
    if (clientSocket < SOCKET_ID_BOUND)
    {
        std::cout << CONNECT_FAILURE_MSG << std::endl;
        exit(EXIT_FAILURE);

    }
    std::cout << CONNECT_SUCCESS_MSG << std::endl;



    Client client;
    client.name = clientName;
    client.socket = clientSocket;


    // TODO: Check client name available.

    fd_set originalSet;
    FD_ZERO(&originalSet);
    FD_SET(STDIN_FILENO, &originalSet);
    FD_SET(clientSocket, &originalSet);

    while (true)
    {
        fd_set currentSet = originalSet;
        int readyFD = select(clientSocket + 1, &currentSet, NULL, NULL, NULL);  // TODO: Magic Number.

        if (readyFD < 0)
        {
            // TODO: Error.
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &currentSet))
        {
            std::string currentInput;
            std::getline(std::cin, currentInput);
            write(clientSocket, currentInput.c_str(), currentInput.length());
        }

        if (FD_ISSET(clientSocket, &currentSet))
        {
            char serverMessage[1024] = {NULL};
            read(clientSocket, serverMessage, 1024);
            std::cout << serverMessage << std::endl;
        }
    }
}