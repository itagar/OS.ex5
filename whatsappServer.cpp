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


/*-----=  Server Initialization Functions  =-----*/


/**
 * @brief Reset the server data.
 */
static void resetServerData()
{
    clients = clientsVector();
    activeClients = MINIMUM_NUMBER_OF_CLIENTS;
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
        std::cout << USAGE_MSG;  // TODO: Check new line.
        return FAILURE_STATE;
    }

    // Check valid port number.
    std::string portNum = argv[PORT_ARGUMENT_INDEX];
    for (int i = 0; i < portNum.length(); ++i)
    {
        if (!isdigit(portNum[i]))
        {
            std::cout << USAGE_MSG;  // TODO: Check new line.
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


/*-----=  Server Input Functions  =-----*/


// TODO: Doxygen.
static void terminateServer()
{
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        close(i->socket);
    }
}

// TODO: Doxygen.
static void handleServerInput()
{
    std::string currentInput;
    std::getline(std::cin, currentInput);

    if (currentInput.compare("EXIT") == 0)  // TODO: Magic Number.
    {
        // If the server received the EXIT input, it should terminate.
        // TODO: Release all resources.
        terminateServer();
        std::cout << SERVER_EXIT_MSG;  // TODO: Check new line.
        exit(EXIT_SUCCESS);
    }
}


/*-----=  Get Connection Functions  =-----*/


// TODO: Doxygen.
static bool checkAvailableClientName(const char *clientName)
{
    for (auto i = clients.begin(); i != clients.end(); ++i)
    {
        if (i->name.compare(clientName) == EQUAL_COMPARISON)
        {
            std::cout << "Client is not available" << std::endl;
            return false;
        }
    }
    return true;
}

// TODO: Doxygen.
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

// TODO: Doxygen.
static void createNewClient(const clientName_t name, const int socket,
                            fd_set *readFDs)
{
    Client client;
    client.name = name;
    client.socket = socket;
    clients.push_back(client);
    FD_SET(client.socket, readFDs);
    activeClients++;
}

// TODO: Doxygen.
static void handleNewConnection(const int welcomeSocket, fd_set *readFDs)
{
    int connectionState = SUCCESS_STATE;
    char clientName[MAX_NAME_SIZE + NULL_TERMINATOR_COUNT] = {NULL};

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
        read(connectionSocket, clientName, MAX_NAME_SIZE);
        std::cout << "Request for client name: " << clientName << std::endl;  // TODO: Delete This.

        // Check if the name is available.
        if (checkAvailableClientName(clientName))
        {
            createNewClient(clientName, connectionSocket, readFDs);
            connectionState = SUCCESS_STATE;
        }
        else
        {
            connectionState = FAILURE_STATE;
        }
    }

    if (connectionState)
    {
        // If the new connection failed.
        // Send to this client that the connection is failed.
        const char *state = "0"; // TODO: Magic Number.
        write(connectionSocket, state, 1);  // TODO: Magic Number.
        std::cout << clientName << " failed to connect." << std::endl;  // TODO: Magic Number.
        // Close the socket stream.
        close(connectionSocket);
    }
    else
    {
        // Send to this client that the connection is successful.
        const char *state = "1";  // TODO: Magic Number.
        write(connectionSocket, state, 1);  // TODO: Magic Number.
        std::cout << clientName << " connected." << std::endl;  // TODO: Magic Number.
    }

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
        return FAILURE_STATE;  // TODO: Check this return value.
    }

    // TODO: Check maybe there should be writeFD Set.
    // The FD Set for the server to read from.
    fd_set readFDs;
    FD_ZERO(&readFDs);
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
            systemCallError("select", errno);  // TODO: Magic Number.
            return FAILURE_STATE;
        }

        if (FD_ISSET(STDIN_FILENO, &currentFDs))
        {
            std::cout << "-- Standard Input --" << std::endl;  // TODO: Delete This.
            handleServerInput();
        }
        else if (FD_ISSET(welcomeSocket, &currentFDs))
        {
            std::cout << "-- Welcome --" << std::endl;  // TODO: Delete This.
            handleNewConnection(welcomeSocket, &readFDs);
        }
        else
        {
            std::cout << "-- From Clients --" << std::endl;  // TODO: Delete This.
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
}