#include "pch_ofApp.h"
#include "MovingHead.h"

namespace DMX {
	//----------
	MovingHead::MovingHead(const Configuration& configuration)
	{
		// Apply configuration
		{
			this->parameters.pan.setMin(configuration.minimumPan);
			this->parameters.pan.setMax(configuration.maximumPan);
			this->parameters.tilt.setMin(configuration.minimumTilt);
			this->parameters.tilt.setMax(configuration.maximumTilt);
		}

		// Setup parameter group
		{
			this->parameters.setName("MovingHead");
			this->parameters.add(this->parameters.pan);
			this->parameters.add(this->parameters.tilt);
			this->parameters.add(this->parameters.dimmer);
			this->parameters.add(this->parameters.focus);
			this->parameters.add(this->parameters.iris);
		}

		RULR_SERIALIZE_LISTENERS;
		RULR_INSPECTOR_LISTENER;
	}

	//----------
	void
	MovingHead::drawWorld()
	{
		ofPushStyle();
		{
			auto transform = this->model->getTransform();

			//draw moving head body
			ofNoFill();
			vector<glm::vec3> transmissionsInObjectSpace;
			ofPushMatrix();
			{
				ofMultMatrix(transform);

				//draw hardware
				{
					ofSetColor(this->isBeingInspected() ? 220 : 100);
					ofDrawAxis(0.4f);

					// base
					ofDrawBox(glm::vec3(0, -0.35, 0), 0.5, 0.1, 0.4);

					auto panTiltIdeal = this->model->panTiltSignalToIdeal(this->getCurrentPanTilt());

					// arrow
					static ofMesh arrow;
					if (arrow.getNumVertices() == 0) {
						arrow.addVertices(
							{
								{ -0.05, -0.3, 0.3 },
								{ +0.05, -0.3, 0.3 },
								{ 0, -0.3, 0.4 }
							});
					}
					arrow.drawWireframe();

					// Axis 1
					ofPushMatrix();
					{
						ofRotateDeg(-panTiltIdeal.x, 0, 1, 0);

						ofDrawBox({ -0.15, -0.3 + 0.35f / 2.0f, 0 }, 0.075, 0.35, 0.2);
						ofDrawBox({ +0.15, -0.3 + 0.35f / 2.0f, 0 }, 0.075, 0.35, 0.2);

						// Axis 2
						ofPushMatrix();
						{
							ofRotateDeg(panTiltIdeal.y, 1, 0, 0);

							// head
							ofPushMatrix();
							{
								ofSetSphereResolution(6);
								ofScale(0.1, 0.2, 0.1);
								ofDrawSphere(1.0);
							}
							ofPopMatrix();

							// beam
							ofPushMatrix();
							{
								ofTranslate(0.0f, 0.05 + 0.5f / 2.0f, 0.0f);
								ofSetColor(this->isBeingInspected() ? 255 : 100);
								ofDrawCylinder(0.02, 0.5f);
							}
							ofPopMatrix();
						}
						ofPopMatrix();
					}
					ofPopMatrix();
				}
			}
			ofPopMatrix();
		}
		ofPopStyle();

		this->solver->drawWorld();
	}

	//----------
	void
	MovingHead::serialize(nlohmann::json& json)
	{
		json << this->parameters.pan;
		json << this->parameters.tilt;
		json << this->parameters.dimmer;
		json << this->parameters.focus;
		json << this->parameters.iris;

		this->model->notifySerialize(json["model"]);
		this->solver->notifySerialize(json["solver"]);
	}

	//----------
	void
	MovingHead::deserialize(const nlohmann::json& json)
	{
		json >> this->parameters.pan;
		json >> this->parameters.tilt;
		json >> this->parameters.dimmer;
		json >> this->parameters.focus;
		json >> this->parameters.iris;

		if (json.contains("model")) {
			this->model->notifyDeserialize(json["model"]);
		}
		if (json.contains("solver")) {
			this->solver->notifyDeserialize(json["solver"]);
		}
	}

	//----------
	void
	MovingHead::populateInspector(ofxCvGui::InspectArguments& args)
	{
		auto inspector = args.inspector;
		inspector->addParameterGroup(this->parameters);
		inspector->addButton("Home", [this]() {
			this->parameters.pan.set(0);
			this->parameters.tilt.set(0);
			});
		inspector->addSubMenu("Model", this->model);
		inspector->addSubMenu("Solver", this->solver);
	}

	//---------
	glm::vec2
	MovingHead::getCurrentPanTilt() const
	{
		return glm::vec2(
			this->parameters.pan.get()
			, this->parameters.tilt.get()
		);
	}

	//---------
	void
	MovingHead::navigateToWorldTarget(const glm::vec3& world)
	{
		// Navigate pan-tilt values
		auto panTiltAngles = this->model->getPanTiltForWorldTarget(world, this->getCurrentPanTilt());
		this->parameters.pan.set(panTiltAngles.x);
		this->parameters.tilt.set(panTiltAngles.y);
	

		// Navigate the focus (NOTE : hardcoded for Robe Pointe)
		{
			//check 'Fit focus values.ipynb'
			auto distance = glm::distance(world, this->model->getPosition());

			if (distance < 1.5f) {
				// near range
				this->parameters.focus.set(1.0f);
			}
			else {
				auto inverseDistance = 1.0f / distance;
				auto dmxCoarseFloat =
					-77.16738954 * inverseDistance * inverseDistance * inverseDistance
					+ -45.98512483 * inverseDistance * inverseDistance
					+ 403.21323162 * inverseDistance
					+ 29.67284246;
				this->parameters.focus.set(dmxCoarseFloat / 256.0f);
			}
		}
	}

	//---------
	shared_ptr<Calibration::Model>
	MovingHead::getModel()
	{
		return this->model;
	}

	//---------
	shared_ptr<Calibration::Solver>
	MovingHead::getSolver()
	{
		return this->solver;
	}
}