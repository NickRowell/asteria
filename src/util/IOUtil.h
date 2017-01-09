/*
 * IOUtil.h
 *
 *  Created on: 26 Dec 2016
 *      Author: nrowell
 */

#ifndef IOUTIL_H_
#define IOUTIL_H_

#include <vector>     // provides vector
#include <string>     // provides string
#include <utility>    // provides pair
#include <fcntl.h>    // Provides open(...)
#include <unistd.h>   // Provides access(...)
#include <stdio.h>    // Provides perror(...)
#include <errno.h>    // provides errno
#include <iostream>
#include <sstream>
#include <list>
#include <sys/ioctl.h>

using namespace std;

class IOUtil {
public:
	IOUtil();
	virtual ~IOUtil();

	string intToString(int nb);

	int xioctl (int fh, int request, void *arg);

private:

};

#endif /* IOUTIL_H_ */
