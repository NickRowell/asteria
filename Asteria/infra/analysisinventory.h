#ifndef ANALYSISINVENTORY_H
#define ANALYSISINVENTORY_H

#include "infra/imageuc.h"

#include <dirent.h>
#include <memory>

#include <QObject>

/**
 * Instances of this class are used to encapsulate all the files and any other outputs
 * from the analysis of single video clips. It provides methods to load and save these
 * to disk and to delete all data for individual clips.
 *
 * @brief The AnalysisInventory class
 */
class AnalysisInventory : public QObject {

    Q_OBJECT

public:
    AnalysisInventory();

    std::shared_ptr<Imageuc> peakHold;

    std::vector<std::shared_ptr<Imageuc>> eventFrames;

    std::vector<MeteorImageLocationMeasurement> locs;

public slots:

    static AnalysisInventory * loadFromDir(std::string path);

    void saveToDir(std::string path);

    void deleteClip();

};

#endif // ANALYSISINVENTORY_H
