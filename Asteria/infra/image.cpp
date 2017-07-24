#include "image.h"
#include "util/ioutil.h"
#include "util/v4l2util.h"

#include <numeric>

Image::Image() {

}

Image::Image(const Image& copyme) : width(copyme.width), height(copyme.height), rawImage(copyme.rawImage), annotatedImage(copyme.annotatedImage),
    epochTimeUs(copyme.epochTimeUs), field(copyme.field) {

}

Image::Image(unsigned int &width, unsigned int &height) : width(width), height(height), rawImage(width * height), annotatedImage(width * height) {

}

Image::Image(unsigned int &width, unsigned int &height, unsigned char val) : width(width), height(height), rawImage(width * height, val), annotatedImage(width * height, val) {

}

Image::~Image() {

}

bool Image::operator < (const Image& str) const {
    return (epochTimeUs < str.epochTimeUs);
}

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
            return input;
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
    }

    // TODO: read any additional header info

    // Read image width, height and 255 (the maximum pixel value)
    if(input.good()) {
        getline (input, line);

        // Tokenize the string
        std::vector<std::string> x = IoUtil::split(line, ' ');

        // Check we found the right amount of elements:
        if(x.size() != 3) {
            fprintf(stderr, "Expected to read width, height and pixel limit, found %d numbers!\n", x.size());
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
