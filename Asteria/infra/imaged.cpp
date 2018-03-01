#include "imaged.h"
#include "util/ioutil.h"
#include "util/v4l2util.h"
#include "util/renderutil.h"

#include <numeric>

Imaged::Imaged() : Image<double>() {
}

Imaged::Imaged(const Imaged& copyme) : Image<double>(copyme) {
}

Imaged::Imaged(unsigned int &width, unsigned int &height) : Image<double>(width, height) {
}

Imaged::Imaged(unsigned int &width, unsigned int &height, double val) : Image<double>(width, height, val) {
}

Imaged::~Imaged() {
}

void Imaged::writeToStream(std::ostream &output) const {

    // Function to write an Image to file

    // Floating point format:
    output << "PF\n";

    // Write the epoch time of image capture
    output << "# epochTimeUs=" << std::to_string(epochTimeUs) << "\n";

    // TODO: write additional header info

    // Write the data section
    output << width << " " << height << "\n";

    // Scale factor / endianness
    output << "1.0\n";

    // Write raster:

    // Pointer to the start of the vector of pixels
    const char* pointer = reinterpret_cast<const char*>(&rawImage[0]);
    size_t bytes = rawImage.size() * sizeof(rawImage[0]);
    output.write(pointer, bytes);

    return;
}

void Imaged::readFromStream(std::istream &input) {

    // Function to load an Image from file

    // Read image signature
    std::string line;
    if(input.good()) {
        getline (input, line);
        // Check magic number (first two chars in file)
        if(*(line.data()) != 'P' || *(line.data()+1) != 'F') {
            fprintf(stderr, "Failed to read image as PFM, magic number wrong: %s\n", line.c_str());
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
    }

    // TODO: read any additional header info

    // Read image width, height and 255 (the maximum pixel value)
    if(input.good()) {
        getline (input, line);

        // Tokenize the string
        std::vector<std::string> x = IoUtil::split(line, ' ');

        // Check we found the right amount of elements:
        if(x.size() != 2) {
            fprintf(stderr, "Expected to read width and height, found %lu numbers!\n", x.size());
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

    // Read scale factor information
    if(input.good()) {
        getline (input, line);

        // Tokenize the string
        std::vector<std::string> x = IoUtil::split(line, ' ');

        // Check we found the right amount of elements:
        if(x.size() != 1) {
            fprintf(stderr, "Expected to read scale factort, found %lu numbers!\n", x.size());
            return;
        }

        // Parse scale factor
        try {
            double scale_factor = std::stod(x[0]);
        }
        catch(std::exception& e) {
            fprintf(stderr, "Couldn't parse scale factor from %s\n", x[0].c_str());
            return;
        }
    }
    else {
        fprintf(stderr, "Ran out of data for parsing image!\n");
        return;
    }

    // Read data section
    rawImage.resize(width*height, 0.0);
    size_t  bytes = width * height * sizeof(double);
    char* pointer = reinterpret_cast<char*>(&rawImage[0]);
    input.read(pointer, bytes);

    return;
}
