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

/**
 * @def SEND_REGEX "send ([a-zA-Z0-9]+) (.*)"
 * @brief A Macro that sets the send command regex.
 */
#define SEND_REGEX "send ([a-zA-Z0-9]+) (.*)"

/**
 * @def GROUP_REGEX "create_group ([a-zA-Z0-9]+) ([a-zA-Z0-9]+[,a-zA-Z0-9]*)"
 * @brief A Macro that sets the group command regex.
 */
#define GROUP_REGEX "create_group ([a-zA-Z0-9]+) ([a-zA-Z0-9]+[,a-zA-Z0-9]*)"


/*-----=  Client Data  =-----*/

/**
 * @brief The client name.
 */
clientName_t clientName;


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


/**
 * @brief Handle the server EXIT command.
 * @param clientSocket The current client socket.
 */
static int handleServerExitCommand(int const clientSocket)
{
    close(clientSocket);
    exit(EXIT_FAILURE);
}

/**
 * @brief Handle response from the server due to client command.
 * @param message The server response.
 */
static void handleServerResponseMessage(const message_t &message)
{
    message_t response = message.substr(1);  // Trim the message tag.
    std::cout << response << std::endl;
}

/**
 * @brief Handle a message from the server.
 * @param message The server response.
 */
static void handleServerMessage(const message_t &message)
{
    std::cout << message << std::endl;
}

/**
 * @brief Process a message received in the given client socket.
 * @param clientSocket The curernt client socket.
 * @param message The message to process.
 */
static void processMessage(int const clientSocket, const message_t &message)
{
    int tagChar = message.front() - TAG_CHAR_BASE;

    switch (tagChar)
    {
        case CREATE_GROUP:
            handleServerResponseMessage(message);
            return;

        case SEND:
            handleServerResponseMessage(message);
            return;

        case WHO:
            handleServerResponseMessage(message);
            return;

        case SERVER_EXIT:
            handleServerExitCommand(clientSocket);
            return;

        default:
            handleServerMessage(message);
            return;
    }
}

/**
 * @brief Parse messages read from the recent read operation from the socket.
 * @param clientSocket The current client socket.
 * @param messages The read data which contain a message or several.
 */
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


/**
 * @brief Handle the exit command of the client.
 * @param clientSocket The current client socket.
 */
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

/**
 * @brief Handles the client who command.
 * @param clientSocket The current client socket.
 */
