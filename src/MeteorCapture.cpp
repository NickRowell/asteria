/**
 * Main application entry point.
 */


#include "Camera.h"

using namespace std;

int main() {
	Camera cam = Camera();

	vector< pair<int,string> > cams = cam.getCamerasList();

	for (unsigned i=0; i < cams.size(); i++) {
	    cout << "Cam " << cams[i].first << ": " << cams[i].second << endl;
	}

}


