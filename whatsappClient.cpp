/**
 * @file whatsappClient.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Client.
 */


/*-----=  Includes  =-----*/


#include <cstring>
#include <regex>
#include <stdlib.h>
#include <cassert>
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
 * @def INVALID_INPUT_MSG "ERROR: Invalid input."
 * @brief A Macro that sets the error message in an invalid input from the user.
 */
#define INVALID_INPUT_MSG "ERROR: Invalid input."

/**
 * @def CONNECT_SUCCESS_MSG "Connected Successfully."
 * @brief A Macro that sets the message upon a successful connection.
 */
#define CONNECT_SUCCESS_MSG "Connected Successfully."

/**
 * @def TAKEN_CLIENT_NAME_MSG "Client name is already in use."
 * @brief A Macro that sets the message when the client name is already in use.
 */
#define TAKEN_CLIENT_NAME_MSG "Client name is already in use."

/**
 * @def CONNECT_FAILURE_MSG "Failed to connect the server"
 * @brief A Macro that sets the message upon a failure in connection.
 */
#define CONNECT_FAILURE_MSG "Failed to connect the server"


/*-----=  Client Initialization Functions  =-----*/


/**
 * @brief Checks if the given client name is a valid name.
 * @param clientName The client name to check.
 * @return 0 if the name is valid, -1 otherwise.
 */
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

/**
 * @brief Checks if the given server address is a valid address.
 * @param serverAddress The server address to check.
 * @return 0 if the address is valid, -1 otherwise.
 */
static int validateServerAddress(std::string const serverAddress)
{
    for (int i = 0; i < serverAddress.length(); ++i)
    {
        if (!isdigit(serverAddress[i]) && serverAddress[i] != ADDRESS_DELIMITER)
        {
            return FAILURE_STATE;
        }
    }
    return SUCCESS_STATE;
}

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
    // Check valid number of arguments.
    if (argc != VALID_ARGUMENTS_COUNT)
    {
        return FAILURE_STATE;
    }

    // Check valid client name.
    if (validateClientName(argv[CLIENT_ARGUMENT_INDEX]))
    {
        return FAILURE_STATE;
    }

    // Check valid client name.
    if (validateServerAddress(argv[SERVER_ARGUMENT_INDEX]))
    {
        return FAILURE_STATE;
    }

    // Check valid port number.
    if (validatePortNumber(argv[PORT_ARGUMENT_INDEX]))
    {
        return FAILURE_STATE;
    }

    return SUCCESS_STATE;
}

/**
 * @brief Request from the server to create itself as a client.
 * @param socket The socket of the client.
 * @param clientName The client name.
 * @return 0 upon success, -1 otherwise.
 */
