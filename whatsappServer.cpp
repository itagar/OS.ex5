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
 * @def EQUAL_COMPARISON 0
 * @brief A Macro that sets the value of equal strings while comparing them.
 */
#define EQUAL_COMPARISON 0

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
 * @def MAX_PENDING_CONNECTIONS 10
 * @brief A Macro that sets the maximal number of pending connections.
 */
#define MAX_PENDING_CONNECTIONS 10

/**
 * @def MINIMUM_NUMBER_OF_CLIENTS 0
 * @brief A Macro that sets the minimum number of clients.
 */
#define MINIMUM_NUMBER_OF_CLIENTS 0


/*-----=  Type Definitions  =-----*/


/**
 * @brief Type Definition for a vector of Client objects.
 */
typedef std::vector<Client> clientsVector;


/*-----=  Server Data  =-----*/


/**
 * @brief The vector of the connected clients.
 */
clientsVector clients = clientsVector();

/**
 * @brief The counter of the current connected clients number.
 */
int activeClients = MINIMUM_NUMBER_OF_CLIENTS;

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
    activeClients = MINIMUM_NUMBER_OF_CLIENTS;
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
        std::cout << USAGE_MSG;
        return FAILURE_STATE;
    }

    // Check valid port number.
    std::string portNum = argv[PORT_ARGUMENT_INDEX];
    for (int i = 0; i < portNum.length(); ++i)
    {
        if (!isdigit(portNum[i]))
        {
            std::cout << USAGE_MSG;
            return FAILURE_STATE;
        }
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
    // Close all of the clients sockets.
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        close(i->socket);
    }
}

/**
 * @brief Handles the server procedure in case of receiving input from the user.
 */
static void handleServerInput()
{
    std::string currentInput;
    std::getline(std::cin, currentInput);

    if (currentInput.compare(SERVER_EXIT_COMMAND) == EQUAL_COMPARISON)
    {
        // If the server received the EXIT command, it should terminate.
        terminateServer();
        std::cout << SERVER_EXIT_MSG;
        exit(EXIT_SUCCESS);
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
        if (i->name.compare(clientName) == EQUAL_COMPARISON)
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
    Client client;
    client.name = name;
    client.socket = socket;
    clients.push_back(client);
    FD_SET(client.socket, &readFDs);
    activeClients++;
}

/**
 * @brief Handles the server procedure on a new connection request.
 * @param welcomeSocket The welcome socket of the server.
 */
static void handleNewConnection(const int welcomeSocket)
{
    int connectionState = SUCCESS_STATE;
    clientName_t clientName;

    int connectionSocket = getConnection(welcomeSocket);
    if (connectionSocket < SOCKET_ID_BOUND)
    {
        // TODO: Check which Error.
        connectionState = FAILURE_STATE;
    }
    else
    {
        // In our protocol, right after connection request there should be a
        // message with the client name. We first check that this client
        // name is available.
        if (readData(connectionSocket, clientName) < 0)
        {
            // TODO: If it failed to read the name I cant print the message with the name.
            connectionState = FAILURE_STATE;
        }
        else
        {
            // Here we have successfully read the client name.
            // Check if the name is available.
            if (checkAvailableClientName(clientName))
            {
                createNewClient(clientName, connectionSocket);
                connectionState = SUCCESS_STATE;
            }
            else
            {
                connectionState = FAILURE_STATE;
            }
        }
    }

    if (connectionState)
    {
        // If the new connection failed.
        // Send to this client that the connection is failed.
        char state = CONNECTION_FAIL_STATE;
        write(connectionSocket, &state, sizeof(char));  // TODO: Check sys call.
        tcflush(connectionSocket, TCIOFLUSH);
        std::cout << clientName << " failed to connect." << std::endl;  // TODO: Magic Number.
        // Close the socket stream.
        close(connectionSocket);
    }
    else
    {
        // Send to this client that the connection is successful.
        char state = CONNECTION_SUCCESS_STATE;
        write(connectionSocket, &state, sizeof(char));  // TODO: Check sys call.
        tcflush(connectionSocket, TCIOFLUSH);
        std::cout << clientName << " connected." << std::endl;  // TODO: Magic Number.
    }
}


/*-----=  Handle Clients Functions  =-----*/


// TODO: Doxygen.
static void handleClients(fd_set *currentFDs)
{
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        if (FD_ISSET(i->socket, currentFDs))
        {
            message_t clientMessage;
            readData(i->socket, clientMessage);  // TODO: Check sys call.
            std::cout << clientMessage << std::endl;
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
        maxID = std::max(maxID, i->socket);
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
            // If select failed.
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