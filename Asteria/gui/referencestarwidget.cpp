#include "referencestarwidget.h"
#include "infra/asteriastate.h"
#include "gui/glmeteordrawer.h"
#include "util/coordinateutil.h"
#include "util/mathutil.h"
#include "util/timeutil.h"
#include "util/renderutil.h"
#include "util/ioutil.h"
#include "gui/doubleslider.h"
#include "infra/calibrationinventory.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>

// TODO: enable creation of cross-matches by user clicks.
// TODO: remove cross-matches for stars that move out of FOV during user adjustment.
// TODO: display azimuth, elevation, roll and focal length in the GUI, and icons depicting mouse controls.

ReferenceStarWidget::ReferenceStarWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state) {

    signalImageViewer = new GLMeteorDrawer(this, this->state->width, this->state->height);

    selectedRefStar = 0;

    // Initialise bools to track mouse button presses
    // FIXME: what if the buttons ARE pressed when the class is initialised?
    leftButtonIsPressed = false;
    middleButtonIsPressed = false;
    rightButtonIsPressed = false;

    displayRefStars = false;
    displaySources = false;
    displayGeoCal = false;

    // TODO: reference star catalogue should know the magnitude range of it's contents
    DoubleSlider * slider = new DoubleSlider(this, -1.0, 6.0, state->ref_star_faint_mag_limit, 100);

    // Player response to user moving the slider
    connect(slider, SIGNAL(doubleSliderMoved(double)), this, SLOT(slide(double)));

    QCheckBox * displayRefStarsCheckbox = new QCheckBox("Show &reference stars", this);
    QCheckBox * displaySourcesCheckbox = new QCheckBox("Show &extracted sources", this);
    QCheckBox * displayGeoCalCheckbox = new QCheckBox("Show &camera calibration", this);

    connect(displayRefStarsCheckbox, SIGNAL(stateChanged(int)), this, SLOT(toggleDisplayRefStars(int)));
    connect(displaySourcesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(toggleDisplaySources(int)));
    connect(displayGeoCalCheckbox, SIGNAL(stateChanged(int)), this, SLOT(toggleDisplayGeoCal(int)));

    refStarMagSliderGroupBox = new QGroupBox(QString("Reference stars faint magnitude limit [%1]").arg(state->ref_star_faint_mag_limit, 0, 'f', 2));
    QVBoxLayout *sliderVbox = new QVBoxLayout;
    sliderVbox->addWidget(slider);
    sliderVbox->addStretch(1);
    refStarMagSliderGroupBox->setLayout(sliderVbox);

    QHBoxLayout *displayOptsLayout = new QHBoxLayout;
    displayOptsLayout->addWidget(displayRefStarsCheckbox);
    displayOptsLayout->addWidget(displaySourcesCheckbox);
    displayOptsLayout->addWidget(displayGeoCalCheckbox);
    displayOptsLayout->addStretch();
    QWidget * displayOptsWidget = new QWidget(this);
    displayOptsWidget->setLayout(displayOptsLayout);

    QVBoxLayout *medianImageLayout = new QVBoxLayout;
    medianImageLayout->addWidget(signalImageViewer);
    medianImageLayout->addWidget(refStarMagSliderGroupBox);
    medianImageLayout->addWidget(displayOptsWidget);
    medianImageLayout->addStretch();
    this->setLayout(medianImageLayout);
}

void ReferenceStarWidget::loadCalibration(std::shared_ptr<CalibrationInventory> inv) {
    this->inv = inv;
    this->signal = make_shared<Imageuc>(*(this->inv->signal));
    update();
}

void ReferenceStarWidget::toggleDisplayRefStars(int checkBoxState) {
    switch(checkBoxState) {
    case Qt::Checked:
        displayRefStars = true;
        break;
    case Qt::Unchecked:
        displayRefStars = false;
        break;
    }
    update();
}

void ReferenceStarWidget::toggleDisplaySources(int checkBoxState) {
    switch(checkBoxState) {
    case Qt::Checked:
        displaySources = true;
        break;
    case Qt::Unchecked:
        displaySources = false;
        break;
    }
    update();
}

