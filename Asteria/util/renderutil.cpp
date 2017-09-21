#include "renderutil.h"

#include <cmath>
#include <cstdio>

RenderUtil::RenderUtil()
{

}

void RenderUtil::drawLine(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                          int x0, int x1, int y0, int y1, unsigned int colour) {

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
        int i = (int) std::round(centre_x + radius * std::sin(theta));
        int j = (int) std::round(centre_y + radius * std::cos(theta));

        if(i < 0 || i >= width || j < 0 || j >= height) {
            // Point is outside image boundary
            continue;
        }

        unsigned int pIdx = j * width + i;
        annotatedImage[pIdx] = colour;
    }
}

/**
 * @brief RenderUtil::drawEllipse
 * @param annotatedImage
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
void RenderUtil::drawEllipse(std::vector<unsigned int> &annotatedImage, unsigned int &width, unsigned int &height,
                             double &centre_x, double &centre_y, double &a, double &b, double &c, float sigmas, unsigned int &colour) {

    // Eigenvalues of image covariance matrix:
    double trA  = a+c;
    double detA = a*c - b*b;
    // intermediate quantity
    double temp = std::sqrt(trA*trA - 4.0*detA)/2.0;
    // actual eigenvalues
    double eVal1 = trA/2.0f + temp;
    double eVal2 = trA/2.0f - temp;

    // Compute normalised eigenvectors from these
    double norm1 = 1.0/std::sqrt(b*b/((a-eVal1)*(a-eVal1)) + 1.0);
    double norm2 = 1.0/std::sqrt(b*b/((a-eVal2)*(a-eVal2)) + 1.0);

    // Components of eigenvector matrix in image plane
    //      [ A   B ]
    //  V = |       |
    //      [ C   D ]
    //
    double eVec1_x = norm1 * (b/(a-eVal1));
    double eVec2_x = norm2 * (b/(a-eVal2));
    double eVec1_y = norm1;
    double eVec2_y = norm2;

    // Check determinant: if it is -ve, then must reverse one of the
    // eigenvectors in order to avoid doing a reflection as well as
    // rotation when we apply this to our circular points later.
    double det = eVec1_x*eVec2_y - eVec2_x*eVec1_y;

    if(det<0) {
        // Reverse first eigenvector.
        eVec1_x *= -1.0;
        eVec1_y *= -1.0;
    }

    // Length of major axes of ellipse - number of standard deviations
    double s_p = std::sqrt(eVal1) * sigmas;
    double s_q = std::sqrt(eVal2) * sigmas;

    // Set angular step size so that distance travelled round circumference
    // is max one pixel (occurs at the major axis).
    double ang_step = std::asin(1.0 / std::max(s_p,s_q));

    double c_ang,s_ang,r;

    // Loop round circumference of circle:
    for (double theta = 0; theta < 2.0 * M_PI; theta += ang_step) {

        c_ang = std::cos(theta);
        s_ang = std::sin(theta);

        // Get radius of ellipse at this angle
        r = s_p * s_q / std::sqrt(s_p * s_p * s_ang * s_ang + s_q * s_q * c_ang * c_ang);

        // Rotate point at (r*c_ang, r*s_ang) back to image frame
        double dx = eVec1_x * r * c_ang + eVec1_y * r * s_ang;
        double dy = eVec2_x * r * c_ang + eVec2_y * r * s_ang;

        // Add centre position; quantize to whole pixels; get pixel index and write to image if within range
        int i = (int) std::round(centre_x + dx);
        int j = (int) std::round(centre_y + dy);

        if(i < 0 || i >= (int)width || j < 0 || j >= (int)height) {
            // Point is outside image boundary
            continue;
        }

        unsigned int pIdx = j * width + i;
        annotatedImage[pIdx] = colour;
    }
}

void RenderUtil::encodeRgb(const unsigned char &r, const unsigned char &g, const unsigned char &b, unsigned int &rgb) {
    rgb = (r << 16) + (g << 8) + b;
}

void RenderUtil::decodeRgb(unsigned char &r, unsigned char &g, unsigned char &b, const unsigned int &rgb) {
    r = (rgb >> 16) & 0xFF;
    g = (rgb >>  8) & 0xFF;
    b = (rgb >>  0) & 0xFF;
}
