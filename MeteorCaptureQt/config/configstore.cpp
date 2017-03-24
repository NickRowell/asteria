#include "config/configstore.h"
#include "configparameterfamily.h"
#include "config/systemparameters.h"
#include "config/stationparameters.h"
#include "config/cameraparameters.h"
#include "infra/meteorcapturestate.h"

ConfigStore::ConfigStore(MeteorCaptureState *state) {
    numFamilies = 3;
    families = new ConfigParameterFamily*[numFamilies];
    families[0] = new SystemParameters(state);
    families[1] = new StationParameters(state);
    families[2] = new CameraParameters(state);
}
