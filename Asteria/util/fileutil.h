#ifndef FILEUTIL_H
#define	FILEUTIL_H

#include <vector>
#include <string>
#include <map>

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

    /**
     * @brief Creates a child directory with the given name in the parent directory of the given path.
     * @param parent
     *  The full path to the parent directory.
     * @param child
     *  The name of the desired child directory.
     * @return
     *  True if the child directory was created successfully, false otherwise.
     */
    static bool createDir(std::string parent, std::string child);

    /**
     * @brief Creates a series of nested directories with the given names in the parent directory of the given path.
     * @param topLevel
     *  The full path to the parent directory.
     * @param subLevels
     *  The names of each successive sub-directory to create in a nested fashion in the parent directory.
     * @return
     *  True if all the sub-directories could be created, false otherwise.
     */
    static bool createDirs(std::string topLevel, std::vector<std::string> subLevels);

    /**
     * @brief Checks if the given pathname exists and is a regular file.
     * @param path
     *  The path to check.
     * @return
     *  True if the path points to a regular file, false otherwise (non-existant / not a regular file).
     */
    static bool fileExists(std::string path);

    /**
     * @brief Maps the contents of the event or calibration directories by time. The events and calibration
     * directories are structured according to e.g.
     *
     * 2017/
     *  - 03/
     *     - 13/
     *        - 2018-03-13T22:27:41.891Z/
     *
     * This function walks the directory tree and builds a map of the full paths to the leaf nodes (the
     * 2018-03-13T22:27:41.891Z directories) by event/calibration time in milliseconds since 1970. This is useful
     * in locating the most recent calibration and in building the models of the analysis and calibration
     * directory contents for the GUI.
     *
     * @param rootPath
     *  The top level directory
     * @return
     *  A map of the full paths to the leaf nodes (the 2018-03-13T22:27:41.891Z directories) by
     * event/calibration time in milliseconds since 1970.
     */
    static std::map<long long, std::string> mapVideoDirectory(std::string rootPath);
};

#endif

