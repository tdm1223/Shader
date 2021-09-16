#include"DirectManager.h"
#include<sstream>

// D3D 객체 및 장치 초기화
bool DirectManager::InitD3D(HWND hWnd) noexcept
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
    {
        return false;
    }

    // D3D장치를 생성하는데 필요한 구조체를 채워넣는다.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.BackBufferWidth = Util::WIN_WIDTH;
    d3dpp.BackBufferHeight = Util::WIN_HEIGHT;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount = 1;
    d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality = 0;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.Windowed = TRUE;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
    d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    d3dpp.FullScreen_RefreshRateInHz = 0;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    // D3D장치를 생성한다.
    if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice)))
    {
        return false;
    }

    return true;
}

bool DirectManager::LoadAssets(std::string shaderName, std::string modelName, std::string textureName)
{
    // 텍스처 로딩
    if (!textureName.empty())
    {
        texture = LoadTexture(textureName);
        if (!texture)
        {
            return false;
        }
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

// 셰이더 로딩
LPD3DXEFFECT DirectManager::LoadShader(std::string filename)
{
    LPD3DXEFFECT ret = NULL;
    LPD3DXBUFFER pError = NULL;
    DWORD dwShaderFlags = 0;

#if _DEBUG
    dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

    D3DXCreateEffectFromFile(d3dDevice, filename.c_str(), NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

    // 셰이더 로딩에 실패한 경우 output창에 셰이더 컴파일 에러를 출력한다.
    if (!ret && pError)
    {
        const int size = pError->GetBufferSize();
        void* ack = pError->GetBufferPointer();

        if (ack)
        {
            std::stringstream  out;
            out << ack;
            OutputDebugString(out.str().c_str());
        }
    }

    return ret;
}

// 모델 로딩
LPD3DXMESH DirectManager::LoadModel(std::string filename) noexcept
{
    LPD3DXMESH ret = NULL;
    if (FAILED(D3DXLoadMeshFromX(filename.c_str(), D3DXMESH_SYSTEMMEM, d3dDevice, NULL, NULL, NULL, NULL, &ret)))
    {
        OutputDebugString("모델 로딩 실패: ");
        OutputDebugString(filename.c_str());
        OutputDebugString("\n");
    };

    return ret;
}

// 텍스처 로딩
LPDIRECT3DTEXTURE9 DirectManager::LoadTexture(std::string filename) noexcept
{
    LPDIRECT3DTEXTURE9 ret = NULL;
    if (FAILED(D3DXCreateTextureFromFile(d3dDevice, filename.c_str(), &ret)))
    {
        OutputDebugString("텍스처 로딩 실패: ");
        OutputDebugString(filename.c_str());
        OutputDebugString("\n");
    }

    return ret;
}

// 디버그 정보를 출력.
void DirectManager::RenderInfo(std::string str) noexcept
{
    // 텍스트 색상
    constexpr D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

    // 텍스트를 출력할 위치
    RECT rct;
    rct.left = 5;
    rct.right = Util::WIN_WIDTH / 3;
    rct.top = 5;
    rct.bottom = Util::WIN_HEIGHT / 3;

    // 텍스트 출력
    font->DrawText(NULL, str.c_str(), -1, &rct, 0, fontColor);
}

//렌더링
void DirectManager::RenderFrame(std::string str)
{
    constexpr D3DCOLOR bgColour = 0xFF0000FF;	// 배경색상 - 파랑

    d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

    d3dDevice->BeginScene();
    {
        RenderScene(); // 3D 물체를 그린다.
        RenderInfo(str); // 디버그 정보를 출력한다.
    }
    d3dDevice->EndScene();

    d3dDevice->Present(NULL, NULL, NULL, NULL);
}

//초기화 코드
bool DirectManager::Init(HWND hWnd, std::string shaderName, std::string modelName, std::string textureName)
{
    // D3D를 초기화
    if (!InitD3D(hWnd))
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