#pragma once
#include "DirectManager.h"

class ColorConversionManager : public DirectManager
{
public:
    ColorConversionManager();

    // 셰이더
    LPD3DXEFFECT noEffect = NULL;
    LPD3DXEFFECT grayScale = NULL;
    LPD3DXEFFECT sepia = NULL;

    // 텍스처
    LPDIRECT3DTEXTURE9 stoneSM = NULL;
    LPDIRECT3DTEXTURE9 stoneNM = NULL;
    LPDIRECT3DCUBETEXTURE9	snowENV = NULL;

    // 회전값
    float rotationY = 0.0f;

    // 빛의 위치
    D3DXVECTOR4 worldLightPosition;

    // 빛의 색상
    D3DXVECTOR4 lightColor;

    // 카메라 위치
    D3DXVECTOR4 worldCameraPosition;

    // 화면을 가득 채우는 사각형
    LPDIRECT3DVERTEXDECLARATION9 fullScreenQuad = NULL;
    LPDIRECT3DVERTEXBUFFER9 fullScreenQuadVB = NULL;
    LPDIRECT3DINDEXBUFFER9 fullScreenQuadIB = NULL;

    // 장면 렌더타깃
    LPDIRECT3DTEXTURE9 sceneRenderTarget = NULL;

    int postProcessIndex = 0;
    bool Init(HWND hWnd, std::string shaderName = "", std::string modelName = "", std::string textureName = "") override;
    void RenderScene() override;
    void InitFullScreenQuad() noexcept;
    bool LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/);
};