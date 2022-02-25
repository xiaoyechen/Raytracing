#pragma once

#include <vector>
#include "Camera.h"
#include "GenericObject.h"
#include "Light.h"

class Scene
{
public:
	Scene() {}
	~Scene() { mObjects.clear(); }

	void SetCamera(Camera* cam) { mCamera = cam; }
	void SetPointLight(PointLight* light) { mLightPoint = light; }
	void SetInfiniteLight(DirectedLight* light) { mLightInfinite = light; }
	void AddObject(GenericObject* obj) { mObjects.push_back(obj); }
	GenericObject* GetObject(unsigned idx) { return idx < mObjects.size() ? mObjects[idx] : mObjects.back(); }

	Camera* GetCamera() const { return mCamera; }
	Camera* GetCamera() { return mCamera; }
	Light* GetPointLight() const { return mLightPoint; }
	Light* GetInfiniteLight() const { return mLightInfinite; }
	std::vector<GenericObject*>& GetObjects() { return mObjects; }

	double GetNear() const { return mCamera->GetNear(); }
	double GetNearHeight() const { return mCamera->GetNearHeight(); }
	double GetNearWidth() const { return mCamera->GetNearWidth(); }

private:
	Camera* mCamera;
	Light* mLightPoint;
	Light* mLightInfinite;
	std::vector<GenericObject*> mObjects;
};