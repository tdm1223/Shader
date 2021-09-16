#pragma once
#include"DirectManager.h"

class LightingManager : public DirectManager
{
public:
    LightingManager();

    // 회전값
    float rotationY;

    //정반사광 계산시 지수로 사용할 수 
    float Pow;

    //카메라 위치
    D3DXVECTOR4 worldCameraPosition;

    //빛의 위치
    D3DXVECTOR4 worldLightPosition;

    void RenderScene() override;
};