#include "renderutil.h"

#include <cmath>
#include <cstdio>

RenderUtil::RenderUtil()
{

}

void RenderUtil::drawLine(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                          unsigned int x0, unsigned int x1, unsigned int y0, unsigned int y1, unsigned int colour) {

    // If point is at the same coordinates in both frames, r=0 and the
    // trig terms here are NaN. Skip the rest of the drawing method.
    if((x0==x1) && (y0==y1)) {
        return;
    }

    // Get shift in each coordinate between frames
    double delta_i = x1 - x0;
    double delta_j = y1 - y0;

    // Absolute shift in position
    double r_max = std::sqrt(delta_j * delta_j + delta_i * delta_i);

    // Use polar representation to draw a straight line from (x0,y0) to (x1,y1)
    double sin_theta = delta_j / r_max;
    double cos_theta = delta_i / r_max;

    for (double r = 0; r <= r_max; r++) {

        int i = x0 + (int) std::round(r * cos_theta);
        int j = y0 + (int) std::round(r * sin_theta);

        if(i < 0 || i >= width || j < 0 || j >= height) {
            // Point is outside image boundary
            continue;
        }

        unsigned int pIdx = j * width + i;
        annotatedImage[pIdx] = colour;
    }
}

void RenderUtil::drawCircle(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                            double centre_x, double centre_y, double radius, unsigned int colour) {

    // Angular step between points on circumference that lie one pixel apart
    double ang = 2.0 * std::asin(0.5/radius);

    // Loop round circumference of circle:
    for (double theta = 0; theta < 2.0 * M_PI; theta += ang) {

        // Coordinates of pixel on circumference.
        int i = centre_x + (int) std::round(radius * std::sin(theta));
        int j = centre_y + (int) std::round(radius * std::cos(theta));

        if(i < 0 || i >= width || j < 0 || j >= height) {
            // Point is outside image boundary
            continue;
        }

        unsigned int pIdx = j * width + i;
        annotatedImage[pIdx] = colour;
    }
}
