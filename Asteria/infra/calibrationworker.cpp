#include "infra/calibrationworker.h"
#include "util/timeutil.h"
#include "util/fileutil.h"
#include "infra/source.h"
#include "util/sourcedetector.h"
#include "util/renderutil.h"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <algorithm>

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

    long long midTimeStamp = (calibrationFrames.front()->epochTimeUs + calibrationFrames.back()->epochTimeUs) >> 1;
    unsigned int field = calibrationFrames.front()->field;

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
    std::vector<unsigned char> stdVals;
    for(unsigned int i=0; i<state->width*state->height; i++) {
        double std = std::sqrt(varianceVals[i]);
        unsigned char pixel = (unsigned char)floor(std);
        stdVals.push_back(pixel);
    }
    Image noise(state->width, state->height);
    noise.field = field;
    noise.epochTimeUs = midTimeStamp;
    noise.rawImage = stdVals;

    Image median(state->width, state->height);
    median.field = field;
    median.epochTimeUs = midTimeStamp;
    median.rawImage = medianVals;

    // Measure the background image from the median
    Image background(state->width, state->height);
    // Algorithm for background calculation: each pixel is the median value of the pixels surrounding it in
    // a window of some particular width.
    // Sliding window extends out to this many pixels on each side of the central pixel
    int hw = (int)state->bkg_median_filter_half_width;
    for(int k=0; k<state->height; k++) {
        for(int l=0; l<state->width; l++) {

            // Compute the boundary of the window region
            int k_min = std::max(k - hw, 0);
            int k_max = std::min(k + hw, (int)state->height);
            int l_min = std::max(l - hw, 0);
            int l_max = std::min(l + hw, (int)state->width);

            // Pixels within the window
            std::vector<unsigned char> pixels;
            for(unsigned int kp=k_min; kp<k_max; kp++) {
                for(unsigned int lp=l_min; lp<l_max; lp++) {
                    unsigned int pixIdx = kp*state->width + lp;
                    pixels.push_back(medianVals[pixIdx]);
                }
            }

            // Get the median value in the window
            std::sort(pixels.begin(), pixels.end());
            unsigned char med;
            if(pixels.size() % 2 == 0) {
                // Even number of elements - take average of central two
                unsigned int a = (int)pixels[pixels.size()/2];
                unsigned int b = (int)pixels[pixels.size()/2 - 1];
                unsigned int c = (a + b)/2;
                unsigned char d = c & 0xFF;
                med = d;
            }
            else {
                // Odd number of elements - pick central one
                med = pixels[pixels.size()/2];
            }
            unsigned int pixIdx = k*state->width + l;
            background.rawImage[pixIdx] = med;
        }
    }

    // Extract sources from the median image
    std::vector<Source> sources = SourceDetector::getSources(median.rawImage, background.rawImage, noise.rawImage,
                                                             state->width, state->height, state->source_detection_threshold_sigmas);

    // Create an image of the extracted sources
    std::vector<unsigned int> sourcesImage(state->width*state->height, 0);

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
        RenderUtil::drawEllipse(sourcesImage, state->width, state->height, source.x0, source.y0, source.c_xx, source.c_xy, source.c_yy, 5.0f, negColour);
    }

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
    std::string utc = TimeUtil::epochToUtcString(calibrationFrames[0u]->epochTimeUs);
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

    char filename [100];
    string utcFrame = TimeUtil::epochToUtcString(calibrationFrames[0]->epochTimeUs);

    // Write out the median image
    sprintf(filename, "%s/median_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << median;
        out.close();
    }

    // Write out the background image
    sprintf(filename, "%s/background_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << background;
        out.close();
    }

    // Write out the variance image
    sprintf(filename, "%s/noise_%s.pgm", path.c_str(), utcFrame.c_str());
    {
        std::ofstream out(filename);
        out << noise;
        out.close();
    }

    // Write out the raw calibration frames
    for(unsigned int i = 0; i < calibrationFrames.size(); ++i) {
        Image &image = *calibrationFrames[i];
        char filename [100];
        string utcFrame = TimeUtil::epochToUtcString(image.epochTimeUs);
        sprintf(filename, "%s/%s.pgm", path.c_str(), utcFrame.c_str());
        std::ofstream out(filename);
        out << image;
        out.close();
    }

    // All done - emit signal
    emit finished(utc);
}
