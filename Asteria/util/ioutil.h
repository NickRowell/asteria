#ifndef IOUTIL_H
#define IOUTIL_H

#include <vector>
#include <string>
#include <sstream>


class IoUtil {

public:
    IoUtil();

    static std::vector<std::string> split(const std::string &s, char delim);

    static std::string intToString(int nb);

    static int xioctl (int fh, int request, void *arg);

    template<typename Out>
    static void split(const std::string &s, char delim, Out result);
};

template<typename Out>
void IoUtil::split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

#endif /* IOUTIL_H */
