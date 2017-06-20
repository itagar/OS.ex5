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
 * @brief Type Definition for a map from socket to client name.
 */
typedef std::map<int, clientName_t> socketToNameMap;


/*-----=  Server Data  =-----*/


/**
 * @brief The vector of the connected clients.
 */
clientsVector clients = clientsVector();

/**
 * @brief The map from the connected client sockets into their names.
 */
socketToNameMap socketsToNames = socketToNameMap();

/**
 * @brief The FD Set for the server to read from.
 */
fd_set readFDs;


/*-----=  Server Initialization Functions  =-----*/


/**
 * @brief Reset the server data.
 */
static void resetServerData()
{
    clients = clientsVector();
    socketsToNames = socketToNameMap();
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
 * @brief Determines if the given client name is available to use in the server.
 * @param clientName The client name to check.
 * @return true if available, false otherwise.
 */
static bool checkAvailableClientName(const clientName_t clientName)
{
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        clientName_t currentName = socketsToNames[*i];
        if ((currentName).compare(clientName) == EQUAL_COMPARISON)
        {
            return false;
        }
    }
    return true;
}

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
            if (checkAvailableClientName(clientName))
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


/**
 * @brief Removes a client from the server.
 * @param clientSocket The client to remove.
 */
static void removeClient(const int clientSocket)
{
    // TODO: Remove client from all groups.
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket));
    FD_CLR(clientSocket, &readFDs);
    socketsToNames.erase(clientSocket);
}

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
        whoResponse += (i == (--currentClients.end()) ? "." : ",");
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
static void processMessage(int const clientSocket, const message_t &message)
{
    int tagChar = message.front() - TAG_CHAR_BASE;

    switch (tagChar)
    {
        case CLIENT_EXIT:
            handleClientExitCommand(clientSocket);
            break;

        case WHO:
            handleClientWhoCommand(clientSocket);
            break;

        default:
            // TODO: Error.
            break;

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