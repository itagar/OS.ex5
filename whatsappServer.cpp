/**
 * @file whatsappServer.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Server.
 */


/*-----=  Includes  =-----*/


#include <string>
#include <climits>
#include <cstring>
#include <vector>
#include <cassert>
#include <algorithm>
#include <map>
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
 * @def USAGE_MSG "Usage: whatsappServer portNum"
 * @brief A Macro that sets the error message when the usage is invalid.
 */
#define USAGE_MSG "Usage: whatsappServer portNum"

/**
 * @def SERVER_EXIT_COMMAND "EXIT"
 * @brief A Macro that sets the exit command when exiting the server.
 */
#define SERVER_EXIT_COMMAND "EXIT"

/**
 * @def SERVER_EXIT_MSG "EXIT command is typed: server is shutting down"
 * @brief A Macro that sets the exit message when exiting the server.
 */
#define SERVER_EXIT_MSG "EXIT command is typed: server is shutting down"

/**
 * @def CONNECT_SUCCESS_MSG_SUFFIX " connected."
 * @brief A Macro that sets the message suffix on connection success.
 */
#define CONNECT_SUCCESS_MSG_SUFFIX " connected."

/**
 * @def CONNECT_FAIL_MSG_SUFFIX " failed to connect."
 * @brief A Macro that sets the message suffix on connection failure.
 */
#define CONNECT_FAIL_MSG_SUFFIX " failed to connect."

/**
 * @def MAX_PENDING_CONNECTIONS 10
 * @brief A Macro that sets the maximal number of pending connections.
 */
#define MAX_PENDING_CONNECTIONS 10


/*-----=  Type Definitions  =-----*/


/**
 * @brief Type Definition for a vector of client sockets.
 */
typedef std::vector<int> clientsVector;

/**
 * @brief Type Definition for a vector of groups.
 */
typedef std::vector<groupName_t> groupVector;

/**
 * @brief Type Definition for a map from socket to client name.
 */
typedef std::map<int, clientName_t> socketToNameMap;

/**
 * @brief Type Definition for a map from group to a clients vector.
 */
typedef std::map<groupName_t, clientsVector> groupToClient;


/*-----=  Server Data  =-----*/


/**
 * @brief The vector of the connected clients.
 */
clientsVector clients = clientsVector();

/**
 * @brief The vector of the open groups.
 */
groupVector groups = groupVector();

/**
 * @brief The map from the connected client sockets into their names.
 */
socketToNameMap socketsToNames = socketToNameMap();

/**
 * @brief The map from the open groups to the vector of their clients.
 */
groupToClient groupsToClients = groupToClient();

/**
 * @brief The FD Set for the server to read from.
 */
fd_set readFDs;


/*-----=  General Functions  =-----*/


/**
 * @brief Gets the max socket ID currently in the server.
 * @param welcomeSocketID The welcome socket ID.
 * @return the max socket ID.
 */
static int getMaxSocketID(int const welcomeSocketID)
{
    int maxID = welcomeSocketID;
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        maxID = std::max(maxID, *i);
    }
    return maxID;
}

/**
 * @brief Determines if the given client name is available to use in the server.
 * @param clientName The client name to check.
 * @return true if available, false otherwise.
 */
static bool checkAvailableName(const clientName_t clientName)
{
    // Check in clients names.
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        clientName_t currentName = socketsToNames[*i];
        if ((currentName).compare(clientName) == EQUAL_COMPARISON)
        {
            return false;
        }
    }

    // Check in groups names.
    for (auto i = groups.begin(); i != groups.end(); ++i)
    {
        if ((*i).compare(clientName) == EQUAL_COMPARISON)
        {
            return false;
        }
    }

    return true;
}


/*-----=  Client Management Functions  =-----*/


// TODO: Doxygen.
static void removeClientFromGroups(const int clientSocket)
{
    for (auto i = groups.begin(); i != groups.end(); ++i)
    {
        groupsToClients[*i].erase(std::remove(groupsToClients[*i].begin(),
                                              groupsToClients[*i].end(),
                                              clientSocket));
    }
}

