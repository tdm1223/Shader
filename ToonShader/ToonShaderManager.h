#pragma once
#include "DirectManager.h"
class ToonShaderManager : public DirectManager
{
public:
    ToonShaderManager();

    // 회전값
    float rotationY;

    //표면의 색상
    D3DXVECTOR4 surfaceColor;

    //툰셰이더 변수
    float toonShaderParam;

    //빛의 위치
    D3DXVECTOR4 worldLightPosition;

    void RenderScene() override;
};

