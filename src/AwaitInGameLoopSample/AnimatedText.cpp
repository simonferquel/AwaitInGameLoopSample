#include "stdafx.h"
#include "AnimatedText.h"
#include <wrl.h>
#include "dx_exception.h"
#include <Windows.h>
#include <cstdint>
#include "../DirectXTK/Inc/WICTextureLoader.h"
#include "../DirectXTK/Inc/CommonStates.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std::chrono;

struct Buffer {
	std::unique_ptr<std::uint8_t[]> data;
	std::uint32_t size;
};

class SafeHandle {
private:
	HANDLE _handle;
public:
	HANDLE get()const { return _handle; }
	explicit SafeHandle(HANDLE h) : _handle(h) {
		if (h == INVALID_HANDLE_VALUE) {
			throw std::bad_alloc();
		}

	}
	~SafeHandle() {
		CloseHandle(_handle);
	}
};

Buffer readFileToMemory(const wchar_t* filePath) {
	SafeHandle file(CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
	LARGE_INTEGER size;
	GetFileSizeEx(file.get(), &size);
	std::unique_ptr<std::uint8_t[]> data(new std::uint8_t[size.QuadPart]);
	DWORD read;
	if (!ReadFile(file.get(), data.get(), size.QuadPart, &read, nullptr)) {
		throw std::exception("cannot read file");
	}
	return Buffer{ std::move(data), (uint32_t)size.QuadPart };
}

struct QuadVertex {
	DirectX::XMFLOAT2 pos;
	DirectX::XMFLOAT2 uv;
	QuadVertex() {}
	QuadVertex(const DirectX::XMFLOAT2& pos_, const DirectX::XMFLOAT2& uv_) : pos(pos_), uv(uv_) {}
};





class AnimatedText::Resources {
public:
	ComPtr<ID3D11Buffer> _quadVertices;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11InputLayout> _inputLayout;
	ComPtr<ID3D11Buffer> _transformsBuffer;

	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11Resource> _texture;
	ComPtr<ID3D11ShaderResourceView> _textureRV;
	ComPtr<ID3D11DepthStencilState> _depthStencilState;
	ComPtr<ID3D11BlendState> _blendState;
public:
	void reset(ID3D11Device* device) {
		QuadVertex quad[] = {
			QuadVertex(XMFLOAT2(-1, +1), XMFLOAT2(0,0)),
			QuadVertex(XMFLOAT2(1, +1), XMFLOAT2(1,0)),
			QuadVertex(XMFLOAT2(-1, -1), XMFLOAT2(0,1)),

			QuadVertex(XMFLOAT2(-1, -1), XMFLOAT2(0,1)),
			QuadVertex(XMFLOAT2(1, +1), XMFLOAT2(1,0)),
			QuadVertex(XMFLOAT2(1, -1), XMFLOAT2(1,1))
		};
		D3D11_SUBRESOURCE_DATA vertexData;
		vertexData.pSysMem = quad;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;
		throwIfFailed(device->CreateBuffer(&CD3D11_BUFFER_DESC(sizeof(quad), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE), &vertexData, &_quadVertices));


		Buffer b = readFileToMemory(L"DrawQuadVS.cso");
		throwIfFailed(device->CreateVertexShader(b.data.get(), b.size, nullptr, &_vertexShader));
		D3D11_INPUT_ELEMENT_DESC inputDesc[]{
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(XMFLOAT2), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		throwIfFailed(device->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), b.data.get(), b.size, &_inputLayout));

		throwIfFailed(device->CreateBuffer(&CD3D11_BUFFER_DESC(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), nullptr, &_transformsBuffer));

		b = readFileToMemory(L"DrawQuadPS.cso");
		throwIfFailed(device->CreatePixelShader(b.data.get(), b.size, nullptr, &_pixelShader));

		b = readFileToMemory(L"Texture.png");
		throwIfFailed(DirectX::CreateWICTextureFromMemory(device, b.data.get(), b.size, &_texture, &_textureRV));

		_depthStencilState = DirectX::CommonStates(device).DepthNone();
		_blendState = DirectX::CommonStates(device).Additive();
	}
};

AnimatedText::AnimatedText() : _resources(std::make_unique<Resources>()), _opacity(0)
{
	XMStoreFloat4x4(&_transform, XMMatrixScaling(0, 0, 1.0f));
}


AnimatedText::~AnimatedText()
{
}

void AnimatedText::loadDeviceDependentResources(ID3D11Device * device)
{
	_resources->reset(device);
}

void AnimatedText::updateState(const GameClock & clock)
{
	if (_opacityAnim) {
		bool animEnded;
		_opacity = _opacityAnim->update(clock.lastFrameDuration(), animEnded);
	}
	if (_transformAnim) {
		bool animEnded;
		_transform = _transformAnim->update(clock.lastFrameDuration(), animEnded);
	}
}

void AnimatedText::draw(ID3D11DeviceContext * deviceContext, ID3D11RenderTargetView * rtv)
{
	D3D11_MAPPED_SUBRESOURCE transformsMappedResource;
	throwIfFailed(deviceContext->Map(_resources->_transformsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &transformsMappedResource));
	XMFLOAT4X4* pOut = reinterpret_cast<XMFLOAT4X4*>(transformsMappedResource.pData);
	*pOut = _transform;
	pOut->_41 = _opacity;
	deviceContext->Unmap(_resources->_transformsBuffer.Get(), 0);

	UINT stride = sizeof(QuadVertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, _resources->_quadVertices.GetAddressOf(), &stride, &offset);
	deviceContext->IASetInputLayout(_resources->_inputLayout.Get());
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->VSSetShader(_resources->_vertexShader.Get(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(0, 1, _resources->_transformsBuffer.GetAddressOf());

	deviceContext->PSSetShader(_resources->_pixelShader.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, _resources->_textureRV.GetAddressOf());

	deviceContext->OMSetBlendState(_resources->_blendState.Get(), nullptr, 0xffffffff);
	deviceContext->OMSetDepthStencilState(_resources->_depthStencilState.Get(), 0);

	deviceContext->Draw(6, 0);
}

GameAwaitablePromise<void> AnimatedText::fadeIn()
{
	_opacityAnim.reset(new Animation<float>(duration_cast<steady_clock::duration>(1s), _opacity, 1.0f, Interoplate_Linear()));
	XMFLOAT4X4A transFrom(&_transform.m[0][0]);
	XMFLOAT4X4A transTo;
	XMStoreFloat4x4A(&transTo, XMMatrixIdentity());
	_transformAnim.reset(new Animation<DirectX::XMFLOAT4X4A>(duration_cast<steady_clock::duration>(1s), transFrom, transTo, Interoplate_Linear()));
	return _transformAnim->getPromise();
}

GameAwaitablePromise<void> AnimatedText::fadeOut()
{
	_opacityAnim.reset(new Animation<float>(duration_cast<steady_clock::duration>(1s), _opacity, 0.0f, Interoplate_Linear()));
	XMFLOAT4X4A transFrom(&_transform.m[0][0]);
	XMFLOAT4X4A transTo;
	XMStoreFloat4x4A(&transTo, XMMatrixScaling(0,0,1.0f));
	_transformAnim.reset(new Animation<DirectX::XMFLOAT4X4A>(duration_cast<steady_clock::duration>(1s), transFrom, transTo, Interoplate_Linear()));
	return _transformAnim->getPromise();
}
