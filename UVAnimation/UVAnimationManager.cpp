#include "UVAnimationManager.h"

UVAnimationManager::UVAnimationManager()
{
    worldCameraPosition = D3DXVECTOR4(200.0f, 100.0f, -200.0f, 1.0f);
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
    lightColor = D3DXVECTOR4(0.7f, 0.7f, 1.0f, 1.0f);
}

void UVAnimationManager::RenderScene()
{
    // 월드행렬을 만든다.
    D3DXMATRIXA16 worldMatrix;
    D3DXMatrixRotationY(&worldMatrix, rotationY);

    // 뷰 행렬을 만든다.
    D3DXMATRIXA16 viewMatrix;
    const D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
    const D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&viewMatrix, &eye, &lookAt, &up);

    // 투영행렬을 만든다.
    D3DXMATRIXA16 projectionMatrix;
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, Util::FOV, Util::ASPECT_RATIO, Util::NEAR_PLANE, Util::FAR_PLANE);

    // 뷰 X 투영행렬을 만든다.
    D3DXMATRIXA16 viewProjectionMatrix;
    D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);

    // 정점 셰이더 전역변수들을 설정
    shader->SetMatrix("worldMatrix", &worldMatrix);
    shader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);
    shader->SetVector("worldLightPosition", &worldLightPosition);
    shader->SetVector("worldCameraPosition", &worldCameraPosition);

    // 시스템 시간을 구한다.
    const ULONGLONG tick = GetTickCount64();
    shader->SetFloat("time", tick / 1000.0f);
    shader->SetFloat("waveHeight", waveHeight);
    shader->SetFloat("speed", speed);
    shader->SetFloat("waveFrequency", waveFrequency);
    shader->SetFloat("uvSpeed", uvSpeed);

    // 픽셀 셰이더 전역변수들을 설정
    shader->SetFloat("power", power);
    shader->SetTexture("diffuseMap_Tex", texture);
    shader->SetTexture("specularMap_Tex", stoneSM);
    shader->SetVector("lightColor", &lightColor);

    UINT numPasses = 0;
    shader->Begin(&numPasses, NULL);
    {
        for (UINT i = 0; i < numPasses; i++)
        {
            shader->BeginPass(i);
            {
                model->DrawSubset(0);
            }
            shader->EndPass();
        }
    }
    shader->End();
}

bool UVAnimationManager::LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
{
    // DiffuseMap 로딩
    if (!textureName.empty())
    {
        texture = LoadTexture(textureName);
        if (!texture)
        {
            return false;
        }
    }

    // SpecularMap 로딩
    stoneSM = LoadTexture("Fieldstone_SM.tga");
    if (!stoneSM)
    {
        return false;
    }

    // 셰이더 로딩
    if (!shaderName.empty())
    {
        shader = LoadShader(shaderName);
        if (!shader)
        {
            return false;
        }
    }

    // 모델 로딩
    if (!modelName.empty())
    {
        model = LoadModel(modelName);
        if (!model)
        {
            return false;
        }
    }

    return true;
}