void ReferenceStarWidget::toggleDisplayGeoCal(int checkBoxState) {
    switch(checkBoxState) {
    case Qt::Checked:
        displayGeoCal = true;
        break;
    case Qt::Unchecked:
        displayGeoCal = false;
        break;
    }
    update();
}

void ReferenceStarWidget::mousePressEvent(QMouseEvent *e) {

    // Position within the median image
    QPoint mouse = signalImageViewer->mapFromGlobal(e->globalPos());
    mousePrevI = mouse.x();
    mousePrevJ = mouse.y();
    mouseStartI = mouse.x();
    mouseStartJ = mouse.y();

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
    QPoint mouse = signalImageViewer->mapFromGlobal(e->globalPos());

    switch(e->button()) {
    case Qt::LeftButton:
        leftButtonIsPressed = false;

        if(mouseStartI == mouse.x() && mouseStartJ == mouse.y()) {
            // No drag occurred, just a click: locate the closest reference star
            selectedRefStar = 0;
            double minDistanceToReferenceStar = std::sqrt(state->width * state->width + state->height * state->height);

            for(ReferenceStar * refStarPtr : visibleReferenceStars) {

                double di = refStarPtr->i - mouse.x();
                double dj = refStarPtr->j - mouse.y();

                // Quick bounding box check to reject reference stars much further away than the current limit
                if(std::fabs(di) > minDistanceToReferenceStar || std::fabs(dj) > minDistanceToReferenceStar) {
                    continue;
                }
                // Passed bounding box check: compute Euclidean distance
                double distanceToReferenceStar = std::sqrt(di * di + dj * dj);

                if(distanceToReferenceStar < minDistanceToReferenceStar) {
                    minDistanceToReferenceStar = distanceToReferenceStar;
                    selectedRefStar = refStarPtr;
                }
            }

            if(selectedRefStar) {
                // A ReferenceStar was selected - update the image to indicate it
                update();
            }
        }
        break;
    case Qt::MiddleButton:
        middleButtonIsPressed = false;
        break;
    case Qt::RightButton:
        rightButtonIsPressed = false;

        if(mouseStartI == mouse.x() && mouseStartJ == mouse.y()) {
            // No drag occurred, just a click: clear the reference star selection
            if(selectedRefStar) {
                selectedRefStar = 0;
                update();
            }
        }
        break;
    default:
        fprintf(stderr, "Unsupported button: %s\n", IoUtil::mouseButtonEnumNameFromValue(e->button()).toStdString().c_str());
        break;
    }
}

void ReferenceStarWidget::mouseDoubleClickEvent(QMouseEvent *e) {

    // Position within the median image (mouse.x(), mouse.y())
    QPoint mouse = signalImageViewer->mapFromGlobal(e->globalPos());

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
    QPoint mouse = signalImageViewer->mapFromGlobal(e->globalPos());

    if(mouse.x() == mousePrevI && mouse.y() == mousePrevJ) {
        // Sanity check: no motion recorded, nothing to do.
        return;
    }

    // Drags with the left & right mouse buttons rotate the camera frame in different ways
    Eigen::Quaterniond q;

    if(leftButtonIsPressed) {
        // Drags with the left button adjust the azimuth & elevation of the camera pointing
        Vector3d r0 = inv->cam->deprojectPixel(mousePrevI, mousePrevJ);
        Vector3d r1 = inv->cam->deprojectPixel(mouse.x(), mouse.y());

        // Quaternion that rotates los0 to los1
        q = Eigen::Quaterniond::FromTwoVectors(r0, r1);
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
        double pi, pj;
        inv->cam->getPrincipalPoint(pi, pj);
        Vector3d r0(mousePrevI - pi, mousePrevJ - pj, 0.0);
        Vector3d r1(mouse.x() - pi, mouse.y() - pj, 0.0);

        // Quaternion that rotates these
        q = Eigen::Quaterniond::FromTwoVectors(r0, r1);
    }

    // Apply the rotation to the camera orientation
    inv->q_sez_cam = q * inv->q_sez_cam;
    inv->q_sez_cam.normalize();

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

    inv->cam->zoom(zoom);

    update();
}

void ReferenceStarWidget::slide(double position) {
    state->ref_star_faint_mag_limit = position;
    refStarMagSliderGroupBox->setTitle(QString("Reference stars faint magnitude limit [%1]").arg(state->ref_star_faint_mag_limit, 0, 'f', 2));
    update();
}