/**
 * @brief Creates a new Client in the server with the given data.
 * @param name The client name.
 * @param socket The socket of the new client.
 */
static void createNewClient(const clientName_t name, const int socket)
{
    clients.push_back(socket);
    FD_SET(socket, &readFDs);
    socketsToNames[socket] = name;
}

/**
 * @brief Removes a client from the server.
 * @param clientSocket The client to remove.
 */
static void removeClient(const int clientSocket)
{
    removeClientFromGroups(clientSocket);
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket));
    FD_CLR(clientSocket, &readFDs);
    socketsToNames.erase(clientSocket);
}

// TODO: Doxygen.
static int getClientSocket(clientName_t const clientName)
{
    int clientSocket = FAILURE_STATE;
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        if (socketsToNames[*i].compare(clientName) == EQUAL_COMPARISON)
        {
            clientSocket = *i;
        }
    }
    return clientSocket;
}

// TODO: Doxygen.
static bool clientOnline(clientName_t const clientName)
{
    return getClientSocket(clientName) > FAILURE_STATE;
}


/*-----=  Group Management Functions  =-----*/


// TODO: Doxygen.
static void createNewGroup(groupName_t const groupName)
{
    groups.push_back(groupName);
    groupsToClients[groupName] = clientsVector();
}

// TODO: Doxygen.
static void removeGroup(groupName_t const groupName)
{
    groups.erase(std::remove(groups.begin(), groups.end(), groupName));
    groupsToClients.erase(groupName);
}

// TODO: Doxygen.
static bool groupContainsClient(groupName_t const groupName, int client)
{
    clientsVector groupClients = groupsToClients[groupName];
    auto i = std::find(groupClients.begin(), groupClients.end(), client);
    return i != groupClients.end();
}

// TODO: Doxygen.
static int addSingleClientToGroup(clientName_t const clientName,
                                  groupName_t const groupName)
{
    int clientSocket = getClientSocket(clientName);

    if (!groupContainsClient(groupName, clientSocket))
    {
        groupsToClients[groupName].push_back(clientSocket);
        return SUCCESS_STATE;
    }
    return FAILURE_STATE;
}

// TODO: Doxygen.
static bool groupOpen(groupName_t const groupName)
{
    auto i = std::find(groups.begin(), groups.end(), groupName);
    return i != groups.end();
}

// TODO: Doxygen.
static int addClientsToGroup(clientName_t const creator,
                             groupName_t const groupName,
                             message_t clientsNames)
{
    int numberOfClients = 0;
    // Add the creator in to the group.
    if (addSingleClientToGroup(creator, groupName))
    {
        return FAILURE_STATE;
    }
    numberOfClients++;

    std::stringstream clientsStream = std::stringstream(clientsNames);
    clientName_t currentName;
    while (getline(clientsStream, currentName, WHITE_SPACE_DELIM))
    {
        if (currentName.compare(EMPTY_MSG))
        {
            // Check that this client is online.
            if (!clientOnline(currentName))
            {
                return FAILURE_STATE;
            }
            if (addSingleClientToGroup(currentName, groupName))
            {
                continue;
            }
            // If the client was added to the group.
            numberOfClients++;
        }
    }

    if (numberOfClients < 2)
    {
        return FAILURE_STATE;
    }

    return SUCCESS_STATE;
}


/*-----=  Server Initialization Functions  =-----*/


/**
 * @brief Reset the server data.
 */
static void resetServerData()
{
    clients = clientsVector();
    groups = groupVector();
    socketsToNames = socketToNameMap();
    groupsToClients = groupToClient();
    FD_ZERO(&readFDs);
}

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
 * @brief Establish connection of the server with the given port number.
 *        This function creates the welcome socket of the server.
 * @param portNumber The given port number of the server.
 * @return The socket ID of the welcome socket upon success, -1 on failure.
 */
