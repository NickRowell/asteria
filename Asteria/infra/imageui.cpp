#include "infra/imageui.h"
#include "util/ioutil.h"
#include "util/renderutil.h"

Imageui::Imageui() : Image<unsigned int>() {
}

Imageui::Imageui(const Imageui& copyme) : Image<unsigned int>(copyme) {
}

Imageui::Imageui(unsigned int &width, unsigned int &height) : Image<unsigned int>(width, height) {
}

Imageui::Imageui(unsigned int &width, unsigned int &height, unsigned int val) : Image<unsigned int>(width, height, val) {
}

Imageui::~Imageui() {
}

void Imageui::writeToStream(std::ostream &output) const {

    // Function to write an Image to file

    // Raw PPMs:
    output << "P6\n";

    // Write the data section
    output << width << " " << height << " 255\n";

    // Write raster
    for(unsigned int i=0; i<width*height; i++) {
        unsigned char r, g, b, a;
        RenderUtil::decodeRgba(r, g, b, a, rawImage[i]);
        output << r;
        output << g;
        output << b;
    }

    return;
}

void Imageui::readFromStream(std::istream &input) {

    // Function to load an Image from file

    // Read image signature
    std::string line;
    if(input.good()) {
        getline (input, line);
        // Check magic number (first two chars in file)
        if(*(line.data()) != 'P' || *(line.data()+1) != '6') {
            fprintf(stderr, "Failed to read image as PPM, magic number wrong: %s\n", line.c_str());
            return;
        }
    }
    else {
        fprintf(stderr, "Ran out of data for parsing image!\n");
        return;
    }

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

    // Read data section
    rawImage.clear();
    // Read 3 bytes at a time
    char bytes[3];
    for(unsigned int i=0; i<width*height; i++) {
        input.read(bytes, 3);
        unsigned char r = bytes[0];
        unsigned char g = bytes[1];
        unsigned char b = bytes[2];
        unsigned int rgba;
        RenderUtil::encodeRgba(r, g, b, 0xFF, rgba);
        rawImage.push_back(rgba);
    }

    return;
}