static int createClientRequest(const int socket, const clientName_t clientName)
{
    // First we write the client name in our socket so the server could read
    // it and analyze it.
    if (writeData(socket, clientName) < 0)
    {
        systemCallError(WRITE_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Now we wait for a response from the server about our name and therefore
    // about our connection state.
    char connectionState;
    if (read(socket, &connectionState, sizeof(char)) < 0)
    {
        systemCallError(READ_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Check the connection state that received from the server.
    if (connectionState == CONNECTION_SUCCESS_STATE)
    {
        // If the connection is valid and the name is valid.
        std::cout << CONNECT_SUCCESS_MSG << std::endl;
        return SUCCESS_STATE;
    }
    else if (connectionState == CONNECTION_IN_USE_STATE)
    {
        // If the name is already taken.
        std::cout << TAKEN_CLIENT_NAME_MSG << std::endl;
        exit(EXIT_FAILURE);
    }
    // Any other failure during connection to the server.
    std::cout << CONNECT_FAILURE_MSG << std::endl;
    exit(EXIT_FAILURE);
}

/**
 * @brief Attempt to connect to the server provided by host and port.
 * @param hostName The host name of the server.
 * @param portNumber The port number of the server.
 * @param clientName The client name to connect.
 * @return 0 upon success, -1 otherwise.
 */
static int callSocket(const char *hostName, const portNumber_t portNumber,
                      const clientName_t clientName)
{
    // Hostent initialization.
    hostent *pHostent = gethostbyname(hostName);
    if (pHostent == nullptr)
    {
        systemCallError(GETHOSTBYNAME_NAME, errno);
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }
    if (connect(socketID, (sockaddr *) &sa, sizeof(sockaddr_in)))
    {
        if (close(socketID))
        {
            systemCallError(CLOSE_NAME, errno);
            exit(EXIT_FAILURE);
        }
        systemCallError(CONNECT_NAME, errno);
        exit(EXIT_FAILURE);
    }

    if (createClientRequest(socketID, clientName))
    {
        return FAILURE_STATE;
    }

    return socketID;
}


/*-----=  Handle Server Functions  =-----*/


// TODO: Doxygen.
static int handleServerExitCommand(int const clientSocket)
{
    close(clientSocket);
    exit(1);
}

// TODO: Doxygen.
static void handleServerWhoCommand(const message_t &message)
{
    message_t whoResponse = message.substr(1);  // Trim the message tag.
    std::cout << whoResponse << std::endl;
}

// TODO: Doxygen.
static void processMessage(int const clientSocket, const message_t &message)
{
    int tagChar = message.front() - TAG_CHAR_BASE;

    switch (tagChar)
    {
        case WHO:
            handleServerWhoCommand(message);
            return;

        case SEND:
            return;

        case CREATE_GROUP:
            return;

        case SERVER_EXIT:
            handleServerExitCommand(clientSocket);
            assert(false);  // We should never reach this line.

        default:
            assert(false);  // We should never reach this line.
    }
}

// TODO: Doxygen.
static void parseMessages(int const clientSocket, const message_t &messages)
{
    message_t currentMessage;
    std::stringstream messageStream = std::stringstream(messages);
    while (std::getline(messageStream, currentMessage))
    {
        processMessage(clientSocket, currentMessage);
    }
}

/**
 * @brief Handles the client procedure in case of receiving message from server.
 */
static void handleServer(int const clientSocket)
{
    message_t serverMessage;
    if (readData(clientSocket, serverMessage) < 0)
    {
        return;
    }
    parseMessages(clientSocket, serverMessage);
}


/*-----=  Handle Input Functions  =-----*/


// TODO: Doxygen.
static void handleClientExitCommand(int const clientSocket)
{
    // Notify the server on the exit.
    message_t clientExit = std::to_string(CLIENT_EXIT);
    if (writeData(clientSocket, clientExit) < 0)
    {
        systemCallError(WRITE_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Wait for response from the server.
    char serverResponse;
    if (read(clientSocket, &serverResponse, sizeof(char)) < 0)
    {
        systemCallError(READ_NAME, errno);
        exit(EXIT_FAILURE);
    }

    if (serverResponse == LOGOUT_SUCCESS_STATE)
    {
        std::cout << LOGOUT_SUCCESS_MSG << std::endl;
        close(clientSocket);
        exit(EXIT_SUCCESS);
    }

    close(clientSocket);
    exit(EXIT_FAILURE);
}

// TODO: Doxygen.
static void handleClientWhoCommand(int const clientSocket)
{
    // Notify the server on the exit.
    message_t clientWho = std::to_string(WHO);
    if (writeData(clientSocket, clientWho) < 0)
    {
        systemCallError(WRITE_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Read the server response.
    handleServer(clientSocket);
}

// TODO: Doxygen.
static int parseClientInput(int const clientSocket, const message_t &clientInput)
{
    std::regex sendRegex("send ([a-zA-Z0-9]+) ([.]*)");
    std::regex groupRegex("create_group ([a-zA-Z0-9]+) ([a-zA-Z0-9](,[a-zA-Z0-9])*)");
    std::smatch matches;


    if (clientInput.compare("exit") == EQUAL_COMPARISON)
    {
        handleClientExitCommand(clientSocket);
        assert(false);  // We should never reach this line.
    }
    if (clientInput.compare("who") == EQUAL_COMPARISON)
    {
        handleClientWhoCommand(clientSocket);
        return SUCCESS_STATE;
    }


    return SUCCESS_STATE;
}

// TODO: Stop sending message to myself.

/**
 * @brief Handles the client procedure in case of receiving input from the user.
 */
static void handleClientInput(int const clientSocket)
{
    message_t clientInput;
    std::getline(std::cin, clientInput);
    if (parseClientInput(clientSocket, clientInput))
    {
        // TODO: Invalid input.
    }
    // TODO: Maybe add to client the boolean field of 'pending'
}


/*-----=  General Functions  =-----*/


// TODO: Doxygen.
int main(int argc, char *argv[])
{
    // Check the client arguments.
    if (checkClientArguments(argc, argv))
    {
        std::cout << USAGE_MSG;
        return FAILURE_STATE;
    }

    clientName_t clientName = argv[CLIENT_ARGUMENT_INDEX];
    const char *serverAddress = argv[SERVER_ARGUMENT_INDEX];
    portNumber_t portNumber = (portNumber_t) std::stoi(argv[PORT_ARGUMENT_INDEX]);

    // Attempt to connect to the server.
    int clientSocket = callSocket(serverAddress, portNumber, clientName);
    if (clientSocket < SOCKET_ID_BOUND)
    {
        return FAILURE_STATE;
    }

    fd_set originalSet;
    FD_ZERO(&originalSet);
    FD_SET(STDIN_FILENO, &originalSet);
    FD_SET(clientSocket, &originalSet);

    while (true)
    {
        fd_set currentSet = originalSet;
        int readyFD = select(clientSocket + 1, &currentSet, NULL, NULL, NULL);

        if (readyFD < 0)
        {
            systemCallError(SELECT_NAME, errno);
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &currentSet))
        {
            handleClientInput(clientSocket);
        }

        if (FD_ISSET(clientSocket, &currentSet))
        {
            handleServer(clientSocket);
        }
    }
}

//
//std::regex whoReg("who");
//std::regex exitReg("exitClient");
//std::regex sendReg("send ([a-zA-Z0-9]+) ([.]*)");
//std::regex groupReg("create_group ([a-zA-Z0-9]+) ([a-zA-Z0-9](,[a-zA-Z0-9])*)");
//
//std::smatch matches;
//
//if (std::regex_match(cmdString, matches, groupReg))
//{
//client.createGroup(matches[1], matches[2]);
//}
//
//else if (std::regex_match(cmdString, matches, sendReg))
//{
//client.sendMsg(matches[1], matches[2]);
//}
//
//else if (std::regex_match(cmdString, matches, whoReg))
//{
//client.who();
//}
//
//else if (std::regex_match(cmdString, matches, exitReg))
//{
//client.exitClient();
//}
//
//else
//{
////todo error
//}
//return 0; //todo