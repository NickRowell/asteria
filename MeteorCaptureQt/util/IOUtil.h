/*
 * IOUtil.h
 *
 *  Created on: 26 Dec 2016
 *      Author: nrowell
 */

#ifndef IOUTIL_H_
#define IOUTIL_H_

#include <vector>
#include <string>

std::vector<std::string> split(const std::string &s, char delim);

std::string intToString(int nb);

int xioctl (int fh, int request, void *arg);

#endif /* IOUTIL_H_ */
