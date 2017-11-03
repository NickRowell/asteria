#include "infra/calibrationinventory.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "util/renderutil.h"
#include "util/serializationutil.h"

#include <dirent.h>
#include <regex>
#include <fstream>
#include <functional>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

CalibrationInventory::CalibrationInventory() {

}

CalibrationInventory::CalibrationInventory(const std::vector<std::shared_ptr<Imageuc> > &calibrationFrames) : calibrationFrames(calibrationFrames) {

}

CalibrationInventory *CalibrationInventory::loadFromDir(std::string path) {

    std::string raw = path + "/raw";
    std::string processed = path + "/processed";

    // Load the raw images
    DIR *dir;
    if ((dir = opendir (raw.c_str())) == NULL) {
        // Couldn't open the directory!
        return NULL;
    }

    CalibrationInventory * inv = new CalibrationInventory();

    // Loop over the contents of the directory
    struct dirent *child;
    while ((child = readdir (dir)) != NULL) {

        // Skip the . and .. directories
        if(strcmp(child->d_name,".") == 0 || strcmp(child->d_name,"..") == 0) {
            continue;
        }

        // Match files with names starting with UTC string, e.g. 2017-06-14T19:41:09.282Z.pgm
        // These are the raw calibration frames
        if(std::regex_search(child->d_name, TimeUtil::utcRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = raw + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            std::ifstream input(childPath);
            auto seq = std::make_shared<Imageuc>();
            input >> *seq;
            inv->calibrationFrames.push_back(seq);
            input.close();
        }
    }
    closedir (dir);

    // Sort the calibration image sequence into ascending order of capture time
    std::sort(inv->calibrationFrames.begin(), inv->calibrationFrames.end(), Imageuc::comparePtrToImage);

    // Load the median, background and noise images

    // Load the median image
    std::string medianImagePath = processed + "/median.pgm";
    if(FileUtil::fileExists(medianImagePath)) {
        std::ifstream ifs(medianImagePath);
        auto medianImage = std::make_shared<Imageuc>();
        ifs >> *medianImage;
        inv->medianImage = medianImage;
        ifs.close();
    }

    // Load the background image
    std::string bkgImagePath = processed + "/background.pgm";
    if(FileUtil::fileExists(bkgImagePath)) {
        std::ifstream ifs(bkgImagePath);
        auto bkgImage = std::make_shared<Imageuc>();
        ifs >> *bkgImage;
        inv->backgroundImage = bkgImage;
        ifs.close();
    }

    // Load the noise image
    std::string noiseImagePath = processed + "/noise.pfm";
    if(FileUtil::fileExists(noiseImagePath)) {
        std::ifstream ifs(noiseImagePath);
        auto noiseImage = std::make_shared<Imaged>();
        ifs >> *noiseImage;
        inv->noiseImage = noiseImage;
        ifs.close();
    }

    // Load the additional serialized calibration data fields

    std::string calibrationData = processed + "/calibration.xml";
    if(FileUtil::fileExists(calibrationData)) {
        std::ifstream ifs(calibrationData);
        boost::archive::xml_iarchive ia(ifs, boost::archive::no_header);
        ia & BOOST_SERIALIZATION_NVP(inv->epochTimeUs);
        ia & BOOST_SERIALIZATION_NVP(inv->sources);
        ia & BOOST_SERIALIZATION_NVP(inv->readNoiseAdu);
        ia & BOOST_SERIALIZATION_NVP(inv->q_sez_cam);
//        ia & BOOST_SERIALIZATION_NVP(inv->cam);
        ifs.close();
    }

    return inv;
}

void CalibrationInventory::saveToDir(std::string topLevelPath) {

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::epochToUtcString(calibrationFrames.front()->epochTimeUs);
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

    // Write out the raw calibration frames
    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {
        Imageuc &image = *calibrationFrames[i];
        char filename [100];
        std::string utcFrame = TimeUtil::epochToUtcString(image.epochTimeUs);
        sprintf(filename, "%s/%s.pgm", raw.c_str(), utcFrame.c_str());
        std::ofstream out(filename);
        out << image;
        out.close();
    }

    // Write out processed data

    char filename [100];

    // Write out the median image
    sprintf(filename, "%s/median.pgm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *medianImage;
        out.close();
    }

    // Write out the background image
    sprintf(filename, "%s/background.pgm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *backgroundImage;
        out.close();
    }

    // Write out the variance image
    sprintf(filename, "%s/noise.pfm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *noiseImage;
        out.close();
    }

    // Save calibration data to text file
    char calibrationDataFilename [100];
    sprintf(calibrationDataFilename, "%s/calibration.xml", processed.c_str());
    {
        std::ofstream ofs(calibrationDataFilename);
        boost::archive::xml_oarchive oa(ofs, boost::archive::no_header);
        oa & BOOST_SERIALIZATION_NVP(epochTimeUs);
        oa & BOOST_SERIALIZATION_NVP(sources);
        oa & BOOST_SERIALIZATION_NVP(readNoiseAdu);
        oa & BOOST_SERIALIZATION_NVP(q_sez_cam);
//        oa & BOOST_SERIALIZATION_NVP(cam);
        ofs.close();
    }

    // Now compute and write out some additional products that are not formally used by the calibration
    // but are useful for visualisation and debugging.

    // Create an RGB image of the extracted sources and save to file
    std::shared_ptr<Imageui> image = RenderUtil::renderSourcesImage(sources, medianImage->width, medianImage->height);
    sprintf(filename, "%s/sources.ppm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *image;
        out.close();
    }

    char rnPlotfilename [100];
    sprintf(rnPlotfilename, "%s/readnoise.png", processed.c_str());

    std::stringstream ss;

    // This script produces a plot of scatter versus signal level
    ss << "set terminal pngcairo dashed enhanced color size 640,480 font \"Helvetica\"\n";
    ss << "set style line 20 lc rgb \"#ddccdd\" lt 1 lw 1.5\n";
    ss << "set style line 21 lc rgb \"#ddccdd\" lt 1 lw 0.5\n";
    ss << "set xlabel \"Signal [ADU]\" font \"Helvetica,14\"\n";
    ss << "set xtics out nomirror offset 0.0,0.0 rotate by 0.0 scale 1.0\n";
    ss << "set mxtics 2\n";
    ss << "set xrange [*:*]\n";
    ss << "set format x \"%g\"\n";
    ss << "set ylabel \"σ^{2} [ADU^{2}]\" font \"Helvetica,14\"\n";
    ss << "set ytics out nomirror offset 0.0,0.0 rotate by 0.0 scale 1.0\n";
    ss << "set mytics 2\n";
    ss << "set yrange [0:*]\n";
    ss << "set format y \"%g\"\n";
    ss << "set key off\n";
    ss << "set grid xtics mxtics ytics mytics back ls 20, ls 21\n";
    ss << "set title \"Readnoise estimate\"\n";
    ss << "set output \"" << rnPlotfilename << "\"\n";
    ss << "plot \"-\" w d notitle\n";

    for(unsigned int i=0; i<medianImage->width*medianImage->height; i++) {
        char buffer[200] = "";
        sprintf(buffer, "%d\t%f\n", medianImage->rawImage[i], noiseImage->rawImage[i]);
        ss << buffer;
    }
    ss << "e\n";

    // Get the path to a temporary file
    std::string tmpFileName = std::tmpnam(nullptr);
    std::ofstream ofs (tmpFileName, std::ofstream::out);
    ofs << ss.str();
    ofs.close();

    char command [100];
    sprintf(command, "gnuplot < %s", tmpFileName.c_str());
    system(command);
}

void CalibrationInventory::deleteCalibration() {
    // TODO: use this to delete each file of a calibration specifically rather than
    // relying on deleting everything in the directory, which is unsafe.
}
