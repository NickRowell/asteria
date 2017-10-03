#ifndef RENDERUTIL_H
#define RENDERUTIL_H

#include <vector>

class RenderUtil
{
public:
    RenderUtil();

    static void drawLine(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                         int x0, int x1, int y0, int y1, unsigned int colour);

    static void drawCircle(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                           double centre_x, double centre_y, double radius, unsigned int colour);

    static void drawEllipse(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                            double &centre_x, double &centre_y, double &a, double &b, double &c, float sigmas, unsigned int &colour);

    static void drawCrossHair(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                         int x0, int y0, unsigned int colour);

    static void encodeRgb(const unsigned char &r, const unsigned char &g, const unsigned char &b, unsigned int &rgb);

    static void decodeRgb(unsigned char &r, unsigned char &g, unsigned char &b, const unsigned int &rgb);
};

#endif // RENDERUTIL_H