static int establish(const portNumber_t portNumber)
{
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

    std::cout << "IP: " << inet_ntoa(*((in_addr *)pHostent->h_addr)) << std::endl;  // TODO: Delete This.

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


/*-----=  Handle Input Functions  =-----*/


/**
 * @brief Perform the actions required when terminating the server.
 */
static void terminateServer()
{
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        // Write to each client that the server is terminating.
        message_t serverExit = std::to_string(SERVER_EXIT);
        writeData(*i, serverExit);
    }

    // Terminate the server.
    std::cout << SERVER_EXIT_MSG;
    exit(EXIT_SUCCESS);
}

/**
 * @brief Handles the server procedure in case of receiving input from the user.
 */
static void handleServerInput()
{
    message_t currentInput;
    std::getline(std::cin, currentInput);

    if (currentInput.compare(SERVER_EXIT_COMMAND) == EQUAL_COMPARISON)
    {
        // If the server received the EXIT command, it should terminate.
        terminateServer();
    }
}


/*-----=  Handle Connection Functions  =-----*/


/**
 * @brief Get a new connection with the given socketID (will be welcome socket).
 * @param socketID The socket to connect with.
 * @return The new socket that the welcome socket returned from the connection.
 */
static int getConnection(const int socketID)
{
    int newSocket = accept(socketID, NULL, NULL);
    if (newSocket < SOCKET_ID_BOUND)
    {
        systemCallError(ACCEPT_NAME, errno);
        return FAILURE_STATE;
    }
    return newSocket;
}

/**
 * @brief Handles the server procedure on a new connection request.
 * @param welcomeSocket The welcome socket of the server.
 */
static void handleNewConnection(const int welcomeSocket)
{
    // Declare indicator variables for this new connection process.
    int connectionState = SUCCESS_STATE;
    bool availableName = true;
    bool receivedName = false;

    clientName_t clientName;

    int connectionSocket = getConnection(welcomeSocket);
    if (connectionSocket < SOCKET_ID_BOUND)
    {
        connectionState = FAILURE_STATE;
    }
    else
    {
        // In our protocol, right after connection request there should be a
        // message with the client name. We first check that this client
        // name is available.
        if (readData(connectionSocket, clientName) < 0)
        {
            connectionState = FAILURE_STATE;
        }
        else
        {
            // Here we have successfully read the client name.
            // Check if the name is available.
            receivedName = true;
            if (checkAvailableName(clientName))
            {
                createNewClient(clientName, connectionSocket);
            }
            else
            {
                availableName = false;
                connectionState = FAILURE_STATE;
            }
        }
    }

    if (connectionState)
    {
        // If the new connection failed.
        char state = NULL;
        if (!availableName)
        {
            // If the failure reason is due to client name in use.
            state = CONNECTION_IN_USE_STATE;
        }
        else
        {
            // If the failure is for any other reason.
            state = CONNECTION_FAIL_STATE;
        }

        // Send to this client that the connection is failed.
        write(connectionSocket, &state, sizeof(char));  // TODO: Check sys call.
        // There might be a failure before we even received the client name.
        if (receivedName)
        {
            std::cout << clientName << CONNECT_FAIL_MSG_SUFFIX << std::endl;
        }
        // Close the socket stream.
        close(connectionSocket);
    }
    else
    {
        assert(availableName);
        // Send to this client that the connection is successful.
        char state = CONNECTION_SUCCESS_STATE;
        write(connectionSocket, &state, sizeof(char));  // TODO: Check sys call.
        std::cout << clientName << CONNECT_SUCCESS_MSG_SUFFIX << std::endl;
    }
}


/*-----=  Handle Clients Functions  =-----*/


// TODO: Doxygen.
static void handleClientExitCommand(int const clientSocket)
{
    clientName_t clientName = socketsToNames[clientSocket];

    // Remove the client from the server data.
    removeClient(clientSocket);

    // Send the client response about the log out and print a message.
    char state = LOGOUT_SUCCESS_STATE;
    write(clientSocket, &state, sizeof(char));  // TODO: Check sys call.
    std::cout << clientName << ": " << LOGOUT_SUCCESS_MSG << std::endl;  // TODO: Magic Number.
}

