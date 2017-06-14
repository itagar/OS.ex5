/**
 * @file whatsappServer.cpp
 * @author Itai Tagar <itagar>
 *
 * @brief An implementation of the WhatsApp Server.
 */


/*-----=  Includes  =-----*/


#include <iostream>
#include <string>
#include <cassert>


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
 * @def USAGE_MSG "Usage: <substring to search> <folders, separated by space>"
 * @brief A Macro that sets the error message when the usage is invalid.
 */
#define USAGE_MSG "Usage: whatsappServer portNum"


/**
 * @brief Checks whether the program received the desired number of arguments.
 *        In case of invalid arguments count the function output an error
 *        message specifying the correct usage.
 * @param argc The number of arguments given to the program.
 * @param argv The array of given arguments.
 */
static void checkArguments(int const argc, char * const argv[])
{
    // Check valid number of arguments.
    if (argc != VALID_ARGUMENTS_COUNT)
    {
        // TODO: Check cout or cerr.
        // TODO: Check new line.
        std::cout << USAGE_MSG << std::endl;
        return;
    }

    // Check valid port number.
    std::string portNum = argv[PORT_ARGUMENT_INDEX];
    for (int i = 0; i < portNum.length(); ++i)
    {
        if (!isdigit(portNum[i]))
        {
            std::cout << USAGE_MSG << std::endl;
            return;
        }
    }

    return;
}


/**
 * @brief The main function running the program.
 */
int main(int argc, char *argv[])
{
    checkArguments(argc, argv);

}