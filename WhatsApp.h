/**
 * @file whatsapp.h
 * @author Itai Tagar <itagar>
 *
 * @brief A Header for the WhatsApp Framework (Server/Client).
 */


#ifndef WHATSAPP_H
#define WHATSAPP_H


/*-----=  Includes  =-----*/


#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>


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
 * @def SOCKET_ID_BOUND 0
 * @brief A Macro that sets the lower bound of socket ID value.
 */
#define SOCKET_ID_BOUND 0


/*-----=  System Calls Name Definitions  =-----*/


/**
 * @def GETHOSTNAME_NAME "gethostname"
 * @brief A Macro that sets function name for gethostname.
 */
#define GETHOSTNAME_NAME "gethostname"

/**
 * @def GETHOSTBYNAME_NAME "gethostbyname"
 * @brief A Macro that sets function name for gethostbyname.
 */
#define GETHOSTBYNAME_NAME "gethostbyname"

/**
 * @def SOCKET_NAME "socket"
 * @brief A Macro that sets function name for socket.
 */
#define SOCKET_NAME "socket"

/**
 * @def CLOSE_NAME "close"
 * @brief A Macro that sets function name for close.
 */
#define CLOSE_NAME "close"

/**
 * @def BIND_NAME "bind"
 * @brief A Macro that sets function name for bind.
 */
#define BIND_NAME "bind"

/**
 * @def LISTEN_NAME "listen"
 * @brief A Macro that sets function name for listen.
 */
#define LISTEN_NAME "listen"

/**
 * @def ACCEPT_NAME "accept"
 * @brief A Macro that sets function name for accept.
 */
#define ACCEPT_NAME "accept"

/**
 * @def CONNECT_NAME "connect"
 * @brief A Macro that sets function name for connect.
 */
#define CONNECT_NAME "connect"

/**
 * @def READ_NAME "read"
 * @brief A Macro that sets function name for read.
 */
#define READ_NAME "read"


/*-----=  Type Definitions  =-----*/


/**
 * @brief Type Definition for the port number.
 */
typedef unsigned short portNumber_t;

// TODO: Doxygen.
typedef struct Client
{
    std::string clientName;
    std::string clientGroup;
    int clientSocket;
    
} Client;


/*-----=  Error Functions  =-----*/


// TODO: Doxygen.
void systemCallError(const std::string callName, const int errorNumber)
{
    // TODO: Check cout or cerr.
    std::cerr << SYSTEM_CALL_ERROR_MSG_PREFIX << ERROR_MSG_SEPARATOR << callName
              << ERROR_MSG_SEPARATOR << errorNumber << std::endl;
}


#endif
