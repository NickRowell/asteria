#include "infra/calibrationinventory.h"
#include "util/timeutil.h"

#include "util/renderutil.h"

#include <regex>
#include <fstream>
#include <functional>

CalibrationInventory::CalibrationInventory() {

}

CalibrationInventory::CalibrationInventory(const std::vector<std::shared_ptr<Imageuc> > &calibrationFrames) : calibrationFrames(calibrationFrames) {

}

CalibrationInventory *CalibrationInventory::loadFromDir(std::string path) {

    // Load all the images found here...
    DIR *dir;
    if ((dir = opendir (path.c_str())) == NULL) {
        // Couldn't open the directory!
        return NULL;
    }

    // Regex suitable for identifying images with filenames starting 'median'
    const std::regex medianRegex = std::regex("median");
    const std::regex backgroundRegex = std::regex("background");

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
        if(std::regex_search(child->d_name, TimeUtil::utcRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            std::ifstream input(childPath);
            auto seq = std::make_shared<Imageuc>();
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
            auto medianImage = std::make_shared<Imageuc>();
            input >> *medianImage;
            inv->medianImage = medianImage;
            input.close();
        }

        // Detect the background image
        if(std::regex_search(child->d_name, backgroundRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path + "/" + child->d_name;
            // Load the image from file
            std::ifstream input(childPath);
            auto backgroundImage = std::make_shared<Imageuc>();
            input >> *backgroundImage;
            inv->backgroundImage = backgroundImage;
            input.close();
        }

        // Detect the text file containing the calibration data
        if(strcmp(child->d_name, "calibration.dat") == 0) {
            fprintf(stderr, "Found %s\n", child->d_name);
        }
    }
    closedir (dir);

    // Sort the calibration image sequence into ascending order of capture time
    std::sort(inv->calibrationFrames.begin(), inv->calibrationFrames.end(), Imageuc::comparePtrToImage);

    return inv;
}

void CalibrationInventory::saveToDir(std::string path) {

    // Create an image of the extracted sources
    std::vector<unsigned int> sourcesImage(medianImage->width*medianImage->height, 0);

    for(unsigned int s=0; s<sources.size(); s++) {

        Source source = sources[s];

        // Get a random colour for this source
        unsigned char red = (unsigned char) rand();
        unsigned char green = (unsigned char) rand();
        unsigned char blue = (unsigned char) rand();

        unsigned int rgb;
        RenderUtil::encodeRgb(red, green, blue, rgb);

        // Loop over the pixels assigned to this source
        for(unsigned int p=0; p<source.pixels.size(); p++) {
            // Index of the pixel that's part of the current source
            unsigned int pixel = source.pixels[p];
            // Insert colour for this pixels
            sourcesImage[pixel] = rgb;
        }

        // Invert the colour
        unsigned int negColour = 0xFFFFFFFF;

        // Now draw a cross on the centroid and an ellipse to represent the dispersion matrix
        RenderUtil::drawEllipse(sourcesImage, medianImage->width, medianImage->height, source.x0, source.y0, source.c_xx, source.c_xy, source.c_yy, 5.0f, negColour);
    }

    //
    // TEMP: save sources image to file
    //
    char sourcesFilename [100];
    sprintf(sourcesFilename, "%s/sources.ppm", path.c_str());
    std::ofstream output(sourcesFilename);
    output << "P6\n";
    // Write the data section
    output << medianImage->width << " " << medianImage->height << " 255\n";
    // Write raster
    for(unsigned int pix : sourcesImage) {
        unsigned char r, g, b;
        RenderUtil::decodeRgb(r, g, b, pix);

        output << r;
        output << g;
        output << b;
    }
    output.close();
    //
    //
    //

    // Save calibration data to text file
    char calibrationDataFilename [100];
    sprintf(calibrationDataFilename, "%s/calibration.dat", path.c_str());
    std::ofstream out1(calibrationDataFilename);
    out1 << "# \n";
    out1 << "# read_noise=" << readNoiseAdu << "\n";
//    out1 << "# Camera.azimuth=" << state->azimuth << "\n";
//    out1 << "# Camera.elevation=" << state->elevation << "\n";
//    out1 << "# Camera.roll=" << state->roll << "\n";
    out1.close();

    char rnPlotfilename [100];
    sprintf(rnPlotfilename, "%s/readnoise.png", path.c_str());

    std::stringstream ss;

    // This script produces a histogram of the deviations:
//    ss << "set terminal pngcairo dashed enhanced color size 640,480 font \"Helvetica\"\n";
//    ss << "set style line 20 lc rgb \"#ddccdd\" lt 1 lw 1.5\n";
//    ss << "set style line 21 lc rgb \"#ddccdd\" lt 1 lw 0.5\n";
//    ss << "set style fill transparent solid 0.5 noborder\n";
//    ss << "set boxwidth 0.95 relative\n";
//    ss << "set xlabel \"Deviation from median [ADU]\" font \"Helvetica,14\"\n";
//    ss << "set xtics out nomirror offset 0.0,0.0 rotate by 0.0 scale 1.0\n";
//    ss << "set mxtics 2\n";
//    ss << "set xrange [-10:10]\n";
//    ss << "set format x \"%g\"\n";
//    ss << "set ylabel \"Frequency [-]\" font \"Helvetica,14\"\n";
//    ss << "set ytics out nomirror offset 0.0,0.0 rotate by 0.0 scale 1.0\n";
//    ss << "set mytics 2\n";
//    ss << "set yrange [*:*]\n";
//    ss << "set format y \"%g\"\n";
//    ss << "set key off\n";
//    ss << "set grid xtics mxtics ytics mytics back ls 20, ls 21\n";
//    ss << "set title \"Readnoise estimate\"\n";
//    ss << "set output \"" << rnPlotfilename << "\"\n";
//    ss << "plot \"-\" w boxes notitle\n";
//    for(int i=0; i<512; i++) {
//        ss << (i-256) << " " << histogramOfDeviations[i] << "\n";
//    }
//    ss << "e\n";

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
        char buffer[80] = "";
//        sprintf(buffer, "%d\t%d\n", medianVals[i], madVals[i]);


//        sprintf(buffer, "%f\t%f\n", meanVals[i], varianceVals[i]);
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

    char filename [100];
    std::string utcFrame = TimeUtil::epochToUtcString(epochTimeUs);

    // Write out the median image
    sprintf(filename, "%s/median_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << *medianImage;
        out.close();
    }

    // Write out the background image
    sprintf(filename, "%s/background_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << *backgroundImage;
        out.close();
    }

    // Write out the variance image
    sprintf(filename, "%s/noise_%s.pfm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << *noiseImage;
        out.close();
    }

    // Write out the raw calibration frames
    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {
        Imageuc &image = *calibrationFrames[i];
        char filename [100];
        std::string utcFrame = TimeUtil::epochToUtcString(image.epochTimeUs);
        sprintf(filename, "%s/%s.pgm", path.c_str(), utcFrame.c_str());
        std::ofstream out(filename);
        out << image;
        out.close();
    }

}
