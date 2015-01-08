#pragma once
#include "SceneObject.h"
#include <memory>

#include <DirectXMath.h>
#include "GameAwaitablePromise.h"
#include "Animation.h"

// this is a simple animated scene object, exposing fadein/fadeout animations as awaitable coroutines
class AnimatedText :
	public SceneObject
{
private:
	class Resources;
	std::unique_ptr<Resources> _resources;
	DirectX::XMFLOAT4X4 _transform;
	float _opacity;
	std::unique_ptr<Animation<float>> _opacityAnim;
	std::unique_ptr<Animation<DirectX::XMFLOAT4X4A>> _transformAnim;
public:
	AnimatedText();
	virtual ~AnimatedText();

	// Inherited via SceneObject
	virtual void loadDeviceDependentResources(ID3D11Device * device) override;
	virtual void updateState(const GameClock & clock) override;
	virtual void draw(ID3D11DeviceContext * deviceContext, ID3D11RenderTargetView * rtv) override;

	// animations can be considered as coroutines that will eventually complete
	GameAwaitableUniquePromise<void>& fadeIn();
	GameAwaitableUniquePromise<void>& fadeOut();
};

