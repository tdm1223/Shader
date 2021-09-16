#include "TextureMappingDirectManager.h"

// 3D 물체를 그린다.
void TextureMappingDirectManager::RenderScene()
{
    // 프레임마다 0.4도씩 회전을 시킨다.
    rotationY -= 0.4f * Util::PI / 180.0f;
    if (rotationY > 2 * Util::PI)
    {
        rotationY -= 2 * Util::PI;
    }

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

    //월드-뷰-투영행렬을 만든다.
    D3DXMATRIXA16 worldViewMatrix;
    D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
    D3DXMATRIXA16 worldViewProjectionMatrix;
    D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

    // 셰이더 전역변수들을 설정
    shader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
    shader->SetTexture("diffuseSampler_Tex", texture);

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
