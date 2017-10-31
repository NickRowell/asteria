#ifndef ANALYSISINVENTORY_H
#define ANALYSISINVENTORY_H

#include "infra/imageuc.h"

#include <memory>

#include <QObject>

/**
 * @brief Instances of this class are used to encapsulate all the files and any other outputs
 * from the capture and analysis of individual video clips. It provides methods to load and save these
 * to disk and to delete all data for individual clips.
 */
class AnalysisInventory : public QObject {

    Q_OBJECT

public:

    AnalysisInventory();
    AnalysisInventory(const std::vector<std::shared_ptr<Imageuc>> &eventFrames);

    std::shared_ptr<Imageuc> peakHold;

    std::vector<std::shared_ptr<Imageuc>> eventFrames;

    std::vector<MeteorImageLocationMeasurement> locs;

public slots:

    /**
     * @brief Load an AnalysisInventory from disk. The contents of the AnalysisInventory
     * are divided into raw and processed types and are stored on disk in the following
     * file structure:
     *
     * \verbatim
       2017/
        |-08/
          |-13/
             |-2017-08-13T01:53:58.832Z/
                |-raw/
                |  |-file1
                |  |-file1
                |  |-fileN
                |-derived/
                   |-peakhold.pgm
      \endverbatim
     *
     * @param path
     *  Path to the directory node containing the AnalysisInventory data, e.g. in the example
     * above this would be the full path to the 2017-08-13T01:53:58.832Z/ directory.
     * @return
     *  A pointer to the loaded AnalysisInventory data.
     */
    static AnalysisInventory * loadFromDir(std::string path);

    void saveToDir(std::string topLevelPath);

    void deleteClip();

};

#endif // ANALYSISINVENTORY_H
