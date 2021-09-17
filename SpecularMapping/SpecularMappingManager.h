#pragma once
#include "DirectManager.h"

class SpecularMappingManager : public DirectManager
{
public:
    SpecularMappingManager();

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

    //빛의 색상
    D3DXVECTOR4 lightColor;

    void RenderScene() override;
    virtual bool LoadAssets(std::string shaderName = "", std::string modelName = "", std::string textureName = "");
};