#include "referencestarwidget.h"
#include "infra/asteriastate.h"
#include "gui/glmeteordrawer.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "util/timeutil.h"
#include "util/renderutil.h"
#include "util/ioutil.h"
#include "gui/doubleslider.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>

// TODO: display azimuth, elevation, roll and focal length in the GUI, and icons depcting mouse controls.
// TODO: integrate camera matrix class that provides project and deproject functions and radial distortion etc
// TODO: formalise the principal point etc; don't base the camera orientation internally on the azimuth, elevation etc.
// TODO: on clicks, identify the nearest reference star in the image and store to internal pointer for use in cross matching with sources

ReferenceStarWidget::ReferenceStarWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state) {

    medianImageViewer = new GLMeteorDrawer(this, this->state->width, this->state->height);

    // Capture mouse clicks etc in the GLMeteorDrawer
    medianImageViewer->installEventFilter(this);

    // Initialise bools to track mouse button presses
    // FIXME: what if the buttons ARE pressed when the class is initialised?
    leftButtonIsPressed = false;
    middleButtonIsPressed = false;
    rightButtonIsPressed = false;

    QPushButton * up_button = new QPushButton(this);
    up_button->setIcon(QIcon(":/images/up.png"));
    QPushButton * down_button = new QPushButton(this);
    down_button->setIcon(QIcon(":/images/down.png"));
    QPushButton * left_button = new QPushButton(this);
    left_button->setIcon(QIcon(":/images/left.png"));
    QPushButton * right_button = new QPushButton(this);
    right_button->setIcon(QIcon(":/images/right.png"));
    QPushButton * clockwise_button = new QPushButton(this);
    clockwise_button->setIcon(QIcon(":/images/clockwise.png"));
    QPushButton * anticlockwise_button = new QPushButton(this);
    anticlockwise_button->setIcon(QIcon(":/images/anticlockwise.png"));
    QPushButton * zoomin_button = new QPushButton(this);
    zoomin_button->setIcon(QIcon(":/images/plus.png"));
    QPushButton * zoomout_button = new QPushButton(this);
    zoomout_button->setIcon(QIcon(":/images/minus.png"));
    // TODO: reference star catalogue should know the magnitude range of it's contents
    DoubleSlider * slider = new DoubleSlider(this, -1.0, 6.0, state->ref_star_faint_mag_limit, 100);

    connect(up_button, SIGNAL(pressed()), this, SLOT(up()));
    connect(down_button, SIGNAL(pressed()), this, SLOT(down()));
    connect(left_button, SIGNAL(pressed()), this, SLOT(left()));
    connect(right_button, SIGNAL(pressed()), this, SLOT(right()));
    connect(clockwise_button, SIGNAL(pressed()), this, SLOT(clockwise()));
    connect(anticlockwise_button, SIGNAL(pressed()), this, SLOT(anticlockwise()));
    connect(zoomin_button, SIGNAL(pressed()), this, SLOT(zoomin()));
    connect(zoomout_button, SIGNAL(pressed()), this, SLOT(zoomout()));
    // Player response to user moving the slider
    connect(slider, SIGNAL(doubleSliderMoved(double)), this, SLOT(slide(double)));

    refStarMagSliderGroupBox = new QGroupBox(QString("Reference stars faint magnitude limit [%1]").arg(state->ref_star_faint_mag_limit, 0, 'f', 2));
    QVBoxLayout *sliderVbox = new QVBoxLayout;
    sliderVbox->addWidget(slider);
    sliderVbox->addStretch(1);
    refStarMagSliderGroupBox->setLayout(sliderVbox);

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
    medianImageLayout->addWidget(refStarMagSliderGroupBox);
    medianImageLayout->addStretch();
    this->setLayout(medianImageLayout);
}

void ReferenceStarWidget::loadImage(std::shared_ptr<Image> &newImage) {
    image = newImage;
    update();
}

void ReferenceStarWidget::mousePressEvent(QMouseEvent *e) {

    // Position within the median image
    QPoint mouse = medianImageViewer->mapFromGlobal(e->globalPos());
    mousePrevI = mouse.x();
    mousePrevJ = mouse.y();

    switch(e->button()) {
    case Qt::LeftButton:
        leftButtonIsPressed = true;
        break;
    case Qt::MiddleButton:
        middleButtonIsPressed = true;
        break;
    case Qt::RightButton:
        rightButtonIsPressed = true;
        break;
    default:
        fprintf(stderr, "Unsupported button: %s\n", IoUtil::mouseButtonEnumNameFromValue(e->button()).toStdString().c_str());
        break;
    }
}

void ReferenceStarWidget::mouseReleaseEvent(QMouseEvent *e) {

    // Position within the median image (mouse.x(), mouse.y())
    QPoint mouse = medianImageViewer->mapFromGlobal(e->globalPos());

    switch(e->button()) {
    case Qt::LeftButton:
        leftButtonIsPressed = false;
        break;
    case Qt::MiddleButton:
        middleButtonIsPressed = false;
        break;
    case Qt::RightButton:
        rightButtonIsPressed = false;
        break;
    default:
        fprintf(stderr, "Unsupported button: %s\n", IoUtil::mouseButtonEnumNameFromValue(e->button()).toStdString().c_str());
        break;
    }
}

