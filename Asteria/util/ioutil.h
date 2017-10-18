#ifndef IOUTIL_H
#define IOUTIL_H

#include <vector>
#include <string>
#include <sstream>

#include <QString>
//#include <QMouseEvent>
//#include <QWheelEvent>

class IoUtil {

public:
    IoUtil();

    static std::vector<std::string> split(const std::string &s, const char &delim);

    static std::string intToString(int nb);

    static QString mouseButtonEnumNameFromValue(const Qt::MouseButton button);

    static int xioctl (int fh, int request, void *arg);
};

#endif /* IOUTIL_H */
