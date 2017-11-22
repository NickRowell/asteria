#include "sourcedetector.h"

#include <algorithm>
#include <set>

SourceDetector::SourceDetector() {

}

/**
 * Main workhorse algorithm for source detection. The samples in the image are sorted into
 * descending order. The sources are gradually formed by either assigning an isolated sample
 * to a new source, or assigning a non-isolated sample to an existing source. The significance
 * of each source is measured by reference to the noise image and the background level. Sources
 * falling below the given significance threshold are culled.
 *
 * @param signal
 *            Vector of all pixel values; this is the measured image from which sources are to be extracted (row-packed) [ADU]
 * @param background
 *            Vector of pixel background values (row-packed) [ADU]
 * @param noise
 *            Vector of pixel noise values, in terms of the standard deviation (row-packed) [ADU]
 * @param width
 *            Width of the image [pixels]
 * @param height
 *            Height of the image [pixels]
 * @param source_detection_threshold_sigmas
 *            Threshold for detection of significant sources, in terms of the number of standard deviations
 *            that the integrated flux lies above the background level [dimensionless].
 * @return Vector containing the Sources detected in the window
 */
std::vector<Source> SourceDetector::getSources(std::vector<double> &signal, std::vector<double> &background, std::vector<double> &noise,
                                               unsigned int &width, unsigned int &height, double &source_detection_threshold_sigmas) {

    // Create an array and List of Samples. The array is used to get a sample for a given coordinate, and
    // the list is used so that we can process the samples in intensity order
    std::vector<Sample<double> *> sortedSamples;
    std::vector<Sample<double> *> allSamples;

    for(unsigned int sIdx=0; sIdx<height * width; sIdx++) {
        Sample<double> * sample = new Sample<double>(sIdx, width, signal[sIdx]);
        allSamples.push_back(sample);
        sortedSamples.push_back(sample);
    }

    // Sort the vector into order of decreasing intensity
    std::sort(sortedSamples.begin(), sortedSamples.end(), Sample<double>::compareSamplePtrDecreasing);

    // Current source label; incremented each time a new source is found
    unsigned int currentLabel = 1;

    // Process samples in decreasing order of intensity
    for(unsigned int s=0; s<sortedSamples.size(); s++) {

        Sample<double> * sample = sortedSamples[s];

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
    for (Sample<double> * sample : allSamples) {
        // Is Sample labelled?
        if (sample->label != 0) {
            sources[sample->label - 1].pixels.push_back(sample->index);
        }
    }

    std::vector<Source> significantSources;

    // Post-process the sources to purge insignificant ones
    for (unsigned int s=0; s<sources.size(); s++) {

        Source source = sources[s];

        // Integrated background-subtracted signal
        source.adu = 0.0;
        // Uncertainty on that
        source.sigma_adu = 0.0;
        // Centre-of-flux
        source.i = 0.0;
        source.j = 0.0;

        for(unsigned int sIdx : source.pixels) {
            double adu = (double)signal[sIdx] - (double)background[sIdx];
            source.adu += adu;
            source.sigma_adu += noise[sIdx] * noise[sIdx];

            unsigned int x = sIdx % width;
            unsigned int y = sIdx / width;
            source.i += ((double)x) * adu;
            source.j += ((double)y) * adu;
        }
        source.sigma_adu = std::sqrt(source.sigma_adu);
        source.i /= source.adu;
        source.j /= source.adu;

        // Detection significance of the source
        double sigmas = source.adu / source.sigma_adu;
        if(sigmas > source_detection_threshold_sigmas) {
            significantSources.push_back(source);
        }
    }

    std::vector<Source> stellarSources;

    // Now measure the flux-weighted sample dispersion matrix for each source.
    for (unsigned int s=0; s<significantSources.size(); s++) {

        Source source = significantSources[s];

        // Compute the flux-weighted sample position dispersion matrix [pix],
        // as A =
        // [a b]
        // [b c]
        double a = 0.0;
        double b = 0.0;
        double c = 0.0;

        for(unsigned int sIdx : source.pixels) {

            unsigned int x = sIdx % width;
            unsigned int y = sIdx / width;

            double weight = ((double)signal[sIdx] - (double)background[sIdx]) / source.adu;

            a += (x - source.i) * (x - source.i) * weight;
            b += (x - source.i) * (y - source.j) * weight;
            c += (y - source.j) * (y - source.j) * weight;
        }

        source.c_ii = a;
        source.c_ij = b;
        source.c_jj = c;

        // Compute the eigenvalues: direct solution for 2x2 matrix
        double tr = a + c;
        double det = a * c - b * b;
        double disc = tr * tr / 4.0 - det;
        if (disc < 0.0) {
            // Eigenvalues are complex; note that the dispersion matrix is
            // real-symmetric so eigenvalues (should be) always real, however we explicitly
            // test this here in order to avoid unanticipated exceptions.
            continue;
        }
        disc = std::sqrt(disc);

        // The eigenvalues
        double l1 = tr / 2.0 + disc;
        double l2 = tr / 2.0 - disc;

        // Update the eigenvalues for the source
        source.l1 = l1;
        source.l2 = l2;

        // Both eigenvalues should be positive, as we're working with intensity maxima. Usually if
        // either is negative then the source is a perfect straight line.
        if (l1 < 0.0 || l2 < 0.0) {
            continue;
        }

        if (b == 0.0) {
            // Special case: principal axes align with X or Y directions. Likely that one eigenvalue
            // is zero.
            if (a > c) {
                source.orientation = M_PI / 2.0;
            } else {
                source.orientation = 0.0;
            }
        } else {
            // General case: principal axis does not align with either X or Y direction
            double lmax = std::max(l1, l2);

            // X and Y components of (normalised) eigenvector corresponding to largest eigenvalue.
            // We take the absolute value in order to reflect the eigenvector into the first
            // quadrant so as to ease the algebra for computing the angle between the vector and
            // the axes.
            double v_x = std::abs(1.0 / std::sqrt(b * b / ((a - lmax) * (a - lmax)) + 1.0));
            double v_y = std::abs(std::sqrt(1 - v_x));

            // Get the angle between the spike and the X direction
            source.orientation = std::atan(v_y / v_x);
        }

        stellarSources.push_back(source);
    }

    fprintf(stderr, "Found %lu sources\n", sources.size());
    fprintf(stderr, "Found %lu significant sources\n", significantSources.size());
    fprintf(stderr, "Found %lu stellar sources\n", stellarSources.size());

    return stellarSources;
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
std::vector<unsigned int> SourceDetector::getNeighbourUniqueLabels(Sample<double> *&sample, const std::vector<Sample<double> *> &samples, unsigned int &width, unsigned int &height) {

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