static void handleClientWhoCommand(int const clientSocket)
{
    message_t clientWho = std::to_string(WHO);
    if (writeData(clientSocket, clientWho) < 0)
    {
        systemCallError(WRITE_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Read the server response.
    handleServer(clientSocket);
}

/**
 * @brief Helper function for create group command which creates the client
 *        message part in the entire group message.
 * @param groupClients A message contains data of the group clients.
 * @return A modified message used by the server.
 */
static message_t createGroupClientsMessage(message_t const groupClients)
{
    message_t clientsNames;
    std::stringstream modifiedClients = std::stringstream(GROUP_CLIENTS_DELIM
                                                          + groupClients);
    modifiedClients << GROUP_CLIENTS_DELIM;

    clientName_t currentName;
    while (getline(modifiedClients, currentName, GROUP_CLIENTS_DELIM))
    {
        if (currentName.compare(EMPTY_MSG))
        {
            // If the name is not empty.
            clientsNames += WHITE_SPACE_SEPARATOR;
            clientsNames += currentName;
        }
    }

    return clientsNames;
}

/**
 * @brief Handles the create group command by the client.
 * @param clientSocket The current client socket.
 * @param groupName The group name.
 * @param groupClients The group clients.
 */
static void handleClientGroupCommand(int const clientSocket,
                                     groupName_t const groupName,
                                     message_t const groupClients)
{
    // Add the message tag representing group creation.
    message_t clientGroup = std::to_string(CREATE_GROUP);
    // Add the group name.
    clientGroup += groupName;
    // Add the group members.
    clientGroup += createGroupClientsMessage(groupClients);

    // Send the server the group creation message.
    if (writeData(clientSocket, clientGroup) < 0)
    {
        systemCallError(WRITE_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Read the server response.
    handleServer(clientSocket);
}

/**
 * @brief Handles the send command by the client.
 * @param clientSocket The current client socket.
 * @param sendTo The receiver name.
 * @param message The message to send.
 */
static void handleClientSendCommand(int const clientSocket,
                                    clientName_t const sendTo,
                                    message_t const message)
{
    // Add the message tag representing send.
    message_t clientSend = std::to_string(SEND);
    clientSend += sendTo + WHITE_SPACE_SEPARATOR + message;
    // Send the server the group creation message.
    if (writeData(clientSocket, clientSend) < 0)
    {
        systemCallError(WRITE_NAME, errno);
        exit(EXIT_FAILURE);
    }

    // Read the server response.
    handleServer(clientSocket);
}

/**
 * @brief Parse and analyze the user input command.
 * @param clientSocket The current client socket.
 * @param clientInput the client input command.
 */
static void parseClientInput(int const clientSocket,
                             const message_t &clientInput)
{
    std::regex sendRegex(SEND_REGEX);
    std::regex groupRegex(GROUP_REGEX);
    std::smatch matcher;

    if (clientInput.compare(EXIT_COMMAND) == EQUAL_COMPARISON)
    {
        handleClientExitCommand(clientSocket);
        assert(false);  // We should never reach this line.
    }
    if (clientInput.compare(WHO_COMMAND) == EQUAL_COMPARISON)
    {
        handleClientWhoCommand(clientSocket);
        return;
    }

    if (clientInput.find(CREATE_GROUP_COMMAND) == MSG_BEGIN_INDEX)
    {
        if (std::regex_match(clientInput, matcher, groupRegex))
        {
            handleClientGroupCommand(clientSocket, matcher[1], matcher[2]);
            return;
        }

        groupName_t groupName = EMPTY_MSG;
        groupName += matcher[1];
        std::cout << GROUP_FAIL_MSG << QUATS << groupName << QUATS
                  << MSG_SUFFIX << std::endl;
        return;
    }

    if (clientInput.find(SEND_COMMAND) == MSG_BEGIN_INDEX)
    {
        if (std::regex_match(clientInput, matcher, sendRegex))
        {
            if (matcher[1].compare(clientName) == EQUAL_COMPARISON)
            {
                // If the client sends a message to itself.
                std::cout << CLIENT_SEND_FAIL_MSG << std::endl;
                return;
            }

            handleClientSendCommand(clientSocket, matcher[1], matcher[2]);
            return;
        }

        std::cout << CLIENT_SEND_FAIL_MSG << std::endl;
        return;
    }

    // If the given command doesn't exists.
    std::cout << INVALID_INPUT_MSG << std::endl;
    return;
}

/**
 * @brief Handles the client procedure in case of receiving input from the user.
 */
static void handleClientInput(int const clientSocket)
{
    message_t clientInput;
    std::getline(std::cin, clientInput);
    parseClientInput(clientSocket, clientInput);
}


/*-----=  Main  =-----*/


/**
 * @brief The main function that runs the client.
 */
int main(int argc, char *argv[])
{
    // Check the client arguments.
    if (checkClientArguments(argc, argv))
    {
        std::cout << USAGE_MSG;
        return FAILURE_STATE;
    }

    clientName = argv[CLIENT_ARGUMENT_INDEX];
    const char *serverAddress = argv[SERVER_ARGUMENT_INDEX];
    portNumber_t portNum = (portNumber_t) std::stoi(argv[PORT_ARGUMENT_INDEX]);

    // Attempt to connect to the server.
    int clientSocket = callSocket(serverAddress, portNum, clientName);
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