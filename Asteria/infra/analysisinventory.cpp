#include "infra/analysisinventory.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "util/serializationutil.h"
#include "util/jpgutil.h"

#include <fstream>
#include <iostream>
#include <functional>
#include <memory>
#include <dirent.h>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

AnalysisInventory::AnalysisInventory() {

}

AnalysisInventory::AnalysisInventory(const std::vector<std::shared_ptr<Imageuc>> &eventFrames) : eventFrames(eventFrames), locs(eventFrames.size()) {

    for(unsigned int i = 0; i < eventFrames.size(); ++i) {
        locs[i].epochTimeUs = eventFrames[i]->epochTimeUs;
    }

    // Read image width & height from first frame
    unsigned int width = eventFrames[0]->width;
    unsigned int height = eventFrames[0]->height;

    // Create a peak hold image
    peakHold = std::make_shared<Imageuc>(width, height);
    peakHold->epochTimeUs = eventFrames[0]->epochTimeUs;
    for(unsigned int i = 0; i < eventFrames.size(); ++i) {
        Imageuc &image = *eventFrames[i];
        // Compute peak hold image
        for(unsigned int k=0; k<image.height; k++) {
            for(unsigned int l=0; l<image.width; l++) {
                unsigned int offset = k*image.width + l;
                peakHold->rawImage[offset] = std::max(peakHold->rawImage[offset], image.rawImage[offset]);
            }
        }
    }
}

AnalysisInventory *AnalysisInventory::loadFromDir(std::string path) {

    std::string raw = path + "/raw";
    std::string processed = path + "/processed";

    // Load the raw images
    DIR *dir;
    if ((dir = opendir (raw.c_str())) == NULL) {
        // Couldn't open the directory!
        return NULL;
    }

    AnalysisInventory * inv = new AnalysisInventory();

    // Load raw data products (the captured images)
    struct dirent *child;
    while ((child = readdir (dir)) != NULL) {

        // Skip the . and .. directories
        if(strcmp(child->d_name,".") == 0 || strcmp(child->d_name,"..") == 0) {
            continue;
        }

        // Match files with names starting with UTC string, e.g. 2017-06-14T19:41:09.282Z.pgm
        // These are the raw frames from the sequence
        if(std::regex_search(child->d_name, TimeUtil::utcRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = raw + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            std::ifstream input(childPath);
            auto seq = std::make_shared<Imageuc>();
            input >> *seq;
            inv->eventFrames.push_back(seq);
            input.close();
        }
    }
    closedir (dir);

    // Sort the image sequence into ascending order of capture time
    std::sort(inv->eventFrames.begin(), inv->eventFrames.end(), Imageuc::comparePtrToImage);

    // Load derived data products

    // Load peakhold image
    std::string peakHoldImage = processed + "/peakhold.pgm";
    if(FileUtil::fileExists(peakHoldImage)) {
        std::ifstream ifs(peakHoldImage);
        auto peakHoldImage = std::make_shared<Imageuc>();
        ifs >> *peakHoldImage;
        inv->peakHold = peakHoldImage;
        ifs.close();
    }

    std::string locationData = processed + "/localisation.xml";
    if(FileUtil::fileExists(locationData)) {
        std::ifstream ifs(locationData);
        boost::archive::xml_iarchive ia(ifs, boost::archive::no_header);
        // write class instance to archive
        ia & BOOST_SERIALIZATION_NVP(inv->locs);
        ifs.close();
    }
    else {
        // Initialise empty location data for each frame
        inv->locs = std::vector<MeteorImageLocationMeasurement>(inv->eventFrames.size(), MeteorImageLocationMeasurement());
    }

    // Sort the location measurements into ascending order of capture time
    std::sort(inv->locs.begin(), inv->locs.end());

    // Generate annnotated images for each raw image, showing analysis of individual frame
    for(unsigned int i=0; i<inv->eventFrames.size(); i++) {
        inv->eventFrames[i]->generateAnnotatedImage(inv->locs[i]);
    }
    // Generate annotated image for the peakHold image, showing analysis of clip
    inv->peakHold->generatePeakholdAnnotatedImage(inv->eventFrames, inv->locs);

    return inv;
}

void AnalysisInventory::saveToDir(std::string topLevelPath) {

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::epochToUtcString(eventFrames[0u]->epochTimeUs);
    std::string yyyy = TimeUtil::extractYearFromUtcString(utc);
    std::string mm = TimeUtil::extractMonthFromUtcString(utc);
    std::string dd = TimeUtil::extractDayFromUtcString(utc);

    std::vector<std::string> subLevels;
    subLevels.push_back(yyyy);
    subLevels.push_back(mm);
    subLevels.push_back(dd);
    subLevels.push_back(utc);
    std::string path = topLevelPath + "/" + yyyy + "/" + mm + "/" + dd + "/" + utc;

    if(!FileUtil::createDirs(topLevelPath, subLevels)) {
        fprintf(stderr, "Couldn't create directory %s\n", path.c_str());
        return;
    }

    // Create raw/ and processed/ subdirectories
    FileUtil::createDir(path, "raw");
    FileUtil::createDir(path, "processed");
    std::string raw = path + "/raw";
    std::string processed = path + "/processed";

    // Write out raw images

    for(unsigned int i = 0; i < eventFrames.size(); ++i) {

        Imageuc &image = *eventFrames[i];

        // Write the image data out to a file
        char filename [100];
        std::string utcFrame = TimeUtil::epochToUtcString(image.epochTimeUs);
        sprintf(filename, "%s/%s.pgm", raw.c_str(), utcFrame.c_str());

        // PGM (grey image)
        std::ofstream out(filename);
        out << image;
        out.close();
    }

    // Write out processed data

    // TODO: encode a video from the raw frames, for display on the website

    // Write out the peak hold image
    char filename [100];
    sprintf(filename, "%s/peakhold.pgm", processed.c_str());
    std::ofstream out(filename);
    out << *peakHold;
    out.close();

    // Write out the peak hold image in JPG format
    sprintf(filename, "%s/peakhold.jpg", processed.c_str());
    JpgUtil::writeJpeg(peakHold->rawImage, peakHold->width, peakHold->height, filename);

    // Write out the localisation information
    sprintf(filename, "%s/localisation.xml", processed.c_str());
    std::ofstream ofs(filename);
    boost::archive::xml_oarchive oa(ofs, boost::archive::no_header);
    // write class instance to archive
    oa & BOOST_SERIALIZATION_NVP(locs);
    ofs.close();
}

void AnalysisInventory::deleteClip() {
    // TODO: use this to delete each file of an analysis specifically rather than
    // relying on deleting everything in the directory, which is unsafe.
}
