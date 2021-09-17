#include "SpecularMappingManager.h"

SpecularMappingManager::SpecularMappingManager()
{
    lightColor = D3DXVECTOR4(0.7f, 0.7f, 1.0f, 1.0f);
    rotationY = 0.0f;
    Pow = 20.0f;
    worldCameraPosition = D3DXVECTOR4(200.0f, 0.0f, -200.0f, 1.0f);
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
}

// 3D 물체를 그린다.
void SpecularMappingManager::RenderScene()
{
    // 프레임마다 0.4도씩 회전을 시킨다.
    rotationY += 0.4f * Util::PI / 180.0f;
    if (rotationY > 2 * Util::PI)
    {
        rotationY -= 2 * Util::PI;
    }

    //월드 행렬을 만든다.
    D3DXMATRIXA16 worldMatrix;
    D3DXMatrixRotationY(&worldMatrix, rotationY);

    // 뷰 행렬을 만든다.
    D3DXMATRIXA16 viewMatrix;
    const D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
    const D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&viewMatrix, &eye, &lookAt, &up);

    // 투영 행렬을 만든다.
    D3DXMATRIXA16 projectionMatrix;
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, Util::FOV, Util::ASPECT_RATIO, Util::NEAR_PLANE, Util::FAR_PLANE);

    //뷰-투영 행렬을 만든다.
    D3DXMATRIXA16 viewProjectionMatrix;
    D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);

    // 셰이더 전역변수들을 설정(정점)
    shader->SetMatrix("worldMatrix", &worldMatrix);
    shader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);
    shader->SetVector("worldCameraPosition", &worldCameraPosition);
    shader->SetVector("worldLightPosition", &worldLightPosition);

    // 셰이더 전역변수들을 설정(픽셀)
    shader->SetFloat("power", Pow);
    shader->SetVector("lightColor", &lightColor);
    shader->SetTexture("diffuseMap_Tex", texture);
    shader->SetTexture("specularMap_Tex", stoneSM);

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

bool SpecularMappingManager::LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
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
