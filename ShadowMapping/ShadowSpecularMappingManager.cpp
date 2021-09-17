#include "ShadowSpecularMappingManager.h"

ShadowSpecularMappingManager::ShadowSpecularMappingManager()
{
    rotationY = 0.0f;
    worldCameraPosition = D3DXVECTOR4(200.0f, 200.0f, 200.0f, 1.0f);
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
    lightColor = D3DXVECTOR4(0.7f, 0.7f, 1.0f, 1.0f);
    discColor = D3DXVECTOR4(0.0f, 1.0f, 1.0f, 1.0f);
}

void ShadowSpecularMappingManager::RenderScene()
{
    // 광원-뷰행렬을 만든다.
    D3DXMATRIXA16 lightViewMatrix;
    {
        const D3DXVECTOR3 eye(worldLightPosition.x, worldLightPosition.y, worldLightPosition.z);
        const D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
        const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
        D3DXMatrixLookAtLH(&lightViewMatrix, &eye, &at, &up);
    }

    // 광원-투영행렬을 만든다.
    D3DXMATRIXA16 lightProjectionMatrix;
    {
        D3DXMatrixPerspectiveFovLH(&lightProjectionMatrix, D3DX_PI / 4.0f, 1, 1, 1000);
    }

    // 뷰-투영행렬을 만든다.
    D3DXMATRIXA16 viewProjectionMatrix;
    {
        // 뷰행렬을 만든다.
        D3DXMATRIXA16 viewMatrix;
        const D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
        const D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
        const D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
        D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

        // 투영행렬을 만든다.
        D3DXMATRIXA16 projectionMatrix;
        D3DXMatrixPerspectiveFovLH(&projectionMatrix, Util::FOV, Util::ASPECT_RATIO, Util::NEAR_PLANE, Util::FAR_PLANE);

        // 둘을 곱해 뷰-투영행렬을 만든다.
        D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);
    }

    // torus의 월드 행렬을 만든다.
    D3DXMATRIXA16 torusWorldMatrix;
    {
        rotationY += 0.4f * Util::PI / 180.0f;
        if (rotationY > 2 * Util::PI)
        {
            rotationY -= 2 * Util::PI;
        }
        D3DXMatrixRotationY(&torusWorldMatrix, rotationY);
    }

    // disc의 월드 행렬을 만든다.
    D3DXMATRIXA16 discWorldMatrix;
    {
        D3DXMATRIXA16 scaleMatrix;
        D3DXMatrixScaling(&scaleMatrix, 2.5f, 2.5f, 2.5f);
        D3DXMATRIXA16 translationMatrix;
        D3DXMatrixTranslation(&translationMatrix, 0.0f, -40.0f, 0.0f);
        D3DXMatrixMultiply(&discWorldMatrix, &scaleMatrix, &translationMatrix);
    }

    d3dDevice->GetRenderTarget(0, &backBuffer);
    d3dDevice->GetDepthStencilSurface(&depthStencilBuffer);

    // 1. 그림자를 만든다.
    if (SUCCEEDED(shadowRenderTarget->GetSurfaceLevel(0, &shadowSurface)))
    {
        d3dDevice->SetRenderTarget(0, shadowSurface);
        shadowSurface->Release();
        shadowSurface = NULL;
    }
    d3dDevice->SetDepthStencilSurface(shadowDepthStencil);

    // 그림자 맵을 하얀색으로 지운다.
    d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0xFFFFFFFF, 1.0f, 0);

    // 그림자 만들기 Shader 전역변수들을 설정한다.
    createShadowShader->SetMatrix("worldMatrix", &torusWorldMatrix);
    createShadowShader->SetMatrix("lightViewMatrix", &lightViewMatrix);
    createShadowShader->SetMatrix("lightProjectionMatrix", &lightProjectionMatrix);

    // 그림자 만들기 Shader 시작
    {
        UINT numPasses = 0;
        createShadowShader->Begin(&numPasses, NULL);
        for (UINT i = 0; i < numPasses; i++)
        {
            createShadowShader->BeginPass(i);
            torus->DrawSubset(0);
            createShadowShader->EndPass();
        }
        createShadowShader->End();
    }

    // 2. 그림자를 입힌다.
    // 하드웨어 백버퍼/깊이 버퍼 사용
    d3dDevice->SetRenderTarget(0, backBuffer);
    d3dDevice->SetDepthStencilSurface(depthStencilBuffer);

    // 위 두 함수로 인해 참조 카운트가 증가된다. 따라서 두개를 해제해줘야 한다.
    backBuffer->Release();
    backBuffer = NULL;
    depthStencilBuffer->Release();
    depthStencilBuffer = NULL;

    // 그림자 입히기 Shader 전역변수들을 설정한다.
    shader->SetVector("lightColor", &lightColor);
    shader->SetVector("worldLightPosition", &worldLightPosition);
    shader->SetVector("worldCameraPosition", &worldCameraPosition);
    shader->SetMatrix("worldMatrix", &torusWorldMatrix);
    shader->SetMatrix("lightViewMatrix", &lightViewMatrix);
    shader->SetMatrix("lightProjectionMatrix", &lightProjectionMatrix);
    shader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);

    shader->SetTexture("diffuseMap_Tex", texture);
    shader->SetTexture("specularMap_Tex", stoneSM);
    shader->SetTexture("ShadowMap_Tex", shadowRenderTarget);

    // Shader를 시작한다.
    UINT numPasses = 0;
    shader->Begin(&numPasses, NULL);
    for (UINT i = 0; i < numPasses; i++)
    {
        shader->BeginPass(i);
        torus->DrawSubset(0);
        shader->EndPass();
    }
    shader->End();

    discShader->SetVector("worldLightPosition", &worldLightPosition);
    discShader->SetMatrix("worldMatrix", &discWorldMatrix);
    discShader->SetMatrix("lightViewMatrix", &lightViewMatrix);
    discShader->SetMatrix("lightProjectionMatrix", &lightProjectionMatrix);
    discShader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);
    discShader->SetTexture("ShadowMap_Tex", shadowRenderTarget);
    discShader->SetVector("objectColor", &discColor);
    numPasses = 0;
    discShader->Begin(&numPasses, NULL);
    for (UINT i = 0; i < numPasses; i++)
    {
        discShader->BeginPass(i);
        model->DrawSubset(0);
        discShader->EndPass();
    }
    discShader->End();
}

bool ShadowSpecularMappingManager::LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
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

    // 그림자 관련 로딩
    createShadowShader = LoadShader("CreateShadow.fx");
    if (!createShadowShader)
    {
        return false;
    }

    discShader = LoadShader("ApplyDisc.fx");
    if (!discShader)
    {
        return false;
    }

    torus = LoadModel("Torus.x");
    if (!torus)
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

bool ShadowSpecularMappingManager::Init(HWND hWnd, std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
{
    // D3D를 초기화
    if (!InitD3D(hWnd))
    {
        return false;
    }


    if (FAILED(d3dDevice->CreateTexture(shadowMapSize, shadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &shadowRenderTarget, NULL)))
    {
        return false;
    }

    if (FAILED(d3dDevice->CreateDepthStencilSurface(shadowMapSize, shadowMapSize, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, true, &shadowDepthStencil, NULL)))
    {
        return false;
    }

    // 모델, 셰이더, 텍스처등을 로딩
    if (!LoadAssets(shaderName, modelName, textureName))
    {
        return false;
    }

    // 폰트를 로딩
    if (FAILED(D3DXCreateFont(d3dDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE), "Arial", &font)))
    {
        return false;
    }

    return true;
}
