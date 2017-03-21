#include "config/configstore.h"
#include "configparameterfamily.h"
#include "config/systemparameters.h"
#include "config/stationparameters.h"
#include "config/cameraparameters.h"

ConfigStore::ConfigStore() {
    numFamilies = 3;
    families = new ConfigParameterFamily*[numFamilies];
    families[0] = new SystemParameters();
    families[1] = new StationParameters();
    families[2] = new CameraParameters();
}
