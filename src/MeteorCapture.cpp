/**
 * Main application entry point.
 */


#include "V4L2Util.h"

using namespace std;

int main() {

	V4L2Util v4l2util;

	vector< pair<int,string> > cams = v4l2util.getCamerasList();

	for (unsigned i=0; i < cams.size(); i++) {
	    cout << "Cam " << cams[i].first << ": " << cams[i].second << endl;
	}

}