void ReferenceStarWidget::mouseDoubleClickEvent(QMouseEvent *e) {

    // Position within the median image (mouse.x(), mouse.y())
    QPoint mouse = medianImageViewer->mapFromGlobal(e->globalPos());

    switch(e->button()) {
    case Qt::LeftButton:
        // Do something?
        break;
    case Qt::MiddleButton:
        // Do something?
        break;
    case Qt::RightButton:
        // Do something?
        break;
    default:
        fprintf(stderr, "Unsupported button: %s\n", IoUtil::mouseButtonEnumNameFromValue(e->button()).toStdString().c_str());
        break;
    }
}

void ReferenceStarWidget::mouseMoveEvent(QMouseEvent *e) {

    // Position within the median image
    QPoint mouse = medianImageViewer->mapFromGlobal(e->globalPos());

    if(mouse.x() == mousePrevI && mouse.y() == mousePrevJ) {
        // Sanity check: no motion recorded, nothing to do.
        return;
    }

    // Drags with the left & right mouse buttons rotate the camera frame in different ways
    Eigen::Quaterniond q;

    if(leftButtonIsPressed) {
        // Drags with the left button adjust the azimuth & elevation of the camera pointing

        // Get the inverse of the camera matrix, for deprojecting pixels
        Matrix3d r_im_cam = CoordinateUtil::getCamIntrinsicMatrixInverse(state->focal_length, state->pixel_width, state->pixel_height, state->width, state->height);

        // Homogenous vectors of the image plane coordinates
        Vector3d r0(mousePrevI, mousePrevJ, 1.0);
        Vector3d r1(mouse.x(), mouse.y(), 1.0);

        // Deproject these to get the unit vectors in the camera 3D frame
        Vector3d los0 = r_im_cam * r0;
        Vector3d los1 = r_im_cam * r1;
        // Quaternion that rotates los0 to los1
        q = Eigen::Quaterniond::FromTwoVectors(los0, los1);
    }
    if(middleButtonIsPressed) {
        // Drags with the middle button do nothing

        // Cache the new mouse position
        mousePrevI = mouse.x();
        mousePrevJ = mouse.y();

        return;
    }
    if(rightButtonIsPressed) {
        // Drags with the right button rotate the camera about the boresight

        // Vectors in the image plane between the principal point and the previous/current mouse location
        double px = (double)state->width/2.0;
        double py = (double)state->height/2.0;
        Vector3d r0(mousePrevI - px, mousePrevJ - py, 0.0);
        Vector3d r1(mouse.x() - px, mouse.y() - py, 0.0);

        // Quaternion that rotates these
        q = Eigen::Quaterniond::FromTwoVectors(r0, r1);
    }

    // Now rotate the camera extrinsic matrix by this much
    Matrix3d r = q.toRotationMatrix();

    Matrix3d r_sez_cam = CoordinateUtil::getSezToCamRot(MathUtil::toRadians(state->azimuth), MathUtil::toRadians(state->elevation), MathUtil::toRadians(state->roll));

    // Rotate the camera matrix by this factor
    r_sez_cam = r * r_sez_cam;

    // Get azimuth, altitude, roll from the rotation matrix
    double az, el, roll;
    CoordinateUtil::getAzElRoll(r_sez_cam, az, el, roll);

    // Reset the state fields with the new orientation
    state->azimuth = MathUtil::toDegrees(az);
    state->elevation = MathUtil::toDegrees(el);
    state->roll = MathUtil::toDegrees(roll);

    // Cache the new mouse position
    mousePrevI = mouse.x();
    mousePrevJ = mouse.y();

    update();
}



void ReferenceStarWidget::wheelEvent(QWheelEvent *e) {

    int wheelDelta = e->delta();

    // Fudge wheelDelta to some value in the vicinity of 1.0 that gives a
    // reasonable scale factor to apply to the focal length. Note that using
    // a scale factor rather than a fixed delta gives better interaction: we
    // get larger shifts when the focal length is greater, and smaller
    // shifts when we're closer to the origin.
    //  - Up one click:   delta =  120
    //  - Down one click: delta = -120
    //  - Basic idea is to scale wheel motion to +/-0.01 and add to 1.0
    //    to obtain a scale factor of either 0.99 or 1.01 depending on which
    //    direction the user pushed the mouse wheel.
    double zoom = 1.0 - 0.01*(wheelDelta/120);

    state->focal_length *= zoom;

    update();
}



void ReferenceStarWidget::up() {

    // Compute change in az/el required to move stars 'up' in image.
    state->elevation += 1.0 * std::cos(MathUtil::toRadians(state->roll));
    state->azimuth += 1.0 * std::sin(MathUtil::toRadians(state->roll));
    fprintf(stderr, "Performing action: up; elevation = %f\n", state->elevation);
    update();
}

