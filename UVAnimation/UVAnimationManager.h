#pragma once
#include "DirectManager.h"

class UVAnimationManager : public DirectManager
{
public:
    float rotationY = 0.0f;

    //폰트
    ID3DXFont* font1 = NULL;
    ID3DXFont* font2 = NULL;

    // 모델
    LPD3DXMESH disc = NULL;

    // 셰이더
    LPD3DXEFFECT uvAnimationShader = NULL;

    // 텍스처
    LPDIRECT3DTEXTURE9 texture = NULL;
    LPDIRECT3DTEXTURE9 stoneSM = NULL;

    //카메라 위치
    D3DXVECTOR4 worldCameraPosition;

    //빛의 위치
    D3DXVECTOR4 worldLightPosition;

    //빛의 색상
    D3DXVECTOR4 lightColor;

    //셰이더에 넘겨줄 전역변수
    float power = 20.0f; //정반사광 계산시 지수로 사용할 수 
    float waveHeight = 3.0f; //출렁이는 웨이브 높이
    float speed = 2.0f; //출렁이는 웨이브 속도
    float waveFrequency = 10.0f; //웨이브 빈도
    float uvSpeed = 0.25f; //웨이브의 이동 속도

    UVAnimationManager();
    void RenderScene() override;
    bool LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/);
};

