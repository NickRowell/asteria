#include "image.h"
#include "util/IOUtil.h"

#include <numeric>

Image::Image() {

}

Image::Image(const Image& copyme) : width(copyme.width), height(copyme.height), rawImage(copyme.rawImage), annotatedImage(copyme.annotatedImage), epochTimeUs(copyme.epochTimeUs) {

}

Image::Image(unsigned int &width, unsigned int &height) : width(width), height(height), rawImage(width * height), annotatedImage(width * height)
{

}

Image::Image(unsigned int &width, unsigned int &height, unsigned char val) : width(width), height(height), rawImage(width * height, val), annotatedImage(width * height, val)
{

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
    output << image.width << " " << image.height << " 255\n";

    // Write the epoch time of image capture
    output << "# epochTimeUs=" << std::to_string(image.epochTimeUs) << "\n";

    // TODO: write additional header info

    // Write data section
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
std::istream &operator>>(std::istream  &input, Image &image) {

    // Function to load an Image from file

    // Read image signature
    std::string line;
    if(input.good()) {
        getline (input, line);
        // Check magic number (first two chars in file)
        if(*(line.data()) != 'P' || *(line.data()+1) != '5') {
            std::cout << "Failed to read image as PGM, magic number wrong: " << line << std::endl;
            return input;
        }
    }
    else {
        std::cout << "Ran out of data for parsing image!" << std::endl;
        return input;
    }

    // Read image width, height and 255 (the maximum pixel value)
    if(input.good()) {
        getline (input, line);

        // Tokenize the string
        std::vector<std::string> x = split(line, ' ');

        // Check we found the right amount of elements:
        if(x.size() != 3) {
            std::cout << "Expected to read width, height and pixel limit, found " << x.size() << " numbers!\n";
            return input;
        }

        // Parse width & height
        try {
            image.width = std::stoi(x[0]);
        }
        catch(std::exception& e) {
            std::cout << "Couldn't parse width from " << x[0];
            return input;
        }
        try {
            image.height = std::stoi(x[1]);
        }
        catch(std::exception& e) {
            std::cout << "Couldn't parse height from " << x[1];
            return input;
        }
    }
    else {
        std::cout << "Ran out of data for parsing image!" << std::endl;
        return input;
    }

    // Read header: any lines starting '#' are a header line and we expect to read a key-value pair
    while(input.peek() == '#') {
        getline (input, line);

        // The line has the format '# key=value' or possibly '# key=value1,value2,value3' etc

        // Tokenize to remove the leading hash symbol(s)
        std::vector<std::string> x = split(line, ' ');
        x.erase(x.begin());
        // Concatenate the remaining line to rejoin any values separated by a space
        std::string keyValue = accumulate(x.begin(), x.end(), std::string(""));

        // Now split the string on '=' to separate into key/value
        std::vector<std::string> y = split(keyValue, '=');
        if(y.size() != 2) {
            std::cout << "Couldn't parse key-value pair from " << keyValue << "\n";
            return input;
        }
        std::string key = y[0];
        std::string val = y[1];

        if(!key.compare("epochTimeUs")) {
            try {
                image.epochTimeUs = std::stoll(val);
            }
            catch(std::exception& e) {
                std::cout << "Couldn't parse epochTimeUs from " << val;
                return input;
            }
        }
    }

    // TODO: read any additional header info

    // Read data section
    if(input.good()) {
        getline (input, line);

        // Check we have the right amount of data
        if(line.size() != image.width*image.height) {
            std::cout << "Found wrong amount of data: expected " << image.width*image.height <<
                         "pixels, found " << line.size();
            return input;
        }

        const char * ptr = line.data();
        for(unsigned int i=0; i<image.width*image.height; i++) {
            // Must first cast to an unsigned char, otherwise numbers in the upper
            // half of the 8-bit range are shifted to negative values, which then
            // get shifted to very large positive values if we cast to an unsigned
            // type afterwards.
            image.rawImage.push_back((unsigned char)*(ptr++));
        }
    }
    else {
        std::cout << "Ran out of data for parsing image!" << std::endl;
        return input;
    }

    return input;
}

