#include "pch_ofApp.h"
#include "Mesh.h"

//---------
Mesh::Mesh()
{
	RULR_SERIALIZE_LISTENERS;
	RULR_INSPECTOR_LISTENER;
}

//---------
string
Mesh::getTypeName() const
{
	return "Mesh";
}

//---------
void
Mesh::update()
{
	if (this->loadedPath != this->parameters.filename) {
		try {
			this->model.loadModel(this->parameters.filename.get().string());
			this->model.setScaleNormalization(false);
			this->model.disableMaterials();
			if (!this->model.hasMeshes()) {
				throw(Exception("File does not contain any meshes"));
				this->model.clear();
			}
			this->loadedPath = this->parameters.filename;
		}
		catch (Exception e) {
			ofLogError("Mesh") << "Couldn't load mesh " << this->parameters.filename.get();
			this->parameters.filename.set(filesystem::path());
		}
	}
}

//---------
void
Mesh::drawWorld()
{
	this->parameters.enableMaterials.get()
		? this->model.enableMaterials()
		: this->model.disableMaterials();

	ofPushMatrix();
	{
		ofMultMatrix(this->getTransform());

		ofPushStyle();
		{
			ofSetColor(255);

			switch (this->drawStyle.get().get()) {
			case DrawStyle::Mix:
			{
				glPushAttrib(GL_POLYGON_BIT);
				{
					glEnable(GL_POLYGON_OFFSET_FILL);
					glPolygonOffset(1.0, 1.0);
					this->model.drawWireframe();
					ofSetColor(100);
					this->model.drawFaces();
				}
				glPopAttrib();
				break;
			}
			case DrawStyle::Fill:
			{
				this->model.drawFaces();
				break;
			}
			case DrawStyle::Wireframe:
			{
				this->model.drawWireframe();
				break;
			}
			default:
				break;
			}
		}
		ofPopStyle();
	}
	ofPopMatrix();
}

//---------
void
Mesh::serialize(nlohmann::json& json)
{
	Data::serialize(json, this->parameters);
	Data::serializeEnum(json, this->drawStyle);
}

//---------
void
Mesh::deserialize(const nlohmann::json& json)
{
	Data::deserialize(json, this->parameters);
	Data::deserializeEnum(json, this->drawStyle);
}

//---------
void
Mesh::populateInspector(ofxCvGui::InspectArguments& args)
{
	auto inspector = args.inspector;

	inspector->addParameterGroup(this->parameters);
	{
		auto widget = inspector->addMultipleChoice("Draw style");
		widget->entangleManagedEnum(this->drawStyle);
	}

	inspector->addTitle("Model info", ofxCvGui::Widgets::Title::H3);
	inspector->addLiveValue<glm::vec3>("Scene Min", [this]() {
		return this->model.getSceneMin();
		});
	inspector->addLiveValue<glm::vec3>("Scene Max", [this]() {
		return this->model.getSceneMax();
		});
	inspector->addLiveValue<size_t>("Mesh count", [this]() {
		return this->model.getMeshCount();
		});
	inspector->addButton("Reload", [this]() {
		this->model.clear();
		this->loadedPath = "";
		});

}

//---------
bool
Mesh::isLoaded()
{
	return !this->loadedPath.empty();
}

//---------
glm::vec3
Mesh::getMinWorld()
{
	return ofxCeres::VectorMath::applyTransform(this->getTransform(), (glm::vec3) this->model.getSceneMin());
}

//---------
glm::vec3
Mesh::getMaxWorld()
{
	return ofxCeres::VectorMath::applyTransform(this->getTransform(), (glm::vec3)this->model.getSceneMax());
}

//---------
glm::mat4
Mesh::getTransform() const
{
	return glm::scale(this->parameters.scale.get() * glm::vec3(1, 1, 1))
		* (glm::mat4) ofxCeres::VectorMath::eulerToQuat(glm::vec3{
				this->parameters.rotation.x.get()
				, this->parameters.rotation.y.get()
				, this->parameters.rotation.z.get()
			} *DEG_TO_RAD);
}

//---------
glm::vec3
Mesh::getPointClosestTo(const glm::vec3& samplePosition, float maxDistance)
{
	auto meshCount = this->model.getMeshCount();
	
	// This value will be used if no close vertex is found
	auto closestVertex = samplePosition;
	auto minDistance = numeric_limits<float>::max();

	for (size_t i = 0; i < meshCount; i++) {
		auto& meshHelper = this->model.getMeshHelper(i);
		auto& vertices = meshHelper.cachedMesh.getVertices();
		for (const auto& rawVertex : vertices) {
			auto transform = (glm::mat4)meshHelper.matrix
				* (glm::mat4)this->model.getModelMatrix()
				* this->getTransform();

			auto vertexPosition = ofxCeres::VectorMath::applyTransform(transform, rawVertex);
			auto distance = glm::distance(vertexPosition, samplePosition);
			if (distance > maxDistance) {
				continue;
			}

			if (distance < minDistance) {
				minDistance = distance;
				closestVertex = vertexPosition;
			}
		}
	}

	return closestVertex;
}