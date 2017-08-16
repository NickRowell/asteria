#include "image.h"
#include "util/ioutil.h"
#include "util/v4l2util.h"
#include "util/renderutil.h"

#include <numeric>

Image::Image() {
    coarse_localisation_success = false;
}

Image::Image(const Image& copyme) : width(copyme.width), height(copyme.height), rawImage(copyme.rawImage), annotatedImage(copyme.annotatedImage),
    epochTimeUs(copyme.epochTimeUs), field(copyme.field), changedPixelsPositive(copyme.changedPixelsPositive), changedPixelsNegative(copyme.changedPixelsNegative) {
    coarse_localisation_success = false;
}

Image::Image(unsigned int &width, unsigned int &height) : width(width), height(height), rawImage(width * height), annotatedImage(width * height) {
    coarse_localisation_success = false;
}

Image::Image(unsigned int &width, unsigned int &height, unsigned char val) : width(width), height(height), rawImage(width * height, val), annotatedImage(width * height, val) {
    coarse_localisation_success = false;
}

Image::~Image() {
    coarse_localisation_success = false;
}

bool Image::operator < (const Image& str) const {
    return (epochTimeUs < str.epochTimeUs);
}

bool Image::comparePtrToImage(std::shared_ptr<Image> a, std::shared_ptr<Image> b) {
    return (*a < *b);
}

/**
 * Serialises the Image to a ostream. The philosophy is that any valid fields are written, except for transient ones
 * such as the overlay image which are generated on-the-fly at runtime.
 *
 * @brief operator <<
 * @param output
 * @param image
 * @return
 */
std::ostream &operator<<(std::ostream &output, const Image &image) {

    // Function to write an Image to file

    // Raw PGMs:
    output << "P5\n";

    // Write the epoch time of image capture
    output << "# epochTimeUs=" << std::to_string(image.epochTimeUs) << "\n";
    // Write scan mode of the image
    output << "# v4l2_field_index=" << std::to_string(image.field) << "\n";
    // Human-readable version (not deserialised; this is for manual inspection of files only)
    output << "# v4l2_field_name=" << V4L2Util::getV4l2FieldNameFromIndex(image.field) << "\n";

    // Analysis results
    if(!image.changedPixelsPositive.empty()) {
        output << "# changed_pixels_positive=";
        for(unsigned int p = 0; p < image.changedPixelsPositive.size(); ++p) {
            unsigned int pIdx = image.changedPixelsPositive[p];
            output << pIdx << ",";
        }
        output << "\n";
    }
    if(!image.changedPixelsNegative.empty()) {
        output << "# changed_pixels_negative=";
        for(unsigned int p = 0; p < image.changedPixelsNegative.size(); ++p) {
            unsigned int pIdx = image.changedPixelsNegative[p];
            output << pIdx << ",";
        }
        output << "\n";
    }

    output << "# coarse_localisation_success=" << image.coarse_localisation_success << "\n";
    if(image.coarse_localisation_success) {
        output << "# bb_xmin=" << image.bb_xmin << "\n";
        output << "# bb_xmax=" << image.bb_xmax << "\n";
        output << "# bb_ymin=" << image.bb_ymin << "\n";
        output << "# bb_ymax=" << image.bb_ymax << "\n";
        output << "# centre_of_flux=" << image.x_flux_centroid << "," << image.y_flux_centroid << "\n";
    }

    // TODO: write additional header info

    // Write the data section
    output << image.width << " " << image.height << " 255\n";

    // Write raster
    for(unsigned int k=0; k<image.height; k++) {
        for(unsigned int l=0; l<image.width; l++) {
            unsigned int offset = k*image.width + l;
            unsigned char pix = image.rawImage[offset];
            output << pix;
        }
    }

     return output;
}

/**
 * Function to read an Image from in input stream. This makes only cursory checks on the
 * validity of the image as it's expected that we'll only ever read Images written by this
 * software so there's very low risk of corruption.
 * @brief operator >>
 * @param input
 * @param D
 * @return
 */
