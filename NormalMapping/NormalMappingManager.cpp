#include "NormalMappingManager.h"

NormalMappingManager::NormalMappingManager()
{
    lightColor = D3DXVECTOR4(0.7f, 0.7f, 1.0f, 1.0f);
    rotationY = 0.0f;
    Pow = 20.0f;
    worldCameraPosition = D3DXVECTOR4(0.0f, 0.0f, -200.0f, 1.0f);
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
}

void NormalMappingManager::RenderScene()
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

    // Y축 기준 회전행렬
    D3DXMATRIX cameraRotationMatrix;
    const D3DXVECTOR3 v1(0.0f, 1.0f, 0.0f);
    D3DXMatrixRotationAxis(&cameraRotationMatrix, &v1, rotationY);

    // 월드 X 뷰 X 투영 행렬을 만든다.
    D3DXMATRIXA16 worldViewMatrix;
    D3DXMATRIXA16 worldViewProjectionMatrix;
    D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
    D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

    // 정점 셰이더 전역변수 설정
    shader->SetMatrix("worldMatrix", &worldMatrix);
    shader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
    shader->SetVector("worldLightPosition", &worldLightPosition);
    shader->SetVector("worldCameraPosition", &worldCameraPosition);

    // 픽셀 셰이더 전역변수 설정
    shader->SetTexture("diffuseMap_Tex", texture);
    shader->SetTexture("specularMap_Tex", stoneSM);
    shader->SetTexture("normalMap_Tex", stoneNM);
    shader->SetVector("lightColor", &lightColor);
    shader->SetFloat("power", Pow);

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

bool NormalMappingManager::LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
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

    // NormalMap 로딩
    stoneNM = LoadTexture("Fieldstone_NM.tga");
    if (!stoneNM)
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
