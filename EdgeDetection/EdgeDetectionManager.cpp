#include "EdgeDetectionManager.h"

EdgeDetectionManager::EdgeDetectionManager()
{
    rotationY = 0.0f;
    worldCameraPosition = D3DXVECTOR4(0.0f, 0.0f, -200.0f, 1.0f);
    worldLightPosition = D3DXVECTOR4(500.0f, 500.0f, -500.0f, 1.0f);
    lightColor = D3DXVECTOR4(0.7f, 0.7f, 1.0f, 1.0f);
}

void EdgeDetectionManager::RenderScene()
{
#pragma region 장면을 렌더타깃 안에 그린다
    // 현재 하드웨어 벡버퍼
    LPDIRECT3DSURFACE9 backBuffer = NULL;
    d3dDevice->GetRenderTarget(0, &backBuffer);

    // 렌더타깃 위에 그린다.
    LPDIRECT3DSURFACE9 sceneSurface = NULL;
    if (SUCCEEDED(sceneRenderTarget->GetSurfaceLevel(0, &sceneSurface)))
    {
        d3dDevice->SetRenderTarget(0, sceneSurface);
        sceneSurface->Release();
        sceneSurface = NULL;
    }

    // 저번 프레임에 그렸던 장면을 지운다
    d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);

    // 프레임마다 0.4도씩 회전을 시킨다.
    rotationY += 0.4f * Util::PI / 180.0f;
    if (rotationY > 2 * Util::PI)
    {
        rotationY -= 2 * Util::PI;
    }

    // 월드행렬을 만든다.
    D3DXMATRIXA16			worldMatrix;
    D3DXMatrixRotationY(&worldMatrix, rotationY);

    // 뷰 행렬을 만든다.
    D3DXMATRIXA16 viewMatrix;
    D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
    D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

    // 투영행렬을 만든다.
    D3DXMATRIXA16			projectionMatrix;
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, Util::FOV, Util::ASPECT_RATIO, Util::NEAR_PLANE, Util::FAR_PLANE);

    // 월드/뷰/투영행렬을 미리 곱한다.
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
    shader->SetTexture("diffuseMap_Tex", stoneDM);
    shader->SetTexture("specularMap_Tex", stoneSM);
    shader->SetTexture("normalMap_Tex", stoneNM);
    shader->SetTexture("environmentMap_Tex", snowENV);
    shader->SetVector("lightColor", &lightColor);

    // 셰이더를 시작한다.
    UINT numPasses = 0;
    shader->Begin(&numPasses, NULL);
    {
        for (UINT i = 0; i < numPasses; ++i)
        {
            shader->BeginPass(i);
            {
                // 구체를 그린다.
                model->DrawSubset(0);
            }
            shader->EndPass();
        }
    }
    shader->End();
#pragma endregion

#pragma region 포스트프로세싱을 적용한다.
    // 하드웨어 백버퍼를 사용한다.
    d3dDevice->SetRenderTarget(0, backBuffer);
    backBuffer->Release();
    backBuffer = NULL;

    // 사용할 포스트프로세스 효과
    LPD3DXEFFECT effectToUse = noEffect;
    if (postProcessIndex == 1)
    {
        effectToUse = grayScale;
    }
    else if (postProcessIndex == 2)
    {
        effectToUse = sepia;
    }
    else if (postProcessIndex == 3)
    {
        effectToUse = edgeDetection;
    }
    else if (postProcessIndex == 4)
    {
        effectToUse = emboss;
    }
    else if (postProcessIndex == 5)
    {
        effectToUse = laplacian;
    }

    D3DXVECTOR4 pixelOffset(1 / (float)Util::WIN_WIDTH, 1 / (float)Util::WIN_HEIGHT, 0, 0);
    if (effectToUse == edgeDetection || effectToUse == emboss || effectToUse == laplacian)
    {
        effectToUse->SetVector("pixelOffset", &pixelOffset);
    }

    effectToUse->SetTexture("SceneTexture_Tex", sceneRenderTarget);
    effectToUse->Begin(&numPasses, NULL);
    {
        for (UINT i = 0; i < numPasses; ++i)
        {
            effectToUse->BeginPass(i);
            {
                // 화면가득 사각형을 그린다.
                d3dDevice->SetStreamSource(0, fullScreenQuadVB, 0, sizeof(float) * 5);
                d3dDevice->SetIndices(fullScreenQuadIB);
                d3dDevice->SetVertexDeclaration(fullScreenQuad);
                d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2);
            }
            effectToUse->EndPass();
        }
    }
    effectToUse->End();
#pragma endregion

}