std::istream &operator>>(std::istream &input, Image &image) {

    // Function to load an Image from file

    // Read image signature
    std::string line;
    if(input.good()) {
        getline (input, line);
        // Check magic number (first two chars in file)
        if(*(line.data()) != 'P' || *(line.data()+1) != '5') {
            fprintf(stderr, "Failed to read image as PGM, magic number wrong: %s\n", line.c_str());
            return input;
        }
    }
    else {
        fprintf(stderr, "Ran out of data for parsing image!\n");
        return input;
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
                image.epochTimeUs = std::stoll(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse epochTimeUs from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("v4l2_field_index")) {
            try {
                image.field = std::stoul(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse v4l2_field_index from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("changed_pixels_positive")) {
            std::vector<std::string> x = IoUtil::split(val, ',');
            for(unsigned int p = 0; p < x.size(); ++p) {
                unsigned int pIdx = std::stoul(x[p]);
                image.changedPixelsPositive.push_back(pIdx);
            }
        }
        if(!key.compare("changed_pixels_negative")) {
            std::vector<std::string> x = IoUtil::split(val, ',');
            for(unsigned int p = 0; p < x.size(); ++p) {
                unsigned int pIdx = std::stoul(x[p]);
                image.changedPixelsNegative.push_back(pIdx);
            }
        }
        if(!key.compare("coarse_localisation_success")) {
            try {
                image.coarse_localisation_success = std::stoll(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse coarse_localisation_success from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("bb_xmin")) {
            try {
                image.bb_xmin = std::stoul(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse bb_xmin from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("bb_xmax")) {
            try {
                image.bb_xmax = std::stoul(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse bb_xmax from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("bb_ymin")) {
            try {
                image.bb_ymin = std::stoul(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse bb_ymin from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("bb_ymax")) {
            try {
                image.bb_ymax = std::stoul(val);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse bb_ymax from %s\n", val.c_str());
                return input;
            }
        }
        if(!key.compare("centre_of_flux")) {
            std::vector<std::string> x = IoUtil::split(val, ',');
            if(x.size() != 2) {
                fprintf(stderr, "Expected to read centre of flux X and Y coordinates, found %lu numbers!\n", x.size());
                return input;
            }
            try {
                image.x_flux_centroid = std::stod(x[0]);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse x_flux_centroid from %s\n", x[0].c_str());
                return input;
            }
            try {
                image.y_flux_centroid = std::stod(x[1]);
            }
            catch(std::exception& e) {
                fprintf(stderr, "Couldn't parse y_flux_centroid from %s\n", x[1].c_str());
                return input;
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
            return input;
        }

        // Parse width & height
        try {
            image.width = std::stoi(x[0]);
        }
        catch(std::exception& e) {
            fprintf(stderr, "Couldn't parse width from %s\n", x[0].c_str());
            return input;
        }
        try {
            image.height = std::stoi(x[1]);
        }
        catch(std::exception& e) {
            fprintf(stderr, "Couldn't parse height from %s\n", x[1].c_str());
            return input;
        }
    }
    else {
        fprintf(stderr, "Ran out of data for parsing image!\n");
        return input;
    }

    // Read data section. Don't do getline because zeros are interpreted as newline characters.
    for(unsigned int i=0; i<image.width*image.height; i++) {

        // Check that input is good
        if(!input.good()) {
            // Ran out of data early
            fprintf(stderr, "Found wrong amount of data: expected %d pixels, found %d\n", image.width*image.height, i);
            return input;
        }

        unsigned char pix = (unsigned char)input.get();
        image.rawImage.push_back(pix);
    }

    return input;
}


void Image::generateAnnotatedImage() {

    annotatedImage.clear();
    annotatedImage.reserve(width * height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < width * height; p++) {
        annotatedImage.push_back(0x00000000);
    }

    // Indicate changed pixels
    for(auto const& p: changedPixelsPositive) {
        // Positive changed pixels - blue
        annotatedImage[p] = 0x0000FFFF;
    }
    for(auto const& p: changedPixelsNegative) {
        // Negative changed pixels - green
        annotatedImage[p] = 0x00FF00FF;
    }

    // Add features
    if(this->coarse_localisation_success) {
        for(unsigned int x = bb_xmin; x<=bb_xmax; x++) {
            annotatedImage[bb_ymin*width + x] = 0xFF0000FF;
            annotatedImage[bb_ymax*width + x] = 0xFF0000FF;
        }
        for(unsigned int y = bb_ymin; y<=bb_ymax; y++) {
            annotatedImage[y*width + bb_xmin] = 0xFF0000FF;
            annotatedImage[y*width + bb_xmax] = 0xFF0000FF;
        }
    }

    // Example usage of rendering functions
//    RenderUtil::drawLine(annotatedImage, width, height, 256u, 350u, 100u, 300u, 0x00FFFFFF);
//    RenderUtil::drawCircle(annotatedImage, width, height, 328.0, 145.6, 53.2, 0x0000FFFF);
//    RenderUtil::drawEllipse(annotatedImage, width, height, 328.0, 145.6, 10.0, 1.5, 3.0, 15.0, 0xFF00FFFF);


}

void Image::generatePeakholdAnnotatedImage(std::vector<std::shared_ptr<Image>> &eventFrames) {

    annotatedImage.clear();
    annotatedImage.reserve(width * height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < width * height; p++) {
        annotatedImage.push_back(0x00000000);
    }

    // Loop over the event images, which are in time sequence
    for(unsigned int i=1; i<eventFrames.size(); i++) {
        if(eventFrames[i]->coarse_localisation_success && eventFrames[i-1]->coarse_localisation_success) {
            // Draw line connecting the centroids between the two frames
            int x0 = (int) std::round(eventFrames[i-1]->x_flux_centroid);
            int y0 = (int) std::round(eventFrames[i-1]->y_flux_centroid);
            int x1 = (int) std::round(eventFrames[i]->x_flux_centroid);
            int y1 = (int) std::round(eventFrames[i]->y_flux_centroid);

            RenderUtil::drawLine(annotatedImage, width, height, x0, x1, y0, y1, 0xFF00FFFF);
        }
    }

    // Example usage of rendering functions
//    RenderUtil::drawLine(annotatedImage, width, height, 256u, 350u, 100u, 300u, 0x00FFFFFF);
//    RenderUtil::drawCircle(annotatedImage, width, height, 328.0, 145.6, 53.2, 0x0000FFFF);
//    RenderUtil::drawEllipse(annotatedImage, width, height, 328.0, 145.6, 10.0, 1.5, 3.0, 15.0, 0xFF00FFFF);


}
