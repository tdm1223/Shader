#pragma once
#include "DirectManager.h"

class NormalMappingManager : public DirectManager
{
public:
    NormalMappingManager();
    // 회전값
    float rotationY = 0.0f;

    //정반사광 계산시 지수로 사용할 수 
    float Pow;

    //카메라 위치
    D3DXVECTOR4 worldCameraPosition;

    //빛의 위치
    D3DXVECTOR4 worldLightPosition;

    // 텍스처
    LPDIRECT3DTEXTURE9 texture = NULL;
    LPDIRECT3DTEXTURE9 stoneSM = NULL;
    LPDIRECT3DTEXTURE9 stoneNM = NULL;

    //빛의 색상
    D3DXVECTOR4 lightColor;

    void RenderScene() override;
    bool LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/);
};