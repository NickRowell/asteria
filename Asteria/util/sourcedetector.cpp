#include "sourcedetector.h"

#include <algorithm>
#include <set>

SourceDetector::SourceDetector() {

}

/**
 * Main workhorse algorithm for source detection. The samples in the image are sorted into
 * descending order. The sources are gradually formed by either assigning an isolated sample
 * to a new source, or assigning a non-isolated sample to an existing source.
 *
 * TODO:
 *  - Include background estimation at each pixel to enable noise suppression
 *  - Investigate the best source detection algorithm;
 *     - Peak-finding (i.e. sources defined by local maxima)
 *     - Thresholding (i.e. sources defined by regions within a contour)
 *
 * @param pixels
 *            Vector of all pixel values (row-packed) [ADU]
 * @param width
 *            Width of the image [pixels]
 * @param height
 *            Height of the image [pixels]
 * @return Vector containing the Sources detected in the window
 */
std::vector<Source> SourceDetector::getSources(std::vector<unsigned char> pixels, unsigned int width, unsigned int height) {

    // Samples above this level will be considered source
    unsigned char detectionThreshold = 150;

    // Create an array and List of Samples. The array is used to get a sample for a given coordinate, and
    // the list is used so that we can process the samples in intensity order
    std::vector<Sample *> sortedSamples;
    std::vector<Sample *> allSamples;

    for(unsigned int sIdx=0; sIdx<height * width; sIdx++) {
        Sample * sample = new Sample(sIdx, width, pixels[sIdx]);
        allSamples.push_back(sample);
        // TODO: dynamic detection threshold based on background image
        if(sample->level > detectionThreshold) {
            sortedSamples.push_back(sample);
        }
    }

    // Sort the vector into order of decreasing intensity
    std::sort(sortedSamples.begin(), sortedSamples.end(), Sample::compareSamplePtrDecreasing);

    // Current source label; incremented each time a new source is found
    unsigned int currentLabel = 1;

    // Process samples in decreasing order of intensity
    for(unsigned int s=0; s<sortedSamples.size(); s++) {

        Sample * sample = sortedSamples[s];

        // Is this sample
        // a) Isolated? If so, initialise a new source
        // b) Connected to an existing source? If so, give it the same label
        // c) Connected to more than one existing source? If so, leave it unlabelled
        std::vector<unsigned int> neighbourLabels = getNeighbourUniqueLabels(sample, allSamples, width, height);

        if(neighbourLabels.size()==0) {
            // Isolated sample - initialise new source
            sample->label = currentLabel;
            currentLabel++;
        }
        else if(neighbourLabels.size()==1) {
            // Neighbouring one source - connect the sample to it
            sample->label = neighbourLabels.back();
        }
        else {
            // Multiple labels! This is a faint sample sandwiched between two unconnected
            // brighter samples - leave it unlabelled.
        }
    }

    // Number of regions detected is indicated in the value of currentLabel.
    // Now extract the samples for these regions into array
    std::vector<Source> sources(currentLabel-1);

    // Assign each uniquely-labelled sample to the right source
    for (Sample * sample : allSamples) {
        // Is Sample labelled?
        if (sample->label != 0) {
            sources[sample->label - 1].pixels.push_back(sample->index);
        }
    }
    return sources;
}

/**
 * @brief SourceDetector::getNeighbourUniqueLabels
 * Examines the labels assigned to the samples neighbouring the current one, and compiles the set of
 * unique labels (ignoring unlabelled, i.e. label=0, samples).
 * @param sample
 *  The central sample
 * @param samples
 *  A vector containing all the samples in the image
 * @param width
 *  The image width [pixels]
 * @param height
 *  The image height [pixels]
 * @return
 *  The set of unique labels found among the neighbouring samples; returned as a vector for ease of access.
 */
std::vector<unsigned int> SourceDetector::getNeighbourUniqueLabels(Sample * &sample, const std::vector<Sample *> &samples, unsigned int &width, unsigned int &height) {

    // Set of labels for the neighbouring samples
    std::set<unsigned int> neighbourLabels;

    // Loop over eight neighbouring pixels
    for(int di = -1; di < 2; di++) {
        for(int dj = -1; dj < 2; dj++) {
            if(di==0 && dj==0) {
                // Don't compare the sample with itself!
                continue;
            }
            // Coordinate of this neighbour
            int i = sample->i + di;
            int j = sample->j + dj;

            // Is this neighbour outside of the image?
            if (i < 0 || i >= width || j < 0 || j >= height) {
                continue;
            }

            unsigned int sIdx = j * width + i;

            if(samples[sIdx]->label != 0) {
                neighbourLabels.insert(samples[sIdx]->label);
            }
        }
    }

    // Read the set contents to a vector for ease of access later
    std::vector<unsigned int> neighbourUniqueLabels(neighbourLabels.begin(), neighbourLabels.end());

    return neighbourUniqueLabels;
}
