// TODO: Move parse + process message to the header file.
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
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termio.h>


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
 * @def NULL_TERMINATOR_COUNT 1
 * @brief A Macro that sets the count for a null terminator in a string.
 */
#define NULL_TERMINATOR_COUNT 1

/**
 * @def WHITE_SPACE_SEPARATOR " "
 * @brief A Macro that sets the white space message separator.
 */
#define WHITE_SPACE_SEPARATOR " "

/**
 * @def EMPTY_MSG ""
 * @brief A Macro that sets the empty message.
 */
#define EMPTY_MSG ""

/**
 * @def MSG_SUFFIX "."
 * @brief A Macro that sets the error message suffix.
 */
#define MSG_SUFFIX "."

/**
 * @def GROUP_CLIENTS_DELIM ','
 * @brief A Macro that sets the group clients delimiter in create group command.
 */
#define GROUP_CLIENTS_DELIM ','

/**
 * @def GROUP_SEP ","
 * @brief A Macro that sets the group clients separator in create group command.
 */
#define GROUP_SEP ","

/**
 * @def WHITE_SPACE_DELIM ' '
 * @brief A Macro that sets the white space delimiter.
 */
#define WHITE_SPACE_DELIM ' '

/**
 * @def SYSTEM_CALL_ERROR_MSG_PREFIX "ERROR:"
 * @brief A Macro that sets the error message prefix for a system called fail.
 */
#define SYSTEM_CALL_ERROR_MSG_PREFIX "ERROR:"

/**
 * @def LOGOUT_SUCCESS_MSG "Unregistered successfully."
 * @brief A Macro that sets the message upon a successful logout.
 */
#define LOGOUT_SUCCESS_MSG "Unregistered successfully."

/**
 * @def WHO_FAILURE_MSG "ERROR: failed to receive list of connected clients."
 * @brief A Macro that sets the message upon a failure of who command.
 */
#define WHO_FAILURE_MSG "ERROR: failed to receive list of connected clients."

/**
 * @def WHO_REQUEST_MSG "Requests the currently connected client names."
 * @brief A Macro that sets the message upon a the who request.
 */
#define WHO_REQUEST_MSG "Requests the currently connected client names."

/**
 * @def EXIT_COMMAND "exit"
 * @brief A Macro that sets the command exit.
 */
#define EXIT_COMMAND "exit"

/**
 * @def WHO_COMMAND "who"
 * @brief A Macro that sets the command who.
 */
#define WHO_COMMAND "who"

/**
 * @def CREATE_GROUP_COMMAND "create_group"
 * @brief A Macro that sets the command create group.
 */
#define CREATE_GROUP_COMMAND "create_group"

/**
 * @def SEND_COMMAND "send"
 * @brief A Macro that sets the command send.
 */
#define SEND_COMMAND "send"

/**
 * @def MSG_BEGIN_INDEX 0
 * @brief A Macro that sets the value of the message begin index.
 */
#define MSG_BEGIN_INDEX 0

/**
 * @def CONNECTION_FAIL_STATE '0'
 * @brief A Macro that sets the value of connection fail state.
 */
#define CONNECTION_FAIL_STATE '0'

/**
 * @def CONNECTION_SUCCESS_STATE '1'
 * @brief A Macro that sets the value of connection success state.
 */
#define CONNECTION_SUCCESS_STATE '1'

/**
 * @def CONNECTION_IN_USE_STATE '2'
 * @brief A Macro that sets the value of connection fail if client name in use.
 */
#define CONNECTION_IN_USE_STATE '2'

/**
 * @def LOGOUT_SUCCESS_STATE '1'
 * @brief A Macro that sets the value of logout success state.
 */
#define LOGOUT_SUCCESS_STATE '1'

/**
 * @def MSG_TERMINATOR '\n'
 * @brief A Macro that sets the message terminator in the server/client.
 */
#define MSG_TERMINATOR '\n'

/**
 * @def TAG_CHAR_BASE '0'
 * @brief A Macro that sets the base value of calculating tag characters.
 */
#define TAG_CHAR_BASE '0'

/**
 * @def SOCKET_ID_BOUND 0
 * @brief A Macro that sets the lower bound of socket ID value.
 */
#define SOCKET_ID_BOUND 0

/**
 * @def EQUAL_COMPARISON 0
 * @brief A Macro that sets the value of equal strings while comparing them.
 */
