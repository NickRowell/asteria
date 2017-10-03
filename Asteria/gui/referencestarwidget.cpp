#include "referencestarwidget.h"
#include "infra/asteriastate.h"
#include "gui/glmeteordrawer.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "util/timeutil.h"
#include "util/renderutil.h"

#include <QPushButton>
#include <QVBoxLayout>

ReferenceStarWidget::ReferenceStarWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state) {
    medianImageViewer = new GLMeteorDrawer(this, this->state->width, this->state->height);

    QIcon upIcon(":/images/play.png");

    up_button = new QPushButton(this);
    up_button->setIcon(upIcon);
    down_button = new QPushButton(this);
    down_button->setIcon(upIcon);
    left_button = new QPushButton(this);
    left_button->setIcon(upIcon);
    right_button = new QPushButton(this);
    right_button->setIcon(upIcon);
    clockwise_button = new QPushButton(this);
    clockwise_button->setIcon(upIcon);
    anticlockwise_button = new QPushButton(this);
    anticlockwise_button->setIcon(upIcon);
    zoomin_button = new QPushButton(this);
    zoomin_button->setIcon(upIcon);
    zoomout_button = new QPushButton(this);
    zoomout_button->setIcon(upIcon);

    connect(up_button, SIGNAL(pressed()), this, SLOT(up()));
    connect(down_button, SIGNAL(pressed()), this, SLOT(down()));
    connect(left_button, SIGNAL(pressed()), this, SLOT(left()));
    connect(right_button, SIGNAL(pressed()), this, SLOT(right()));
    connect(clockwise_button, SIGNAL(pressed()), this, SLOT(clockwise()));
    connect(anticlockwise_button, SIGNAL(pressed()), this, SLOT(anticlockwise()));
    connect(zoomin_button, SIGNAL(pressed()), this, SLOT(zoomin()));
    connect(zoomout_button, SIGNAL(pressed()), this, SLOT(zoomout()));

    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(up_button);
    controlsLayout->addWidget(down_button);
    controlsLayout->addWidget(left_button);
    controlsLayout->addWidget(right_button);
    controlsLayout->addWidget(clockwise_button);
    controlsLayout->addWidget(anticlockwise_button);
    controlsLayout->addWidget(zoomin_button);
    controlsLayout->addWidget(zoomout_button);
    QWidget * controls = new QWidget(this);
    controls->setLayout(controlsLayout);

    QVBoxLayout *medianImageLayout = new QVBoxLayout;
    medianImageLayout->addWidget(medianImageViewer);
    medianImageLayout->addWidget(controls);
    medianImageLayout->addStretch();
    this->setLayout(medianImageLayout);
}

void ReferenceStarWidget::loadImage(std::shared_ptr<Image> &newImage) {
    image = newImage;
    update();
}

void ReferenceStarWidget::up() {
    fprintf(stderr, "Performing action: up");
    state->elevation += 1.0;
    update();
}

void ReferenceStarWidget::down() {
    fprintf(stderr, "Performing action: down");
    state->elevation -= 1.0;
    update();
}

void ReferenceStarWidget::left() {
    fprintf(stderr, "Performing action: left");
    state->azimuth += 1.0;
    update();
}

void ReferenceStarWidget::right() {
    fprintf(stderr, "Performing action: right");
    state->azimuth -= 1.0;
    update();
}

void ReferenceStarWidget::clockwise() {
    fprintf(stderr, "Performing action: clockwise");
    state->roll += 1.0;
    update();
}

void ReferenceStarWidget::anticlockwise() {
    fprintf(stderr, "Performing action: anticlockwise");
    state->roll -= 1.0;
    update();
}

void ReferenceStarWidget::zoomin() {
    fprintf(stderr, "Performing action: zoomin");
    state->focal_length += 1.0;
    update();
}

void ReferenceStarWidget::zoomout() {
    fprintf(stderr, "Performing action: zoomout");
    state->focal_length -= 1.0;
    update();
}

void ReferenceStarWidget::update() {

    if(!image) {
        // No image loaded yet
        return;
    }

    // Clear the annotated image ready to be filled in with reference stars
    image->annotatedImage.clear();
    image->annotatedImage.reserve(image->width * image->height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < image->width * image->height; p++) {
        image->annotatedImage.push_back(0x00000000);
    }

    // Project the reference stars into the image
    double gmst = TimeUtil::epochToGmst(image->epochTimeUs);

    double lon = MathUtil::toRadians(state->longitude);
    double lat = MathUtil::toRadians(state->latitude);
    double az = MathUtil::toRadians(state->azimuth);
    double el = MathUtil::toRadians(state->elevation);
    double roll = MathUtil::toRadians(state->roll);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = CoordinateUtil::getSezToCamRot(az, el, roll);
    Matrix3d r_cam_im = CoordinateUtil::getCamIntrinsicMatrix(state->focal_length, state->pixel_width, state->pixel_height, state->width, state->height);

    // Full transformation
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    for(ReferenceStar &star : state->refStarCatalogue) {

        // Unit vector towards star in original frame:
        Vector3d r_bcrf;
        CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, star.ra, star.dec);
        // Transform to CAM frame:
        Vector3d r_cam = r_bcrf_cam * r_bcrf;

        if(r_cam[2] < 0) {
            // Star is behind the camera
            continue;
        }

        // Project into image coordinates
        Vector3d r_im = r_cam_im * r_cam;

        double i = r_im[0] / r_im[2];
        double j = r_im[1] / r_im[2];

        if(i>0 && i<state->width && j>0 && j<state->height) {
            // Star is visible in image!
            //fprintf(stderr, "%f\t%f\t%f\n", i, j, star.mag);
            int ii = (int)std::round(i);
            int jj = (int)std::round(j);
            RenderUtil::drawCrossHair(image->annotatedImage, image->width, image->height, ii, jj, 0xFFFF00FF);

        }

    }

    medianImageViewer->newFrame(image, true, true, true);
}