// TODO: Doxygen.
static message_t setWhoResponse()
{
    // Set the message tag.
    message_t whoResponse = std::to_string(WHO);

    // Set a new container of all the client names.
    std::vector<clientName_t> currentClients;

    // Add all the names of all clients in the server.
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        currentClients.push_back(socketsToNames[*i]);
    }

    // Sort and create the who response message.
    std::sort(currentClients.begin(), currentClients.end());
    for (auto i = currentClients.begin(); i != currentClients.end(); ++i)
    {
        whoResponse += *i;
        whoResponse += (i == (--currentClients.end()) ? MSG_SUFFIX : GROUP_SEP);
    }

    return whoResponse;
}

// TODO: Doxygen.
static void handleClientWhoCommand(int const clientSocket)
{
    clientName_t clientName = socketsToNames[clientSocket];

    // Print an informative message to the server.
    std::cout << clientName << ": " << WHO_REQUEST_MSG << std::endl;  // TODO: Magic Number.

    // Set a response for the client.
    message_t whoResponse = setWhoResponse();

    if (writeData(clientSocket, whoResponse) < 0)
    {
        // TODO: Check what to do in this case from the client point.
    }
}

// TODO: Doxygen.
static void handleClientGroupCommand(int const clientSocket,
                                     const message_t &message)
{
    bool successState = false;
    clientName_t clientName = socketsToNames[clientSocket];
    message_t modifiedMessage = message.substr(1);  // Trim the message tag.

    auto trimIndex = modifiedMessage.find(WHITE_SPACE_DELIM);
    groupName_t groupName = modifiedMessage.substr(0, trimIndex);

    // Prepare the message for clients reading by removing the group name.
    modifiedMessage = modifiedMessage.substr(trimIndex + 1);

    // Check the group name is available.
    if (checkAvailableName(groupName))
    {
        // If group name is valid.
        createNewGroup(groupName);
        // Add each client to the group.
        if (addClientsToGroup(clientName, groupName, modifiedMessage) == SUCCESS_STATE)
        {
            successState = true;
        }
    }

    message_t groupResponse = std::to_string(CREATE_GROUP);
    if (successState)
    {
        // Set a response for the client.
        groupResponse += "Group \"" + groupName + "\" was created successfully.";
        // Print an informative message to the server.
        std::cout << clientName << ": " << "Group \"" << groupName << "\" was created successfully." << std::endl;  // TODO: Magic Number.
    }
    else
    {
        // Remove the newly created group.
        removeGroup(groupName);
        // Set a response for the client.
        groupResponse += "ERROR: failed to create group \"" + groupName + "\".";
        // Print an informative message to the server.
        std::cout << clientName << ": " << "ERROR: failed to create group \"" << groupName << "\"." << std::endl;  // TODO: Magic Number.
    }

    if (writeData(clientSocket, groupResponse) < 0)
    {
        // TODO: Check what to do in this case from the client point.
    }
}

// TODO: Doxygen.
static void sendMessageToClient(clientName_t const senderName,
                                clientName_t const receiverName,
                                message_t const &message)
{
    int receiverSocket = getClientSocket(receiverName);
    message_t toSend = senderName + ": " + message;
    writeData(receiverSocket, toSend);
}

// TODO: Doxygen.
static void sendMessageToGroup(clientName_t const senderName,
                               groupName_t const groupName,
                               message_t const &message)
{
    clientsVector groupClients = groupsToClients[groupName];

    for (auto i = groupClients.begin(); i != groupClients.end(); ++i)
    {
        clientName_t currentName = socketsToNames[*i];
        if (currentName.compare(senderName) == EQUAL_COMPARISON)
        {
            continue;
        }
        sendMessageToClient(senderName, currentName, message);
    }
}