#define EQUAL_COMPARISON 0

/**
 * @def READ_CHUNK 256
 * @brief A Macro that sets the read chunk size.
 */
#define READ_CHUNK 256

/**
 * @def INITIAL_READ_COUNT 0
 * @brief A Macro that sets the initial value of read byte count.
 */
#define INITIAL_READ_COUNT 0

/**
 * @def INITIAL_WRITE_COUNT 0
 * @brief A Macro that sets the initial value of write byte count.
 */
#define INITIAL_WRITE_COUNT 0


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

/**
 * @def WRITE_NAME "write"
 * @brief A Macro that sets function name for write.
 */
#define WRITE_NAME "write"

/**
 * @def SELECT_NAME "select"
 * @brief A Macro that sets function name for select.
 */
#define SELECT_NAME "select"


/*-----=  Type Definitions & Enums  =-----*/


/**
 * @brief Type Definition for the port number.
 */
typedef unsigned short portNumber_t;

/**
 * @brief Type Definition for the client name.
 */
typedef std::string clientName_t;

/**
 * @brief Type Definition for the group name.
 */
typedef std::string groupName_t;

/**
 * @brief Type Definition for a general message.
 */
typedef std::string message_t;

/**
 * @brief Enum for the types of messages types that the server can receive.
 */
enum MessageTag { CREATE_GROUP, SEND, WHO, CLIENT_EXIT, SERVER_EXIT };


/*-----=  Server/Client Functions  =-----*/


/**
 * @brief A function that handles the system call error procedure and print out
 *        an informative message.
 * @param callName The system call name.
 * @param errorNumber The error number.
 */
void systemCallError(const std::string callName, const int errorNumber)
{
    std::cerr << SYSTEM_CALL_ERROR_MSG_PREFIX << WHITE_SPACE_SEPARATOR
              << callName << WHITE_SPACE_SEPARATOR
              << errorNumber << MSG_SUFFIX << std::endl;
}

/**
 * @brief Validates the given port number.
 * @param portNumber The port number to validate.
 * @return 0 if the port number is a valid number, -1 otherwise.
 */
static int validatePortNumber(std::string const portNumber)
{
    for (int i = 0; i < portNumber.length(); ++i)
    {
        if (!isdigit(portNumber[i]))
        {
            return FAILURE_STATE;
        }
    }
    return SUCCESS_STATE;
}

/**
 * @brief Reads data from the given socket into the given buffer.
 * @param socketID The socket to read from.
 * @param buffer The buffer to read into.
 * @return The number of bytes read or -1 in case of failure.
 */
static int readData(const int socketID, message_t &buffer)
{
    int totalCount = INITIAL_READ_COUNT;
    while (true)
    {
        char currentChunk[READ_CHUNK + NULL_TERMINATOR_COUNT] = {NULL};
        // Each time read some chunk of the message.
        ssize_t currentCount = read(socketID, currentChunk, READ_CHUNK);
        if (currentCount < 0)
        {
            systemCallError(READ_NAME, errno);
            return FAILURE_STATE;
        }
        totalCount += currentCount;
        buffer += currentChunk;
        if (buffer.back() == MSG_TERMINATOR)
        {
            // If the read operation has read the entire message.
            // remove the NEW_LINE we added to the message.
            buffer.pop_back();
            break;
        }
    }
    return totalCount;
}

/**
 * @brief Writes data into the given socket from the given buffer.
 * @param socketID The socket to write into.
 * @param buffer The buffer to write from.
 * @return The number of bytes written or -1 in case of failure.
 */
static int writeData(const int socketID, const message_t &buffer)
{
    // Add to the message the NEW_LINE which indicates the end of the message.
    message_t modified = buffer + (char) MSG_TERMINATOR;

    int totalCount = INITIAL_WRITE_COUNT;
    auto totalSize = modified.length();

    while (true)
    {
        auto currentCount = write(socketID, modified.c_str(), totalSize);
        if (currentCount < 0)
        {
            systemCallError(WRITE_NAME, errno);
            return FAILURE_STATE;
        }
        totalCount += currentCount;
        totalSize -= currentCount;
        // TODO: Check if need to increment the buffer by current count.
        if (totalCount == modified.length())
        {
            return totalCount;
        }
    }
}

#endif
