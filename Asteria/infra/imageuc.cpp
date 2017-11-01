#include "imageuc.h"
#include "util/ioutil.h"
#include "util/v4l2util.h"
#include "util/renderutil.h"

#include <numeric>

Imageuc::Imageuc() : Image<unsigned char>() {
}

Imageuc::Imageuc(const Imageuc& copyme) : Image<unsigned char>(copyme), annotatedImage(copyme.annotatedImage),
    field(copyme.field) {
}

Imageuc::Imageuc(unsigned int &width, unsigned int &height) : Image<unsigned char>(width, height), annotatedImage(width * height) {
}

Imageuc::Imageuc(unsigned int &width, unsigned int &height, unsigned char val) : Image<unsigned char>(width, height, val), annotatedImage(width * height, val) {
}

Imageuc::~Imageuc() {
}

void Imageuc::writeToStream(std::ostream &output) const {

    // Function to write an Image to file

    // Raw PGMs:
    output << "P5\n";

    // Write the epoch time of image capture
    output << "# epochTimeUs=" << std::to_string(epochTimeUs) << "\n";
    // Write scan mode of the image
    output << "# v4l2_field_index=" << std::to_string(field) << "\n";
    // Human-readable version (not deserialised; this is for manual inspection of files only)
    output << "# v4l2_field_name=" << V4L2Util::getV4l2FieldNameFromIndex(field) << "\n";

    // TODO: write additional header info

    // Write the data section
    output << width << " " << height << " 255\n";

    // Write raster
    for(unsigned int k=0; k<height; k++) {
        for(unsigned int l=0; l<width; l++) {
            unsigned int offset = k*width + l;
            unsigned char pix = rawImage[offset];
            output << pix;
        }
    }

     return;
}

void Imageuc::readFromStream(std::istream &input) {

    // Function to load an Image from file

    // Read image signature
    std::string line;
    if(input.good()) {
        getline (input, line);
        // Check magic number (first two chars in file)
        if(*(line.data()) != 'P' || *(line.data()+1) != '5') {
            fprintf(stderr, "Failed to read image as PGM, magic number wrong: %s\n", line.c_str());
            return;
        }
    }
    else {
        fprintf(stderr, "Ran out of data for parsing image!\n");
        return;
    }

    // Read header: any lines starting '#' are a header line and we expect to read a key-value pair
    while(input.peek() == '#') {
        getline (input, line);

        // The line has the format '# key=value' or possibly '# key=value1,value2,value3' etc

        // Tokenize to remove the leading hash symbol(s)
        std::vector<std::string> x = IoUtil::split(line, ' ');
        x.erase(x.begin());
        // Concatenate the remaining line to rejoin any values separated by a space
        std::string keyValue = accumulate(x.begin(), x.end(), std::string(""));

        // Now split the string on '=' to separate into key/value
        std::vector<std::string> y = IoUtil::split(keyValue, '=');
        if(y.size() != 2) {
            fprintf(stderr, "Couldn't parse key-value pair from %s\n", keyValue.c_str());
            continue;
        }
        std::string key = y[0];
        std::string val = y[1];

        if(!key.compare("epochTimeUs")) {
            try {
                epochTimeUs = std::stoll(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse epochTimeUs from %s\n", val.c_str());
                return;
            }
        }
        if(!key.compare("v4l2_field_index")) {
            try {
                field = std::stoul(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse v4l2_field_index from %s\n", val.c_str());
                return;
            }
        }
    }

    // TODO: read any additional header info

    // Read image width, height and 255 (the maximum pixel value)
    if(input.good()) {
        getline (input, line);

        // Tokenize the string
        std::vector<std::string> x = IoUtil::split(line, ' ');

        // Check we found the right amount of elements:
        if(x.size() != 3) {
            fprintf(stderr, "Expected to read width, height and pixel limit, found %lu numbers!\n", x.size());
            return;
        }

        // Parse width & height
        try {
            width = std::stoi(x[0]);
        }
        catch(std::exception& e) {
            fprintf(stderr, "Couldn't parse width from %s\n", x[0].c_str());
            return;
        }
        try {
            height = std::stoi(x[1]);
        }
        catch(std::exception& e) {
            fprintf(stderr, "Couldn't parse height from %s\n", x[1].c_str());
            return;
        }
    }
    else {
        fprintf(stderr, "Ran out of data for parsing image!\n");
        return;
    }

    // Read data section. Don't do getline because zeros are interpreted as newline characters.
    for(unsigned int i=0; i<width*height; i++) {

        // Check that input is good
        if(!input.good()) {
            // Ran out of data early
            fprintf(stderr, "Found wrong amount of data: expected %d pixels, found %d\n", width*height, i);
            return;
        }

        unsigned char pix = (unsigned char)input.get();
        rawImage.push_back(pix);
    }

    return;
}

void Imageuc::generateAnnotatedImage(const MeteorImageLocationMeasurement &loc) {

    annotatedImage.clear();
    annotatedImage.reserve(width * height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < width * height; p++) {
        annotatedImage.push_back(0x00000000);
    }

    // Indicate changed pixels
    for(auto const& p: loc.changedPixelsPositive) {
        // Positive changed pixels - blue
        annotatedImage[p] = 0x0000FFFF;
    }
    for(auto const& p: loc.changedPixelsNegative) {
        // Negative changed pixels - green
        annotatedImage[p] = 0x00FF00FF;
    }

    // Add features
    if(loc.coarse_localisation_success) {
        for(unsigned int x = loc.bb_xmin; x<=loc.bb_xmax; x++) {
            annotatedImage[loc.bb_ymin*width + x] = 0xFF0000FF;
            annotatedImage[loc.bb_ymax*width + x] = 0xFF0000FF;
        }
        for(unsigned int y = loc.bb_ymin; y<=loc.bb_ymax; y++) {
            annotatedImage[y*width + loc.bb_xmin] = 0xFF0000FF;
            annotatedImage[y*width + loc.bb_xmax] = 0xFF0000FF;
        }
    }
}

void Imageuc::generatePeakholdAnnotatedImage(std::vector<std::shared_ptr<Imageuc>> &eventFrames, const std::vector<MeteorImageLocationMeasurement> &locs) {

    annotatedImage.clear();
    annotatedImage.reserve(width * height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < width * height; p++) {
        annotatedImage.push_back(0x00000000);
    }

    // Loop over the event images, which are in time sequence
    for(unsigned int i=1; i<eventFrames.size(); i++) {
        if(locs[i].coarse_localisation_success && locs[i-1].coarse_localisation_success) {
            // Draw line connecting the centroids between the two frames
            int x0 = (int) std::round(locs[i-1].x_flux_centroid);
            int y0 = (int) std::round(locs[i-1].y_flux_centroid);
            int x1 = (int) std::round(locs[i].x_flux_centroid);
            int y1 = (int) std::round(locs[i].y_flux_centroid);

            RenderUtil::drawLine(annotatedImage, width, height, x0, x1, y0, y1, 0xFF00FFFF);
        }
    }
}