// TODO: Doxygen.
static void handleClientSendCommand(int const clientSocket,
                                    const message_t &message)
{
    bool successState = false;
    clientName_t senderName = socketsToNames[clientSocket];
    message_t modifiedMessage = message.substr(1);  // Trim the message tag.

    auto trimIndex = modifiedMessage.find(WHITE_SPACE_DELIM);
    clientName_t sendTo = modifiedMessage.substr(0, trimIndex);

    // Prepare the message for clients reading by removing the group name.
    modifiedMessage = modifiedMessage.substr(trimIndex + 1);

    // Check the group name is available.
    if (clientOnline(sendTo))
    {
        // If client name to send is valid.
        sendMessageToClient(senderName, sendTo, modifiedMessage);
        successState = true;

    }
    else if (groupOpen(sendTo))
    {
        // If the send request is for a valid group.
        sendMessageToGroup(senderName, sendTo, modifiedMessage);
        successState = true;
    }

    message_t sendResponse = std::to_string(SEND);
    if (successState)
    {
        // Set a response for the client.
        sendResponse += "Sent successfully.";
        // Print an informative message to the server.
        std::cout << senderName << ": \"" << modifiedMessage << "\" was sent successfully to " << sendTo << "." << std::endl;  // TODO: Magic Number.
    }
    else
    {
        // Set a response for the client.
        sendResponse += "ERROR: failed to send.";
        // Print an informative message to the server.
        std::cout << senderName << ": ERROR: failed to send \"" << modifiedMessage << "\" to " << sendTo << "." << std::endl;  // TODO: Magic Number.
    }

    if (writeData(clientSocket, sendResponse) < 0)
    {
        // TODO: Check what to do in this case from the client point.
    }
}

// TODO: Doxygen.
static void processMessage(int const clientSocket, const message_t &message)
{
    int tagChar = message.front() - TAG_CHAR_BASE;

    switch (tagChar)
    {
        case CREATE_GROUP:
            handleClientGroupCommand(clientSocket, message);
            return;

        case SEND:
            handleClientSendCommand(clientSocket, message);
            return;

        case WHO:
            handleClientWhoCommand(clientSocket);
            return;

        case CLIENT_EXIT:
            handleClientExitCommand(clientSocket);
            return;

        default:
            assert(false);
            return;
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

// TODO: Doxygen.
static void handleClients(fd_set *currentFDs)
{
    for (int clientSocket : clients)
    {
        if (FD_ISSET(clientSocket, currentFDs))
        {
            message_t clientMessage;
            if (readData(clientSocket, clientMessage) < 0)
            {
                return;
            }
            parseMessages(clientSocket, clientMessage);
        }
    }
}


/*-----=  Main  =-----*/


// TODO: Doxygen.
int main(int argc, char *argv[])
{
    resetServerData();

    // Check the server arguments.
    if (checkServerArguments(argc, argv))
    {
        std::cout << USAGE_MSG;
        return FAILURE_STATE;
    }

    // Set the port number and create a welcome socket with that port.
    portNumber_t portNumber = (portNumber_t) std::stoi(argv[PORT_ARGUMENT_INDEX]);
    int welcomeSocket = establish(portNumber);
    if (welcomeSocket < SOCKET_ID_BOUND)
    {
        return FAILURE_STATE;
    }

    // Update the readFDs set.
    FD_SET(STDIN_FILENO, &readFDs);
    FD_SET(welcomeSocket, &readFDs);

    while (true)
    {
        // Create a temporary FD Set for this iteration.
        fd_set currentFDs = readFDs;
        // Get the max socket ID for the select function.
        int maxSocketID = getMaxSocketID(welcomeSocket);
        // Select.
        int readyFD = select(maxSocketID + 1, &currentFDs, NULL, NULL, NULL);

        if (readyFD < 0)
        {
            systemCallError(SELECT_NAME, errno);
            return FAILURE_STATE;
        }

        if (FD_ISSET(STDIN_FILENO, &currentFDs))
        {
            handleServerInput();
        }
        else if (FD_ISSET(welcomeSocket, &currentFDs))
        {
            handleNewConnection(welcomeSocket);
        }
        else
        {
            handleClients(&currentFDs);
        }
    }
}