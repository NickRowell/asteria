#ifndef FILEUTIL_H
#define	FILEUTIL_H

#include <vector>
#include <string>

class FileUtil
{
    
public:
    FileUtil();

    /**
     * @brief deleteFilePath
     *
     * Deletes the contents of the given file path; if this is a file then it is simply deleted, if this is
     * a directory then the directory and all of it's contents are recursively deleted.
     * Only directories and regular files are deleted, which is all we should ever need to do. In case the
     * function encounters other types of object then these will be left in place.
     * @param path
     * @return Bool stating whether the path was successfully deleted.
     */
    static bool deleteFilePath(std::string path);

    static bool createDir(std::string parent, std::string child);

    static bool createDirs(std::string topLevel, std::vector<std::string> subLevels);
};

#endif	/* GLUTILS_H */

