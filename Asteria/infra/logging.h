#ifndef LOGGING_H
#define LOGGING_H

#include <QTextStream>
#include <QFile>

class Logging {
public:
    Logging();

    static QTextStream stdOutStream;
    static QTextStream stdErr;
    static QFile logFile;
    static QTextStream logFileOut;

};




#endif // LOGGING_H
