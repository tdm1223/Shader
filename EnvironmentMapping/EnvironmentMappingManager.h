#pragma once
#include "DirectManager.h"
class EnvironmentMappingManager : public DirectManager
{
public:
    EnvironmentMappingManager();

    // 회전값
    float rotationY;

    //카메라 위치
    D3DXVECTOR4 worldCameraPosition;

    //빛의 위치
    D3DXVECTOR4 worldLightPosition;

    //빛의 색상
    D3DXVECTOR4 lightColor;

    // 텍스처
    LPDIRECT3DTEXTURE9 texture = NULL;
    LPDIRECT3DTEXTURE9 stoneSM = NULL;
    LPDIRECT3DTEXTURE9 stoneNM = NULL;
    LPDIRECT3DCUBETEXTURE9 snowENV = NULL;

    void RenderScene() override;
    bool LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/);
};