#include "infra/calibrationinventory.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "util/renderutil.h"
#include "util/serializationutil.h"
#include "optics/pinholecamera.h"
#include "optics/pinholecamerawithradialdistortion.h"

#include <dirent.h>
#include <regex>
#include <fstream>
#include <functional>

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

    // Load the signal, background and noise images

    // Load the signal image
    std::string signalPath = processed + "/signal.pfm";
    if(FileUtil::fileExists(signalPath)) {
        std::ifstream ifs(signalPath);
        auto signal = std::make_shared<Imaged>();
        ifs >> *signal;
        inv->signal = signal;
        ifs.close();
    }

    // Load the background image
    std::string backgroundPath = processed + "/background.pfm";
    if(FileUtil::fileExists(backgroundPath)) {
        std::ifstream ifs(backgroundPath);
        auto background = std::make_shared<Imaged>();
        ifs >> *background;
        inv->background = background;
        ifs.close();
    }

    // Load the noise image
    std::string noisePath = processed + "/noise.pfm";
    if(FileUtil::fileExists(noisePath)) {
        std::ifstream ifs(noisePath);
        auto noise = std::make_shared<Imaged>();
        ifs >> *noise;
        inv->noise = noise;
        ifs.close();
    }

    // Load the additional serialized calibration data fields

    std::string calibrationData = processed + "/calibration.xml";
    if(FileUtil::fileExists(calibrationData)) {
        std::ifstream ifs(calibrationData);
        boost::archive::xml_iarchive ia(ifs, boost::archive::no_header);
        ia & BOOST_SERIALIZATION_NVP(inv->epochTimeUs);
        ia & BOOST_SERIALIZATION_NVP(inv->sources);
        ia & BOOST_SERIALIZATION_NVP(inv->xms);
        ia & BOOST_SERIALIZATION_NVP(inv->readNoiseAdu);
        ia & BOOST_SERIALIZATION_NVP(inv->q_sez_cam);
        ia & BOOST_SERIALIZATION_NVP(inv->cam);
        ia & BOOST_SERIALIZATION_NVP(inv->longitude);
        ia & BOOST_SERIALIZATION_NVP(inv->latitude);
        ia & BOOST_SERIALIZATION_NVP(inv->altitude);
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

    // Write out the signal image
    sprintf(filename, "%s/signal.pfm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *signal;
        out.close();
    }
    // Write out pgm version of the signal image for offline inspection
    sprintf(filename, "%s/signal.pgm", processed.c_str());
    {
        std::ofstream out(filename);
        out << Imageuc(*signal);
        out.close();
    }

    // Write out the background image
    sprintf(filename, "%s/background.pfm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *background;
        out.close();
    }
    // Write out pgm version of the background image for offline inspection
    sprintf(filename, "%s/background.pgm", processed.c_str());
    {
        std::ofstream out(filename);
        out << Imageuc(*background);
        out.close();
    }

    // Write out the noise image
    sprintf(filename, "%s/noise.pfm", processed.c_str());
    {
        std::ofstream out(filename);
        out << *noise;
        out.close();
    }
    // Write out pgm version of the noise image for offline inspection
    sprintf(filename, "%s/noise.pgm", processed.c_str());
    {
        std::ofstream out(filename);
        out << Imageuc(*noise);
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
        oa & BOOST_SERIALIZATION_NVP(xms);
        oa & BOOST_SERIALIZATION_NVP(readNoiseAdu);
        oa & BOOST_SERIALIZATION_NVP(q_sez_cam);
        oa & BOOST_SERIALIZATION_NVP(cam);
        oa & BOOST_SERIALIZATION_NVP(longitude);
        oa & BOOST_SERIALIZATION_NVP(latitude);
        oa & BOOST_SERIALIZATION_NVP(altitude);
        ofs.close();
    }

    // Now compute and write out some additional products that are not formally used by the calibration
    // but are useful for visualisation and debugging.

    // Create an RGB image of the extracted sources and save to file
    Imageui sourcesImage(signal->width, signal->height, 0x000000FF);
    RenderUtil::drawSources(sourcesImage.rawImage, sources, signal->width, signal->height, true);
    sprintf(filename, "%s/sources.ppm", processed.c_str());
    {
        std::ofstream out(filename);
        out << sourcesImage;
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

    for(unsigned int i=0; i<signal->width*signal->height; i++) {
        char buffer[200] = "";
        sprintf(buffer, "%f\t%f\n", signal->rawImage[i], noise->rawImage[i]);
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