void EdgeDetectionManager::InitFullScreenQuad()
{
    // 정점 선언을 만든다
    D3DVERTEXELEMENT9 vertices[3];
    int offset = 0;
    int i = 0;

    // 위치
    vertices[i].Stream = 0;
    vertices[i].Offset = offset;
    vertices[i].Type = D3DDECLTYPE_FLOAT3;
    vertices[i].Method = D3DDECLMETHOD_DEFAULT;
    vertices[i].Usage = D3DDECLUSAGE_POSITION;
    vertices[i].UsageIndex = 0;

    offset += sizeof(float) * 3;
    ++i;

    // UV좌표 0
    vertices[i].Stream = 0;
    vertices[i].Offset = offset;
    vertices[i].Type = D3DDECLTYPE_FLOAT2;
    vertices[i].Method = D3DDECLMETHOD_DEFAULT;
    vertices[i].Usage = D3DDECLUSAGE_TEXCOORD;
    vertices[i].UsageIndex = 0;

    offset += sizeof(float) * 2;
    ++i;

    // 정점포맷의 끝임을 표현 (D3DDECL_END())
    vertices[i].Stream = 0xFF;
    vertices[i].Offset = 0;
    vertices[i].Type = D3DDECLTYPE_UNUSED;
    vertices[i].Method = 0;
    vertices[i].Usage = 0;
    vertices[i].UsageIndex = 0;

    d3dDevice->CreateVertexDeclaration(vertices, &fullScreenQuad);

    // 정점버퍼를 만든다.
    d3dDevice->CreateVertexBuffer(offset * 4, 0, 0, D3DPOOL_MANAGED, &fullScreenQuadVB, NULL);
    void* vertexData = NULL;
    fullScreenQuadVB->Lock(0, 0, &vertexData, 0);
    {
        float* data = (float*)vertexData;
        *data++ = -1.0f;	*data++ = 1.0f;		*data++ = 0.0f;
        *data++ = 0.0f;		*data++ = 0.0f;

        *data++ = 1.0f;		*data++ = 1.0f;		*data++ = 0.0f;
        *data++ = 1.0f;		*data++ = 0;

        *data++ = 1.0f;		*data++ = -1.0f;	*data++ = 0.0f;
        *data++ = 1.0f;		*data++ = 1.0f;

        *data++ = -1.0f;	*data++ = -1.0f;	*data++ = 0.0f;
        *data++ = 0.0f;		*data++ = 1.0f;
    }
    fullScreenQuadVB->Unlock();

    // 색인버퍼를 만든다.
    d3dDevice->CreateIndexBuffer(sizeof(short) * 6, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &fullScreenQuadIB, NULL);
    void* indexData = NULL;
    fullScreenQuadIB->Lock(0, 0, &indexData, 0);
    {
        unsigned short* data = (unsigned short*)indexData;
        *data++ = 0;	*data++ = 1;	*data++ = 3;
        *data++ = 3;	*data++ = 1;	*data++ = 2;
    }
    fullScreenQuadIB->Unlock();
}

bool EdgeDetectionManager::Init(HWND hWnd, std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
{
    // D3D를 초기화
    if (!InitD3D(hWnd))
    {
        return false;
    }

    // 화면을 가득채우는 사각형을 하나 만든다
    InitFullScreenQuad();

    // 렌더타깃을 만든다.
    if (FAILED(d3dDevice->CreateTexture(Util::WIN_WIDTH, Util::WIN_HEIGHT, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8,
        D3DPOOL_DEFAULT, &sceneRenderTarget, NULL)))
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
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE),
        "Arial", &font)))
    {
        return false;
    }

    return true;
}

bool EdgeDetectionManager::LoadAssets(std::string shaderName /*= ""*/, std::string modelName /*= ""*/, std::string textureName /*= ""*/)
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

    stoneNM = LoadTexture("Fieldstone_NM.tga");
    if (!stoneNM)
    {
        return false;
    }

    D3DXCreateCubeTextureFromFile(d3dDevice, "Snow_ENV.dds", &snowENV);
    if (!snowENV)
    {
        return false;
    }

    noEffect = LoadShader("NoEffect.fx");
    if (!noEffect)
    {
        return false;
    }

    grayScale = LoadShader("Grayscale.fx");
    if (!grayScale)
    {
        return false;
    }

    sepia = LoadShader("Sepia.fx");
    if (!sepia)
    {
        return false;
    }

    edgeDetection = LoadShader("EdgeDetection.fx");
    if (!edgeDetection)
    {
        return false;
    }

    emboss = LoadShader("Emboss.fx");
    if (!emboss)
    {
        return false;
    }
    laplacian = LoadShader("Laplacian.fx");
    if (!laplacian)
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