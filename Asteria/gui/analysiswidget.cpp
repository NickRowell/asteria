#include "gui/analysiswidget.h"
#include "gui/glmeteordrawer.h"
#include "util/timeutil.h"

#include <dirent.h>
#include <regex>
#include <fstream>
#include <iostream>
#include <memory>

#include <QHBoxLayout>


AnalysisWidget::AnalysisWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state) {

    replay = new GLMeteorDrawer(this, this->state, false);


    // Arrange layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(replay);
    this->setLayout(mainLayout);

}



void AnalysisWidget::loadClip(QString path) {

    // Regex suitable for identifying images with filenames e.g. 2017-06-14T19:41:09.282Z
    const std::regex utcRegex = TimeUtil::getUtcRegex();
    // Regex suitable for identifying images with filenames starting 'peakhold'
    const std::regex peakHoldRegex = std::regex("peakhold");

    // Load all the images found here...
    DIR *dir;
    if ((dir = opendir (path.toStdString().c_str())) == NULL) {
        // Couldn't open the directory!
        return;
    }

    Image peakHold;
    std::vector<Image> sequence;

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
            std::string childPath = path.toStdString() + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            Image seq;
            std::ifstream input(childPath);
            input >> seq;
            sequence.push_back(seq);
            input.close();
        }

        // Detect the peak hold image
        if(std::regex_search(child->d_name, peakHoldRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path.toStdString() + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the peakHold variable
            std::ifstream input(childPath);
            input >> peakHold;
            input.close();
        }
    }
    closedir (dir);

    // Sort the image sequence into ascending order of capture time
    std::sort(sequence.begin(), sequence.end());

    std::shared_ptr<Image> peakHoldPtr = std::make_shared<Image>(peakHold);

    // Initialise it with the peak hold image
    replay->newFrame(peakHoldPtr);
}
