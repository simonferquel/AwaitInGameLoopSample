#pragma once
#include "GameClock.h"
#include <d3d11_2.h>

// scene object that must be updated and drawn at each frame
class SceneObject
{
public:
	SceneObject();
	virtual ~SceneObject() = default;
	virtual void loadDeviceDependentResources(ID3D11Device* device) = 0;
	virtual void updateState(const GameClock& clock) = 0;
	virtual void draw(ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* rtv) = 0;
};

