#include "util/ioutil.h"

#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>

/**
 * @brief Tokenise the string
 * @param s The string
 * @param delim The token delimiter
 * @return Vector of string tokens
 */
std::vector<std::string> IoUtil::split(const std::string &s, const char &delim) {

    std::vector<std::string> tokens;
    std::stringstream   mySstream(s);
    std::string         temp;

    while( std::getline( mySstream, temp, delim ) ) {
        tokens.push_back( temp );
    }

    return tokens;
}

/**
 * Converts an int to a string
 * \param i Integer to convert to a string
 * \return	String containing the converted integer
 */
std::string IoUtil::intToString(int i){
    std::ostringstream oss;
    oss << i;
    std::string result = oss.str();
    return result;
}

/**
 * Wraps up the ioctl function (which manipulates the underlying device
 * parameters of special files). This wrapper calls the function until it
 * returns successfully, in case of interrupted system calls.
 *
 * \param fd Open file descriptor pointing to the device.
 * \param request The request to execute on the underlying device identified by the file descriptor.
 * \param arg Untyped pointer to memory, where the results of the request are written.
 * \return Error flag: 0 for success, -1 indicates error.
 */
int IoUtil::xioctl (int fd, int request, void *arg) {
	int r;

	do {
		r = ioctl(fd, request, arg);
	}
	while (-1 == r && EINTR == errno);

	return r;
}

/**
 * @brief Converts the enum type of a MouseButton to a QString description.
 * @param button
 *  The enum type representing the MouseButton
 * @return
 *  The name of the enum
 */
QString IoUtil::mouseButtonEnumNameFromValue(const Qt::MouseButton button) {

    if (button == Qt::NoButton)      return "NoButton";
    if (button == Qt::LeftButton)    return "LeftButton";
    if (button == Qt::RightButton)   return "RightButton";
    if (button == Qt::MiddleButton)  return "MiddleButton";
    if (button == Qt::BackButton)    return "BackButton";
    if (button == Qt::ForwardButton) return "ForwardButton";
    if (button == Qt::TaskButton)    return "TaskButton";
    if (button == Qt::ExtraButton4)  return "ExtraButton4";
    if (button == Qt::ExtraButton5)  return "ExtraButton5";
    if (button == Qt::ExtraButton6)  return "ExtraButton6";
    if (button == Qt::ExtraButton7)  return "ExtraButton7";
    if (button == Qt::ExtraButton8)  return "ExtraButton8";
    if (button == Qt::ExtraButton9)  return "ExtraButton9";
    if (button == Qt::ExtraButton10) return "ExtraButton10";
    if (button == Qt::ExtraButton11) return "ExtraButton11";
    if (button == Qt::ExtraButton12) return "ExtraButton12";
    if (button == Qt::ExtraButton13) return "ExtraButton13";
    if (button == Qt::ExtraButton14) return "ExtraButton14";
    if (button == Qt::ExtraButton15) return "ExtraButton15";
    if (button == Qt::ExtraButton16) return "ExtraButton16";
    if (button == Qt::ExtraButton17) return "ExtraButton17";
    if (button == Qt::ExtraButton18) return "ExtraButton18";
    if (button == Qt::ExtraButton19) return "ExtraButton19";
    if (button == Qt::ExtraButton20) return "ExtraButton20";
    if (button == Qt::ExtraButton21) return "ExtraButton21";
    if (button == Qt::ExtraButton22) return "ExtraButton22";
    if (button == Qt::ExtraButton23) return "ExtraButton23";
    if (button == Qt::ExtraButton24) return "ExtraButton24";
    qDebug("QMouseShortcutEntry::addShortcut contained Invalid Qt::MouseButton value");
    return "NoButton";
}
