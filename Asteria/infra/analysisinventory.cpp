#include "infra/analysisinventory.h"
#include "util/timeutil.h"

#include <regex>
#include <fstream>
#include <iostream>
#include <functional>

AnalysisInventory::AnalysisInventory() {

}

AnalysisInventory *AnalysisInventory::loadFromDir(std::string path) {

    // Load all the images found here...
    DIR *dir;
    if ((dir = opendir (path.c_str())) == NULL) {
        // Couldn't open the directory!
        return NULL;
    }

    // Regex suitable for identifying images with filenames e.g. 2017-06-14T19:41:09.282Z
    const std::regex utcRegex = TimeUtil::getUtcRegex();
    // Regex suitable for identifying images with filenames starting 'peakhold'
    const std::regex peakHoldRegex = std::regex("peakhold");

    AnalysisInventory * inv = new AnalysisInventory();

    // Loop over the contents of the directory
    struct dirent *child;
    while ((child = readdir (dir)) != NULL) {

        // Skip the . and .. directories
        if(strcmp(child->d_name,".") == 0 || strcmp(child->d_name,"..") == 0) {
            continue;
        }

        // Parse the filename to decide what type of file it is using regex

        // Match files with names starting with UTC string, e.g. 2017-06-14T19:41:09.282Z.pgm
        // These are the raw frames from the sequence
        if(std::regex_search(child->d_name, utcRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            std::ifstream input(childPath);
            auto seq = std::make_shared<Image>();
            input >> *seq;
            inv->eventFrames.push_back(seq);
            input.close();
        }

        // Detect the peak hold image
        if(std::regex_search(child->d_name, peakHoldRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the peakHold variable
            std::ifstream input(childPath);
            auto peakHoldImage = std::make_shared<Image>();
            input >> *peakHoldImage;
            inv->peakHold = peakHoldImage;
            input.close();
        }
    }
    closedir (dir);

    // Sort the image sequence into ascending order of capture time
    std::sort(inv->eventFrames.begin(), inv->eventFrames.end(), Image::comparePtrToImage);

    // Generate annnotated images for each raw image, showing analysis of individual frame
    for(unsigned int i=0; i<inv->eventFrames.size(); i++) {
        inv->eventFrames[i]->generateAnnotatedImage();
    }
    // Generate annotated image for the peakHold image, showing analysis of clip
    inv->peakHold->generatePeakholdAnnotatedImage(inv->eventFrames);

    return inv;
}

void AnalysisInventory::saveToDir(std::string path) {

}

void AnalysisInventory::deleteClip() {
    // TODO: use this to delete each file of an analysis specifically rather than
    // relying on deleting everything in the directory, which is unsafe.
}
