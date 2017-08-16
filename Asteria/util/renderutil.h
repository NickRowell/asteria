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
                            double centre_x, double centre_y, double a, double b, double c, float sigmas, unsigned int colour);
};

#endif // RENDERUTIL_H
