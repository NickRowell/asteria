#ifndef RENDERUTIL_H
#define RENDERUTIL_H

#include "infra/source.h"
#include "infra/imageui.h"

#include <vector>
#include <memory>

class RenderUtil
{
public:
    RenderUtil();

    static void drawLine(std::vector<unsigned int> &pixels, unsigned int &width, unsigned int &height,
                         int x0, int x1, int y0, int y1, unsigned int colour);

    static void drawCircle(std::vector<unsigned int> &pixels, unsigned int &width, unsigned int &height,
                           double centre_x, double centre_y, double radius, unsigned int colour);

    /**
     * @brief RenderUtil::drawEllipse
     * @param pixels
     * @param width
     * @param height
     * @param centre_x
     * @param centre_y
     * @param a
     * @param b
     * @param c
     * @param sigmas
     * @param colour
     * Covariance matrix is
     *
     * [a b]
     * [b c]
     *
     * in units is pixels.
     */
    static void drawEllipse(std::vector<unsigned int> &pixels, unsigned int &width, unsigned int &height,
                            double &centre_x, double &centre_y, double &a, double &b, double &c, float sigmas, unsigned int &colour);

    static void drawCrossHair(std::vector<unsigned int> &pixels, unsigned int &width, unsigned int &height,
                         int x0, int y0, unsigned int length, unsigned int gap, unsigned int colour);

    static void drawSources(std::vector<unsigned int> &pixels, std::vector<Source> &sources, unsigned int &width, unsigned int &height, bool fill);

    static void encodeRgb(const unsigned char &r, const unsigned char &g, const unsigned char &b, unsigned int &rgb);

    static void decodeRgb(unsigned char &r, unsigned char &g, unsigned char &b, const unsigned int &rgb);

    static void encodeRgba(const unsigned char &r, const unsigned char &g, const unsigned char &b, const unsigned char &a, unsigned int &rgba);

    static void decodeRgba(unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a, const unsigned int &rgba);
};

#endif // RENDERUTIL_H
