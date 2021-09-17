#pragma once
#include "DirectManager.h"

class ShadowSpecularMappingManager : public DirectManager
{
public:
    ShadowSpecularMappingManager();
    void RenderScene() override;
    bool LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/) override;
    bool Init(HWND hWnd, std::string shaderName = "", std::string modelName = "", std::string textureName = "") override;

    // 회전값
    float rotationY;

    // 모델
    LPD3DXMESH torus = nullptr;

    // 셰이더
    LPD3DXEFFECT createShadowShader = nullptr;
    LPD3DXEFFECT discShader = nullptr;

    // 텍스처
    LPDIRECT3DTEXTURE9 texture = nullptr;
    LPDIRECT3DTEXTURE9 stoneSM = nullptr;

    // 카메라 위치
    D3DXVECTOR4 worldCameraPosition;

    // 빛의 위치
    D3DXVECTOR4 worldLightPosition;

    // 빛의 색상
    D3DXVECTOR4 lightColor;

    // disc 색상
    D3DXVECTOR4 discColor;

    LPDIRECT3DTEXTURE9 shadowRenderTarget = nullptr;
    LPDIRECT3DSURFACE9 shadowDepthStencil = nullptr;

    LPDIRECT3DSURFACE9 shadowSurface;

    // 하드웨어 깊이 버퍼
    LPDIRECT3DSURFACE9 depthStencilBuffer;

    // 하드웨어 백버퍼
    LPDIRECT3DSURFACE9 backBuffer;

    const int shadowMapSize = 2048;
};

