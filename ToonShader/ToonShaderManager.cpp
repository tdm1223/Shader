#include "ToonShaderManager.h"

ToonShaderManager::ToonShaderManager()
{
    rotationY = 0.0f;
    toonShaderParam = 5.0f;
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
    surfaceColor = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.0f);
}

// 3D 물체등을 그린다.
void ToonShaderManager::RenderScene()
{
    // 월드행렬을 만든다.
    D3DXMATRIXA16 worldMatrix;
    D3DXMatrixRotationY(&worldMatrix, rotationY);

    // 뷰 행렬을 만든다.
    D3DXMATRIXA16 viewMatrix;
    const D3DXVECTOR3 eye(0.0f, 0.0f, -200.0f);
    const D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&viewMatrix, &eye, &lookAt, &up);

    // 투영행렬을 만든다.
    D3DXMATRIXA16 projectionMatrix;
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, Util::FOV, Util::ASPECT_RATIO, Util::NEAR_PLANE, Util::FAR_PLANE);

    // 월드 X 뷰 X 투영 행렬을 만든다.
    D3DXMATRIXA16 worldViewMatrix;
    D3DXMATRIXA16 worldViewProjectionMatrix;
    D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
    D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

    // 월드행렬의 역행렬을 구한다.
    D3DXMATRIXA16 inverseWorldMatrix;
    D3DXMatrixTranspose(&inverseWorldMatrix, &worldMatrix);

    //정점 셰이더 전역변수 설정
    shader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
    shader->SetMatrix("inverseWorldMatrix", &inverseWorldMatrix);
    shader->SetVector("worldLightPosition", &worldLightPosition);

    //픽셀 셰이더 전역 변수 설정
    shader->SetVector("surfaceColor", &surfaceColor);
    shader->SetFloat("toonShaderParam", toonShaderParam);

    UINT numPasses = 0;
    shader->Begin(&numPasses, NULL);
    for (UINT i = 0; i < numPasses; i++)
    {
        shader->BeginPass(i);
        {
            model->DrawSubset(0);
        }
        shader->EndPass();
    }
    shader->End();
}