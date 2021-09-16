#include "LightingManager.h"

LightingManager::LightingManager()
{
    rotationY = 0.0f;
    Pow = 20.0f;
    worldCameraPosition = D3DXVECTOR4(0.0f, 0.0f, -200.0f, 1.0f);
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
}

// 3D 물체를 그린다.
void LightingManager::RenderScene()
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

    D3DXMATRIXA16 viewProjectionMatrix;
    D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);

    // 셰이더 전역변수들을 설정
    shader->SetMatrix("worldMatrix", &worldMatrix);
    shader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);

    shader->SetVector("worldCameraPosition", &worldCameraPosition);
    shader->SetVector("worldLightPosition", &worldLightPosition);
    shader->SetFloat("Pow", Pow);

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