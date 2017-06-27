#include "infra/logging.h"

#include <stdio.h>

QTextStream Logging::stdOutStream(stdout);
QTextStream Logging::stdErr(stderr);
QFile Logging::logFile("asteria.log");
QTextStream Logging::logFileOut(&Logging::logFile);