void ReferenceStarWidget::down() {

    // Compute change in az/el required to move stars 'down' in image.
    state->elevation -= 1.0 * std::cos(MathUtil::toRadians(state->roll));
    state->azimuth -= 1.0 * std::sin(MathUtil::toRadians(state->roll));

    fprintf(stderr, "Performing action: down; elevation = %f\n", state->elevation);
    update();
}

void ReferenceStarWidget::left() {

    // Compute change in az/el required to move stars 'left' in image.
    state->elevation += 1.0 * std::sin(MathUtil::toRadians(state->roll));
    state->azimuth -= 1.0 * std::cos(MathUtil::toRadians(state->roll));

    fprintf(stderr, "Performing action: left; azimuth = %f\n", state->azimuth);
    update();
}

void ReferenceStarWidget::right() {

    // Compute change in az/el required to move stars 'right' in image.
    state->elevation -= 1.0 * std::sin(MathUtil::toRadians(state->roll));
    state->azimuth += 1.0 * std::cos(MathUtil::toRadians(state->roll));

    fprintf(stderr, "Performing action: right; azimuth = %f\n", state->azimuth);
    update();
}

void ReferenceStarWidget::clockwise() {
    state->roll -= 1.0;
    fprintf(stderr, "Performing action: clockwise; roll = %f\n", state->roll);
    update();
}

void ReferenceStarWidget::anticlockwise() {
    state->roll += 1.0;
    fprintf(stderr, "Performing action: anticlockwise; roll = %f\n", state->roll);
    update();
}

void ReferenceStarWidget::zoomin() {
    state->focal_length += 0.1;
    fprintf(stderr, "Performing action: zoomin; f = %f\n", state->focal_length);
    update();
}

void ReferenceStarWidget::zoomout() {
    if(state->focal_length > 0.1) {
        state->focal_length -= 0.1;
    }
    fprintf(stderr, "Performing action: zoomout; f = %f\n", state->focal_length);
    update();
}

void ReferenceStarWidget::slide(double position) {
    state->ref_star_faint_mag_limit = position;
    refStarMagSliderGroupBox->setTitle(QString("Reference stars faint magnitude limit [%1]").arg(state->ref_star_faint_mag_limit, 0, 'f', 2));
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

    // Transformation BCRF->SEZ
    Matrix3d r_bcrf_sez = r_ecef_sez * r_bcrf_ecef;

    for(ReferenceStar &star : state->refStarCatalogue) {

        // Reject stars fainter than faint mag limit
        if(star.mag > state->ref_star_faint_mag_limit) {
            continue;
        }

        // Unit vector towards star in original frame:
        Vector3d r_bcrf;
        CoordinateUtil::sphericalToCartesian(r_bcrf, 1.0, star.ra, star.dec);
        // Transform to CAM frame:
        Vector3d r_cam = r_bcrf_cam * r_bcrf;

        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        // TEMP
        //
        // Transform to SEZ frame:
        Vector3d r_sez = r_bcrf_sez * r_bcrf;
        // Get position in spherical coordinates
        double r, theta, phi;
        CoordinateUtil::cartesianToSpherical(r_sez, r, theta, phi);
        // Transform east-of-south angle to east-of-north for conventional azimuth
        CoordinateUtil::eastOfSouthToEastOfNorth(theta);
        //
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        if(r_cam[2] < 0) {
            // Star is behind the camera
            continue;
        }

        // Project into image coordinates
        Vector3d r_im = r_cam_im * r_cam;

        star.i = r_im[0] / r_im[2];
        star.j = r_im[1] / r_im[2];

        if(star.i>0 && star.i<state->width && star.j>0 && star.j<state->height) {
            // Star is visible in image!
            int ii = (int)std::round(star.i);
            int jj = (int)std::round(star.j);

            // Translate magnitude of star to gap size in cross hair
            double m0 = -1.0; // Bright magnitude limit
            double g0 = 6.0; // Maximum gap; for stars at the bright magnitude limit
            double m1 = state->ref_star_faint_mag_limit; // Faint magnitude limit
            double g1 = 2.0; // Minimum gap; for stars at the faint magnitude limit

            double gap;
            if(star.mag > m1) {
                gap = g1;
            }
            else if(star.mag < m0) {
                gap = g0;
            }
            else {
                gap = g0 + (star.mag - m0) * ((g1 - g0)/(m1 - m0));
            }

            unsigned int gap_int = (unsigned int)std::round(gap);

            RenderUtil::drawCrossHair(image->annotatedImage, image->width, image->height, ii, jj, 5, gap_int, 0xFFFF00FF);

        }
    }

    // Draw a crosshair at the image principal point
    RenderUtil::drawCrossHair(image->annotatedImage, image->width, image->height, image->width/2, image->height/2, 10, 0, 0x00FFFFFF);

    medianImageViewer->newFrame(image, true, true, true);
}



