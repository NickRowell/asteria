#ifndef RENDERUTIL_H
#define RENDERUTIL_H

#include <vector>

class RenderUtil
{
public:
    RenderUtil();

    static void drawLine(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                         unsigned int x0, unsigned int x1, unsigned int y0, unsigned int y1, unsigned int colour);

    static void drawCircle(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                           double centre_x, double centre_y, double radius, unsigned int colour);
};

#endif // RENDERUTIL_H
