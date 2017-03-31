#include "jpgutil.h"

JpgUtil::JpgUtil()
{

}

void JpgUtil::convertYuyv422(unsigned char * buffer, const unsigned long insize, std::vector<char> &decodedImage) {

    // Pointer to data in input image buffer
    unsigned char * pBuf = buffer;

    // Pixels are encoded in groups of four bytes
    for(unsigned int p=0; p<insize; p += 4) {

        char y1 = *(pBuf++);
        char cr = *(pBuf++);
        char y2 = *(pBuf++);
        char cb = *(pBuf++);

        // Discard the colour information, or use it somehow?
        decodedImage.push_back(y1);
        decodedImage.push_back(y2);
    }
}

void JpgUtil::convertJpeg(unsigned char * buffer, const unsigned long insize, std::vector<char> &decodedImage) {

    unsigned char r, g, b;
    int width, height;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Read JPEG from file
//    const char * Name;  /* File name */
//    FILE * infile;        /* source file */
//    if ((infile = fopen(Name, "rb")) == NULL) {
//        fprintf(stderr, "can't open %s\n", Name);
//        return 0;
//    }
//    jpeg_stdio_src(&cinfo, infile);
//    fclose(infile);

    // Read JPEG from memory buffer
    jpeg_mem_src(&cinfo, buffer, insize);

    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    width = cinfo.output_width;
    height = cinfo.output_height;

    int row_stride = width * cinfo.output_components;
    // Output row buffer
    JSAMPARRAY pJpegBuffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

//    char * pDec = decodedImage;

    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
        for (int x = 0; x < width; x++) {

            r = pJpegBuffer[0][cinfo.output_components * x];
            if (cinfo.output_components > 2) {
                g = pJpegBuffer[0][cinfo.output_components * x + 1];
                b = pJpegBuffer[0][cinfo.output_components * x + 2];
            } else {
                g = r;
                b = r;
            }

            // Convert RGB to greyscale
//            *(pDec++) = (r + g + b)/3;
            char grey = (r + g + b)/3;
            decodedImage.push_back(grey);
        }
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}