void ReferenceStarWidget::update() {

    if(!inv) {
        // No calibration loaded yet
        return;
    }

    // Clear the annotated image ready to be filled in with reference stars
    signal->annotatedImage.clear();
    signal->annotatedImage.reserve(signal->width * signal->height);

    // Initialise to full transparency
    for(unsigned int p = 0; p < signal->width * signal->height; p++) {
        signal->annotatedImage.push_back(0x00000000);
    }

    // Clear the current set of visible reference stars
    visibleReferenceStars.clear();

    // Project the reference stars into the image
    double gmst = TimeUtil::epochToGmst(inv->epochTimeUs);

    double lon = MathUtil::toRadians(inv->longitude);
    double lat = MathUtil::toRadians(inv->latitude);

    // Rotation matrices
    Matrix3d r_bcrf_ecef = CoordinateUtil::getBcrfToEcefRot(gmst);
    Matrix3d r_ecef_sez  = CoordinateUtil::getEcefToSezRot(lon, lat);
    Matrix3d r_sez_cam = inv->q_sez_cam.toRotationMatrix();

    // Full transformation BCRF->CAM
    Matrix3d r_bcrf_cam = r_sez_cam * r_ecef_sez * r_bcrf_ecef;

    for(ReferenceStar &star : state->refStarCatalogue) {

        // Skip stars fainter than faint mag limit
        if(star.mag > state->ref_star_faint_mag_limit) {
            continue;
        }

        CoordinateUtil::projectReferenceStar(star, r_bcrf_cam, *(inv->cam));

        if(star.visible) {
            // Star is visible in image!
            visibleReferenceStars.push_back(&star);
        }
    }

    if(displayRefStars) {

        for(ReferenceStar * star : visibleReferenceStars) {

            // Quantize coordinates for display in image
            int ii = (int)std::round(star->i);
            int jj = (int)std::round(star->j);

            // Translate magnitude of star to gap size in cross hair
            double m0 = -1.0; // Bright magnitude limit
            double g0 = 6.0; // Maximum gap; for stars at the bright magnitude limit
            double m1 = state->ref_star_faint_mag_limit; // Faint magnitude limit
            double g1 = 2.0; // Minimum gap; for stars at the faint magnitude limit

            double gap;
            if(star->mag > m1) {
                gap = g1;
            }
            else if(star->mag < m0) {
                gap = g0;
            }
            else {
                gap = g0 + (star->mag - m0) * ((g1 - g0)/(m1 - m0));
            }

            unsigned int gap_int = (unsigned int)std::round(gap);

            RenderUtil::drawCrossHair(signal->annotatedImage, signal->width, signal->height, ii, jj, 5, gap_int, 0xFF00FFFF);
        }

        if(selectedRefStar) {
            int ii = (int)std::round(selectedRefStar->i);
            int jj = (int)std::round(selectedRefStar->j);
            RenderUtil::drawCrossHair(signal->annotatedImage, signal->width, signal->height, ii, jj, 10, 0, 0x0000FFFF);
        }
    }

    if(displaySources) {
        // Render extracted sources
        RenderUtil::drawSources(signal->annotatedImage, inv->sources, signal->width, signal->height, false);
    }

    if(displaySources && displayRefStars) {
        // Render the cross-matches
        for(std::pair<Source, ReferenceStar> &xm : inv->xms) {
            RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, xm.first.i, xm.second.i, xm.first.j, xm.second.j, 0xFFAAFFFF);
        }
    }

    if(displayGeoCal) {

        const CameraModelBase& cam = *(inv->cam);

        double pi, pj;
        cam.getPrincipalPoint(pi, pj);

        // Draw a rectilinear grid centred on the principal point and distorted according to the camera projection model

        // Grid spacing in camera-frame (x,y) coordinates
        double dxy = 0.1;
        // Spacing between points on grid lines
        double ddxy = 0.01;

        // The edges of the image (coordinates of projected rays that are valid) are hard to determine in advance.
        // Need to draw outwards from the centre and stop when things go unphysical or outside of the image.

        // Cache the i,j coordinate of the previous grid line starting point; used to determine if the projection has become unphysical
        double i_tmp = pi;
        double j_tmp = pj;

        // Draw vertical grid lines on right half of image
        for(double x=0; ; x += dxy) {

            // New line:
            double i_start, j_start, i0, j0, i1, j1;
            Eigen::Vector3d r_cam(x,0,1);
            cam.projectVector(r_cam, i_start, j_start);

            // Stop drawing grid if we've moved outside of the image area
            if(i_start > signal->width) {
                break;
            }
            // Stop drawing grid if the projection has become unphysical
            if(i_start < i_tmp) {
                break;
            }

            i_tmp = i_start;

            // Draw vertical lines in lower right quater of image
            i0 = i_start;
            j0 = j_start;
            for(double y=ddxy; ; y+= ddxy) {

                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);

                // Stop drawing line if we've moved outside of the image area
                if(j1 > signal->height) {
                    break;
                }

                // Stop drawing line if the projection has become unphysical
                if(j1 < j0) {
                    break;
                }

                // Draw line
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);

                // Cache point for next iteration
                i0 = i1;
                j0 = j1;
            }

            // Draw vertical lines in upper right quarter of image
            i0 = i_start;
            j0 = j_start;
            for(double y=-ddxy; ; y-= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(j1 < 0 || j1 > j0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }
        }

        // Draw vertical grid lines on left half of image
        i_tmp = pi;
        for(double x=0; ; x -= dxy) {

            // New line:
            double i_start, j_start, i0, j0, i1, j1;
            Eigen::Vector3d r_cam(x,0,1);
            cam.projectVector(r_cam, i_start, j_start);

            if(i_start < 0 || i_start > i_tmp) {
                break;
            }

            i_tmp = i_start;

            // Draw vertical lines in lower left quater of image
            i0 = i_start;
            j0 = j_start;
            for(double y=ddxy; ; y+= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(j1 > signal->height || j1 < j0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }

            // Draw vertical lines in upper left quarter of image
            i0 = i_start;
            j0 = j_start;
            for(double y=-ddxy; ; y-= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(j1 < 0 || j1 > j0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }
        }

        // Draw horizontal grid lines on lower half of image
        j_tmp = pj;
        for(double y=0; ; y += dxy) {

            // New line:
            double i_start, j_start, i0, j0, i1, j1;
            Eigen::Vector3d r_cam(0,y,1);
            cam.projectVector(r_cam, i_start, j_start);

            if(j_start > signal->height || j_start < j_tmp) {
                break;
            }

            j_tmp = j_start;

            // Draw horizontal lines in lower right quarter of image
            i0 = i_start;
            j0 = j_start;
            for(double x=ddxy; ; x+= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(i1 > signal->width || i1 < i0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }

            // Draw horizontal lines in lower left quarter of image
            i0 = i_start;
            j0 = j_start;
            for(double x=-ddxy; ; x-= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(i1 < 0 || i1 > i0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }
        }

        // Draw horizontal grid lines on upper half of image
        j_tmp = pj;
        for(double y=0; ; y -= dxy) {

            // New line:
            double i_start, j_start, i0, j0, i1, j1;
            Eigen::Vector3d r_cam(0,y,1);
            cam.projectVector(r_cam, i_start, j_start);

            if(j_start < 0 || j_start > j_tmp) {
                break;
            }

            j_tmp = j_start;

            // Draw horizontal lines in upper right quarter of image
            i0 = i_start;
            j0 = j_start;
            for(double x=ddxy; ; x+= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(i1 > signal->width || i1 < i0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }

            // Draw horizontal lines in upper left quarter of image
            i0 = i_start;
            j0 = j_start;
            for(double x=-ddxy; ; x-= ddxy) {
                r_cam = Eigen::Vector3d(x,y,1);
                cam.projectVector(r_cam, i1, j1);
                if(i1 < 0 || i1 > i0) {
                    break;
                }
                RenderUtil::drawLine(signal->annotatedImage, signal->width, signal->height, i0, i1, j0, j1, 0x00FFFFFF);
                i0 = i1;
                j0 = j1;
            }
        }

        // Draw a crosshair at the image principal point
        RenderUtil::drawCrossHair(signal->annotatedImage, signal->width, signal->height, pi, pj, 10, 5, 0xFF0000FF);

    }

    signalImageViewer->newFrame(signal, true, true, true);
}
