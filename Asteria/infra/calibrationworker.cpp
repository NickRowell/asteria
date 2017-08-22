#include "infra/calibrationworker.h"
#include "util/timeutil.h"
#include "util/fileutil.h"

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
    //               Perform image analysis                  //
    //                                                       //
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    fprintf(stderr, "Got %d frames for calibration\n", calibrationFrames.size());

    // Create new directory to store results for this clip. The path is set by the
    // date and time of the first frame
    std::string utc = TimeUtil::convertToUtcString(calibrationFrames[0u]->epochTimeUs);
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

    // Compute the median and MAD value of each pixel
    std::vector<unsigned char> medianVals;
    std::vector<unsigned char> madVals;
    int histogramOfDeviations[512] = {0};

    for(unsigned int k=0; k<state->height; k++) {
        for(unsigned int l=0; l<state->width; l++) {
            unsigned int pixIdx = k*state->width + l;

            vector<unsigned char> pixel = pixels[pixIdx];
            std::sort(pixel.begin(), pixel.end());

            unsigned char med;
            unsigned char mad;

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

            if (pixel.size()  % 2 == 0) {
                mad = (absDevs[pixel.size() / 2 - 1] + absDevs[pixel.size() / 2]) / 2;
            }
            else {
                mad = absDevs[pixel.size() / 2];
            }

            medianVals.push_back(med);
            madVals.push_back(mad);
        }
    }


    // Prototype linear algebra library usage
    Eigen::MatrixXd m(2,2);
    m(0,0) = 3;
    m(1,0) = 2.5;
    m(0,1) = -1;
    m(1,1) = m(1,0) + m(0,1);
    std::cout << m << std::endl;

    // TODO: Add gnuplot installation to the build setup file
    // TODO: Measure xrange from percentiles of data
    // TODO: Get readnoise estimate from data
    // TODO: Terminate calibration recording if event detected

    char rnPlotfilename [100];
    sprintf(rnPlotfilename, "%s/readnoise.png", path.c_str());

    std::stringstream ss;

    ss << "set terminal pngcairo dashed enhanced color size 640,480 font \"Helvetica\"\n";
    ss << "set style line 20 lc rgb \"#ddccdd\" lt 1 lw 1.5\n";
    ss << "set style line 21 lc rgb \"#ddccdd\" lt 1 lw 0.5\n";
    ss << "set style fill transparent solid 0.5 noborder\n";
    ss << "set boxwidth 0.95 relative\n";
    ss << "set xlabel \"Deviation from median [ADU]\" font \"Helvetica,14\"\n";
    ss << "set xtics out nomirror offset 0.0,0.0 rotate by 0.0 scale 1.0\n";
    ss << "set mxtics 2\n";
    ss << "set xrange [-10:10]\n";
    ss << "set format x \"%g\"\n";
    ss << "set ylabel \"Frequency [-]\" font \"Helvetica,14\"\n";
    ss << "set ytics out nomirror offset 0.0,0.0 rotate by 0.0 scale 1.0\n";
    ss << "set mytics 2\n";
    ss << "set yrange [*:*]\n";
    ss << "set format y \"%g\"\n";
    ss << "set key off\n";
    ss << "set grid xtics mxtics ytics mytics back ls 20, ls 21\n";
    ss << "set title \"Readnoise estimate\"\n";
    ss << "set output \"" << rnPlotfilename << "\"\n";
    ss << "plot \"-\" w boxes notitle\n";
    for(int i=0; i<512; i++) {
        ss << (i-256) << " " << histogramOfDeviations[i] << "\n";
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








    Image median(state->width, state->height);
    median.rawImage = medianVals;

    // Write out the median image
    char filename [100];
    string utcFrame = TimeUtil::convertToUtcString(calibrationFrames[0]->epochTimeUs);
    sprintf(filename, "%s/median_%s.pgm", path.c_str(), utcFrame.c_str());

    // PGM (grey image)
    std::ofstream out(filename);
    out << median;
    out.close();

    // All done - emit signal
    emit finished(utc);
}

