#include "infra/calibrationinventory.h"
#include "util/timeutil.h"

#include <regex>
#include <fstream>
#include <iostream>
#include <functional>

CalibrationInventory::CalibrationInventory() {

}

CalibrationInventory *CalibrationInventory::loadFromDir(std::string path) {

    // Load all the images found here...
    DIR *dir;
    if ((dir = opendir (path.c_str())) == NULL) {
        // Couldn't open the directory!
        return NULL;
    }

    // Regex suitable for identifying images with filenames e.g. 2017-06-14T19:41:09.282Z
    const std::regex utcRegex = TimeUtil::getUtcRegex();
    // Regex suitable for identifying images with filenames starting 'median'
    const std::regex medianRegex = std::regex("median");

    CalibrationInventory * inv = new CalibrationInventory();

    // Loop over the contents of the directory
    struct dirent *child;
    while ((child = readdir (dir)) != NULL) {

        // Skip the . and .. directories
        if(strcmp(child->d_name,".") == 0 || strcmp(child->d_name,"..") == 0) {
            continue;
        }

        // Parse the filename to decide what type of file it is using regex

        // Match files with names starting with UTC string, e.g. 2017-06-14T19:41:09.282Z.pgm
        // These are the raw calibration frames
        if(std::regex_search(child->d_name, utcRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            std::ifstream input(childPath);
            auto seq = std::make_shared<Image>();
            input >> *seq;
            inv->calibrationFrames.push_back(seq);
            input.close();
        }

        // Detect the median image
        if(std::regex_search(child->d_name, medianRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path + "/" + child->d_name;
            // Load the image from file
            std::ifstream input(childPath);
            auto medianImage = std::make_shared<Image>();
            input >> *medianImage;
            inv->medianImage = medianImage;
            input.close();
        }

        // Detect the text file containing the calibration data
        if(strcmp(child->d_name, "calibration.dat") == 0) {
            fprintf(stderr, "Found %s\n", child->d_name);
        }
    }
    closedir (dir);

    // Sort the calibration image sequence into ascending order of capture time
    std::sort(inv->calibrationFrames.begin(), inv->calibrationFrames.end(), Image::comparePtrToImage);

    return inv;
}

void CalibrationInventory::saveToDir(std::string path) {

}
