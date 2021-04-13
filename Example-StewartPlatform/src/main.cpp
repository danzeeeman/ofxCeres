#include "pch_ofApp.h"

#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
#ifdef OF_TARGET_OPENGLES
	ofGLESWindowSettings settings;
	settings.glesVersion = 2;
#else
	ofGLWindowSettings settings;
	settings.setGLVersion(3, 2);
#endif
	settings.setSize(1920, 1200);
	ofCreateWindow(settings);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
