#!/bin/bash

# Dependencies for robotArmRepo
#make sure you are in the robotArmRepo project when you run this script

cd ../../addons

PREFIX="git clone http://github.com/"

${PREFIX}elliotwoods/ofxAssets
${PREFIX}elliotwoods/ofxCvGui
${PREFIX}elliotwoods/ofxGrabCam
${PREFIX}elliotwoods/ofxLiquidEvent
${PREFIX}elliotwoods/ofxSingleton
${PREFIX}elliotwoods/ofxTextInputField
${PREFIX}elliotwoods/ofxAudioDecoder
cd ../addons/ofxRobotArm