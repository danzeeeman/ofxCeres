#include "pch_ofApp.h"

#include "ofApp.h"
#include "ofxCeres.h"

#define COUNT 100
#define NOISE 0.01

//--------------------------------------------------------------
void ofApp::setup() {
	this->gui.init();

	this->movingHeads.emplace("1", make_shared<MovingHead>());
	this->movingHeads.emplace("2", make_shared<MovingHead>());

	{
		auto strip = ofxCvGui::Panels::Groups::makeStrip();
		{
			strip->setCellSizes({ -1, 500 });
			this->gui.add(strip);
		}

		{
			this->worldPanel = ofxCvGui::Panels::makeWorld();
			this->worldPanel->onDrawWorld += [this](ofCamera &) {
				this->drawWorld();
			};
			this->worldPanel->setGridEnabled(false);
			strip->add(this->worldPanel);
		}

		{
			this->widgetsPanel = ofxCvGui::Panels::makeWidgets();
			this->repopulateWidgets();
			strip->add(widgetsPanel);
		}
	}

	this->load();
}

//--------------------------------------------------------------
void ofApp::update(){
	this->solvedTransform = ofxCeres::VectorMath::createTransform(this->parameters.translation.get(), this->parameters.rotation.get());
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::drawWorld() {
	if (this->parameters.drawGrid) {
		ofPushStyle();
		{
			ofSetColor(150);

			ofPushMatrix();
			{
				ofRotateDeg(180, 0, 1, 0);
				ofRotateDeg(90, 0, 0, 1);
				ofDrawGridPlane(1.0f, 20, true);
			}
			ofPopMatrix();
		}
		ofPopStyle();
	}

	for (auto & movingHead : this->movingHeads) {
		movingHead.second->drawWorld(movingHead.first == this->selection);
	}
}

//--------------------------------------------------------------
void ofApp::repopulateWidgets() {
	this->widgetsPanel->clear();

	static shared_ptr<ofxCvGui::Widgets::MultipleChoice> selector;
	if (!selector) {
		selector = make_shared<ofxCvGui::Widgets::MultipleChoice>("Moving head");
		{
			for (auto & it : this->movingHeads) {
				selector->addOption(it.first);
			}
		}

		selector->onValueChange += [this](int) {
			this->selection = selector->getSelection();
			this->repopulateWidgets();
		};

		this->selection = selector->getSelection();
	}

	this->widgetsPanel->addFps();
	this->widgetsPanel->addButton("Save all", [this]() {
		this->save();
	});
	this->widgetsPanel->add(selector);

	this->widgetsPanel->addSpacer();
	
	this->movingHeads[this->selection]->populateWidgets(this->widgetsPanel);
}

//--------------------------------------------------------------
void ofApp::load() {
	for (const auto & it : this->movingHeads) {
		ofFile file;
		file.open(it.first + ".json");
		if (file.exists()) {
			nlohmann::json json;
			file >> json;
			it.second->deserialize(json);
		}
	}
}

//--------------------------------------------------------------
void ofApp::save() {
	for (const auto & it : this->movingHeads) {
		nlohmann::json json;
		it.second->serialize(json);

		ofFile file;
		file.open(it.first + ".json", ofFile::Mode::WriteOnly);
		file << std::setw(4) << json;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	this->movingHeads[this->selection]->setWorldCursorPosition(this->worldPanel->getCamera().getCursorWorld());
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}