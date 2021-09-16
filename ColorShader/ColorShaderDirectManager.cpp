#include "ColorShaderDirectManager.h"

ColorShaderDirectManager::ColorShaderDirectManager(D3DXVECTOR4 d3dvector) noexcept
{
    objectColor = d3dvector;
}

void ColorShaderDirectManager::RenderScene()
{
    // 월드 행렬
    D3DXMATRIXA16 worldMatrix;
    D3DXMatrixIdentity(&worldMatrix);

    // 뷰 행렬
    D3DXMATRIXA16 viewMatrix;
    const D3DXVECTOR3 eye(0.0f, 0.0f, -200.0f);
    const D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

    // 투영 행렬
    D3DXMATRIXA16 projectionMatrix;
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, Util::FOV, Util::ASPECT_RATIO, Util::NEAR_PLANE, Util::FAR_PLANE);

    // 위의 셋을 곱해서 월드-뷰-투영 행렬을 만듦.
    D3DXMATRIXA16 worldViewMatrix;
    D3DXMATRIXA16 worldViewProjectionMatrix;
    D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
    D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

    // 셰이더의 전역변수 설정
    shader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
    shader->SetVector("objectColor", &objectColor);

    UINT numPasses = 0;
    shader->Begin(&numPasses, NULL);
    for (UINT i = 0; i < numPasses; i++)
    {
        shader->BeginPass(i);
        model->DrawSubset(0);
        shader->EndPass();
    }
    shader->End();
}
