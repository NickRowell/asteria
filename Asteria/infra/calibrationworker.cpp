#include "infra/calibrationworker.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "infra/source.h"
#include "util/sourcedetector.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>    // std::max

#include <Eigen/Dense>

#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>

CalibrationWorker::CalibrationWorker(QObject *parent, AsteriaState * state, std::vector<std::shared_ptr<Image>> calibrationFrames)
    : QObject(parent), state(state), calibrationFrames(calibrationFrames) {

}

CalibrationWorker::~CalibrationWorker() {
}

void CalibrationWorker::process() {

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //                 Perform calibration                   //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    fprintf(stderr, "Got %d frames for calibration\n", calibrationFrames.size());

    // Compute the median image; for each pixel, store a vector of all the values
    std::vector< std::vector<unsigned char>> pixels(state->width * state->height);

    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {

        Image &image = *calibrationFrames[i];

        // Accumulate pixel values
        for(unsigned int k=0; k<image.height; k++) {
            for(unsigned int l=0; l<image.width; l++) {
                unsigned int pixIdx = k*image.width + l;
                pixels[pixIdx].push_back(image.rawImage[pixIdx]);
            }
        }
    }

    // Compute the median, MAD and RMS value of each pixel
    std::vector<unsigned char> medianVals;
    std::vector<unsigned char> madVals;
    std::vector<double> meanVals;
    std::vector<double> varianceVals;
    double maxVariance = 0;
    int histogramOfDeviations[512] = {0};

    for(unsigned int k=0; k<state->height; k++) {
        for(unsigned int l=0; l<state->width; l++) {
            unsigned int pixIdx = k*state->width + l;

            vector<unsigned char> pixel = pixels[pixIdx];

            double mean = 0.0;
            double meanOfSquare = 0.0;
            for(unsigned int p = 0; p < pixel.size(); p++) {
                double dPix = (double)pixel[p];
                mean += dPix;
                meanOfSquare += (dPix * dPix);
            }
            mean /= (double)pixel.size();
            meanOfSquare /= (double)pixel.size();
            double variance = meanOfSquare - (mean*mean);
            meanVals.push_back(mean);
            varianceVals.push_back(variance);
            maxVariance = std::max(variance, maxVariance);

            unsigned char med;
            unsigned char mad;
            std::sort(pixel.begin(), pixel.end());

            // Compute the median value
            if(pixel.size() % 2 == 0) {
                // Even number of elements - take average of central two
                unsigned int a = (int)pixel[pixel.size()/2];
                unsigned int b = (int)pixel[pixel.size()/2 - 1];
                unsigned int c = (a + b)/2;
                unsigned char d = c & 0xFF;
                med = d;
            }
            else {
                // Odd number of elements - pick central one
                med = pixel[pixel.size()/2];
            }

            // Compute the MAD value
            std::vector<unsigned int> absDevs;
            for(unsigned int p = 0; p < pixel.size(); ++p) {
                int dev = (int)pixel[p] - med;
                histogramOfDeviations[dev+256]++;
                absDevs.push_back(abs(dev));
            }

            std::sort(absDevs.begin(), absDevs.end());

            if (pixel.size() % 2 == 0) {
                mad = (absDevs[pixel.size() / 2 - 1] + absDevs[pixel.size() / 2]) / 2;
            }
            else {
                mad = absDevs[pixel.size() / 2];
            }

            medianVals.push_back(med);
            madVals.push_back(mad);
        }
    }

    // Construct an image of the noise level in each pixel to detect fixed pattern noise etc
    std::vector<unsigned char> varImage;
    for(unsigned int i=0; i<state->width*state->height; i++) {
        double variance = varianceVals[i];
        // Normalise this to [0:1] range applying a stretch as appropriate
        double scaled = std::pow(variance, 0.25)/std::pow(maxVariance, 0.25);
        // Scale to 0:255 range and quantize
        unsigned char pixel = (unsigned char)floor(scaled * 255.0);
        varImage.push_back(pixel);
    }
    Image varianceImage(state->width, state->height);
    varianceImage.rawImage = varImage;

    Image median(state->width, state->height);
    median.rawImage = medianVals;

    // Extract sources from the median image
    fprintf(stderr, "Getting sources...\n");
    std::vector<Source> sources = SourceDetector::getSources(median.rawImage, state->width, state->height);
    fprintf(stderr, "Found %d sources\n", sources.size());

    // Create black sources image; we'll fill in each source with colour
    std::vector<unsigned char> sourcesImageBlank(state->width*state->height*3, 0);

    for(unsigned int s=0; s<sources.size(); s++) {
        Source source = sources[s];

        // Get a random colour for this source
        unsigned char red = (unsigned char) rand();
        unsigned char green = (unsigned char) rand();
        unsigned char blue = (unsigned char) rand();

        // Loop over the pixels assigned to this source
        for(unsigned int p=0; p<source.pixels.size(); p++) {
            // Index of the pixel that's part of the current source
            unsigned int pixel = source.pixels[p];
            // Insert colour for this pixels
            sourcesImageBlank[3*pixel + 0] = red;
            sourcesImageBlank[3*pixel + 1] = green;
            sourcesImageBlank[3*pixel + 2] = blue;
        }
    }

    // TODO: measure the background image for use in thresholded source detection

    // TODO: Measure xrange from percentiles of data
    // TODO: Get readnoise estimate from data
    double readNoiseAdu = 5.0;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                                       //
    //          Save calibration results to disk             //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::convertEpochToUtcString(calibrationFrames[0u]->epochTimeUs);
    std::string yyyy = TimeUtil::extractYearFromUtcString(utc);
    std::string mm = TimeUtil::extractMonthFromUtcString(utc);
    std::string dd = TimeUtil::extractDayFromUtcString(utc);

    std::vector<std::string> subLevels;
    subLevels.push_back(yyyy);
    subLevels.push_back(mm);
    subLevels.push_back(dd);
    subLevels.push_back(utc);
    string path = state->calibrationDirPath + "/" + yyyy + "/" + mm + "/" + dd + "/" + utc;

    if(!FileUtil::createDirs(state->calibrationDirPath, subLevels)) {
        fprintf(stderr, "Couldn't create directory %s\n", path.c_str());
        return;
    }

    //
    // TEMP: save sources image to file
    //
    char sourcesFilename [100];
    sprintf(sourcesFilename, "%s/sources.ppm", path.c_str());
    std::ofstream output(sourcesFilename);
    output << "P6\n";
    // Write the data section
    output << state->width << " " << state->height << " 255\n";
    // Write raster
    for(unsigned char pix : sourcesImageBlank) {
        output << pix;
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
    for(unsigned int i=0; i<state->width*state->height; i++) {
        char buffer[80] = "";
//        sprintf(buffer, "%d\t%d\n", medianVals[i], madVals[i]);
        sprintf(buffer, "%f\t%f\n", meanVals[i], varianceVals[i]);
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


    // Write out the median image
    char filename [100];
    string utcFrame = TimeUtil::convertEpochToUtcString(calibrationFrames[0]->epochTimeUs);
    sprintf(filename, "%s/median_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << median;
        out.close();
    }

    // Write out the variance image
    sprintf(filename, "%s/noise_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << varianceImage;
        out.close();
    }


    // Write out the raw calibration frames
    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {
        Image &image = *calibrationFrames[i];
        char filename [100];
        string utcFrame = TimeUtil::convertEpochToUtcString(image.epochTimeUs);
        sprintf(filename, "%s/%s.pgm", path.c_str(), utcFrame.c_str());
        std::ofstream out(filename);
        out << image;
        out.close();
    }

    // All done - emit signal
    emit finished(utc);
}
