#include"DirectManager.h"
#include<sstream>

// D3D ��ü �� ��ġ �ʱ�ȭ
bool DirectManager::InitD3D(HWND hWnd) noexcept
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
    {
        return false;
    }

    // D3D��ġ�� �����ϴµ� �ʿ��� ����ü�� ä���ִ´�.
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

    // D3D��ġ�� �����Ѵ�.
    if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice)))
    {
        return false;
    }

    return true;
}

bool DirectManager::LoadAssets(std::string shaderName, std::string modelName, std::string textureName)
{
    // ���̴� �ε�
    shader = LoadShader(shaderName);
    if (!shader)
    {
        return false;
    }

    // �� �ε�
    model = LoadModel(modelName);
    if (!model)
    {
        return false;
    }

    return true;
}

// ���̴� �ε�
LPD3DXEFFECT DirectManager::LoadShader(std::string filename)
{
    LPD3DXEFFECT ret = NULL;
    LPD3DXBUFFER pError = NULL;
    DWORD dwShaderFlags = 0;

#if _DEBUG
    dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

    D3DXCreateEffectFromFile(d3dDevice, filename.c_str(), NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

    // ���̴� �ε��� ������ ��� outputâ�� ���̴� ������ ������ ����Ѵ�.
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

// �� �ε�
LPD3DXMESH DirectManager::LoadModel(std::string filename) noexcept
{
    LPD3DXMESH ret = NULL;
    if (FAILED(D3DXLoadMeshFromX(filename.c_str(), D3DXMESH_SYSTEMMEM, d3dDevice, NULL, NULL, NULL, NULL, &ret)))
    {
        OutputDebugString("�� �ε� ����: ");
        OutputDebugString(filename.c_str());
        OutputDebugString("\n");
    };

    return ret;
}

// �ؽ�ó �ε�
LPDIRECT3DTEXTURE9 DirectManager::LoadTexture(std::string filename) noexcept
{
    LPDIRECT3DTEXTURE9 ret = NULL;
    if (FAILED(D3DXCreateTextureFromFile(d3dDevice, filename.c_str(), &ret)))
    {
        OutputDebugString("�ؽ�ó �ε� ����: ");
        OutputDebugString(filename.c_str());
        OutputDebugString("\n");
    }

    return ret;
}

// ����� ������ ���.
void DirectManager::RenderInfo(std::string str) noexcept
{
    // �ؽ�Ʈ ����
    constexpr D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

    // �ؽ�Ʈ�� ����� ��ġ
    RECT rct;
    rct.left = 5;
    rct.right = Util::WIN_WIDTH / 3;
    rct.top = 5;
    rct.bottom = Util::WIN_HEIGHT / 3;

    // �ؽ�Ʈ ���
    font->DrawText(NULL, str.c_str(), -1, &rct, 0, fontColor);
}

//������
void DirectManager::RenderFrame(std::string str)
{
    constexpr D3DCOLOR bgColour = 0xFF0000FF;	// ������ - �Ķ�

    d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

    d3dDevice->BeginScene();
    {
        RenderScene(); // 3D ��ü�� �׸���.
        RenderInfo(str); // ����� ������ ����Ѵ�.
    }
    d3dDevice->EndScene();

    d3dDevice->Present(NULL, NULL, NULL, NULL);
}

//�ʱ�ȭ �ڵ�
bool DirectManager::Init(HWND hWnd, std::string shaderName, std::string modelName, std::string textureName)
{
    // D3D�� �ʱ�ȭ
    if (!InitD3D(hWnd))
    {
        return false;
    }

    // ��, ���̴�, �ؽ�ó���� �ε�
    if (!LoadAssets(shaderName, modelName, textureName))
    {
        return false;
    }

    // ��Ʈ�� �ε�
    if (FAILED(D3DXCreateFont(d3dDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE), "Arial", &font)))
    {
        return false;
    }

    return true;
}