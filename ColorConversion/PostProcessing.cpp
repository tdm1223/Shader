#include "PostProcessing.h"
#include <cstdio>

#define PI           3.14159265f
#define FOV          (PI/4.0f)							// �þ߰�
#define ASPECT_RATIO (WIN_WIDTH/(float)WIN_HEIGHT)		// ȭ���� ��Ⱦ��
#define NEAR_PLANE   1									// ���� ���
#define FAR_PLANE    10000								// ���Ÿ� ���

// ��������

// D3D ����
LPDIRECT3D9             d3d = NULL;				// D3D
LPDIRECT3DDEVICE9       d3dDevice = NULL;		// D3D ��ġ

// ��Ʈ													
ID3DXFont*              font= NULL;             

// ��
LPD3DXMESH				teapot = NULL;

// ���̴�
LPD3DXEFFECT			environmentMappingShader = NULL;
LPD3DXEFFECT			noEffect = NULL;
LPD3DXEFFECT			grayScale = NULL;
LPD3DXEFFECT			sepia = NULL;

// �ؽ�ó
LPDIRECT3DTEXTURE9		stoneDM = NULL;
LPDIRECT3DTEXTURE9		stoneSM = NULL;
LPDIRECT3DTEXTURE9		stoneNM = NULL;
LPDIRECT3DCUBETEXTURE9	snowENV = NULL;

// ���α׷� �̸�
const char*				appName = "PostProcessing Framework";

// ȸ����
float					rotationY = 0.0f;

// ���� ��ġ
D3DXVECTOR4				worldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

// ���� ����
D3DXVECTOR4				lightColor(0.7f, 0.7f, 1.0f, 1.0f);

// ī�޶� ��ġ
D3DXVECTOR4				worldCameraPosition(0.0f, 0.0f, -200.0f, 1.0f);

// ȭ���� ���� ä��� �簢��
LPDIRECT3DVERTEXDECLARATION9	fullScreenQuad = NULL;
LPDIRECT3DVERTEXBUFFER9			fullScreenQuadVB = NULL;
LPDIRECT3DINDEXBUFFER9			fullScreenQuadIB = NULL;

// ��� ����Ÿ��
LPDIRECT3DTEXTURE9		sceneRenderTarget = NULL;

// ����� ����Ʈ���μ��� ���̴��� ����
int postProcessIndex = 0;

// ������
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// ������ Ŭ������ ����Ѵ�.
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		appName, NULL };
	RegisterClassEx(&wc);

	// ���α׷� â�� �����Ѵ�.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(appName, appName,style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Client Rect ũ�Ⱑ WIN_WIDTH, WIN_HEIGHT�� ������ ũ�⸦ �����Ѵ�.
	POINT ptDiff;
	RECT rcClient, rcWindow;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd, rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// D3D�� ����� ��� ���� �ʱ�ȭ�Ѵ�.
	if (!InitEverything(hWnd))
	{
		PostQuitMessage(1);
	}

	// �޽��� ����
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else // �޽����� ������ ������ ������Ʈ�ϰ� ����� �׸���
		{
			PlayDemo();
		}
	}

	UnregisterClass(appName, wc.hInstance);
	return 0;
}

// �޽��� ó����
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		ProcessInput(hWnd, wParam);
		break;
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Ű���� �Է�ó��
void ProcessInput(HWND hWnd, WPARAM keyPress)
{
	switch (keyPress)
	{
		// ESC Ű�� ������ ���α׷��� �����Ѵ�.
	case VK_ESCAPE:
		PostMessage(hWnd, WM_DESTROY, 0L, 0L);
		break;
	case '1':
	case '2':
	case '3':
		postProcessIndex = keyPress - '0' - 1;
		break;
	}
}

// ���ӷ���
void PlayDemo()
{
	Update();
	RenderFrame();
}

// ���ӷ��� ������Ʈ
void Update()
{
}

// ������
void RenderFrame()
{
	D3DCOLOR bgColour = 0x00000000;	

	d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

	d3dDevice->BeginScene();
	{
		RenderScene();				// 3D ��ü���� �׸���.
		RenderInfo();				// ����� ���� ���� ����Ѵ�.
	}
	d3dDevice->EndScene();

	d3dDevice->Present(NULL, NULL, NULL, NULL);
}


// 3D ��ü�� �׸���.
void RenderScene()
{
#pragma region 1. ����� ����Ÿ�� �ȿ� �׸���
	// ���� �ϵ���� ������
	LPDIRECT3DSURFACE9 backBuffer = NULL;
	d3dDevice->GetRenderTarget(0, &backBuffer);

	// ����Ÿ�� ���� �׸���.
	LPDIRECT3DSURFACE9 sceneSurface = NULL;
	if (SUCCEEDED(sceneRenderTarget->GetSurfaceLevel(0, &sceneSurface)))
	{
		d3dDevice->SetRenderTarget(0, sceneSurface);
		sceneSurface->Release();
		sceneSurface = NULL;
	}

	// ���� �����ӿ� �׷ȴ� ����� �����
	d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);

	// �����Ӹ��� 0.4���� ȸ���� ��Ų��.
	rotationY += 0.4f * PI / 180.0f;
	if (rotationY > 2 * PI)
	{
		rotationY -= 2 * PI;
	}

	// ��������� �����.
	D3DXMATRIXA16			worldMatrix;
	D3DXMatrixRotationY(&worldMatrix, rotationY);

	// �� ����� �����.
	D3DXMATRIXA16 viewMatrix;
	D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
	D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);
	
	// ��������� �����.
	D3DXMATRIXA16			projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

	// ����/��/��������� �̸� ���Ѵ�.
	D3DXMATRIXA16 worldViewMatrix;
	D3DXMATRIXA16 worldViewProjectionMatrix;
	D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
	D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

	// ���� ���̴� �������� ����
	environmentMappingShader->SetMatrix("worldMatrix", &worldMatrix);
	environmentMappingShader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
	environmentMappingShader->SetVector("worldLightPosition", &worldLightPosition);
	environmentMappingShader->SetVector("worldCameraPosition", &worldCameraPosition);

	// �ȼ� ���̴� �������� ����
	environmentMappingShader->SetTexture("diffuseMap_Tex", stoneDM);
	environmentMappingShader->SetTexture("specularMap_Tex", stoneSM);
	environmentMappingShader->SetTexture("normalMap_Tex", stoneNM);
	environmentMappingShader->SetTexture("environmentMap_Tex", snowENV);
	environmentMappingShader->SetVector("lightColor", &lightColor);

	// ���̴��� �����Ѵ�.
	UINT numPasses = 0;
	environmentMappingShader->Begin(&numPasses, NULL);
	{
		for (UINT i = 0; i < numPasses; ++i)
		{
			environmentMappingShader->BeginPass(i);
			{
				// ��ü�� �׸���.
				teapot->DrawSubset(0);
			}
			environmentMappingShader->EndPass();
		}
	}
	environmentMappingShader->End();
#pragma endregion

#pragma region 2. ����Ʈ���μ����� �����Ѵ�.
	// �ϵ���� ����۸� ����Ѵ�.
	d3dDevice->SetRenderTarget(0, backBuffer);
	backBuffer->Release();
	backBuffer = NULL;

	// ����� ����Ʈ���μ��� ȿ��
	LPD3DXEFFECT effectToUse = noEffect;
	if (postProcessIndex == 1)
	{
		effectToUse = grayScale;
	}
	else if (postProcessIndex == 2)
	{
		effectToUse = sepia;
	}

	effectToUse->SetTexture("SceneTexture_Tex", sceneRenderTarget);
	effectToUse->Begin(&numPasses, NULL);
	{
		for (UINT i = 0; i < numPasses; ++i)
		{
			effectToUse->BeginPass(i);
			{
				// ȭ�鰡�� �簢���� �׸���.
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

// ����� ������ ���.
void RenderInfo()
{
	// �ؽ�Ʈ ����
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// �ؽ�Ʈ�� ����� ��ġ
	RECT rect;
	rect.left = 5;
	rect.right = WIN_WIDTH / 2;
	rect.top = 5;
	rect.bottom = WIN_HEIGHT / 2;

	// Ű �Է� ������ ���
	font->DrawText(NULL, "ESC: ��������\n1: Į��\n2: ���\n3: ���Ǿ�", -1, &rect, 0, fontColor);
}

// �ʱ�ȭ �ڵ�
bool InitEverything(HWND hWnd)
{
	// D3D�� �ʱ�ȭ
	if (!InitD3D(hWnd))
	{
		return false;
	}

	// ȭ���� ����ä��� �簢���� �ϳ� �����
	InitFullScreenQuad();

	// ����Ÿ���� �����.
	if (FAILED(d3dDevice->CreateTexture(WIN_WIDTH, WIN_HEIGHT,
		1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, &sceneRenderTarget, NULL)))
	{
		return false;
	}

	// ��, ���̴�, �ؽ�ó���� �ε�
	if (!LoadAssets())
	{
		return false;
	}

	// ��Ʈ�� �ε�
	if (FAILED(D3DXCreateFont(d3dDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE),"Arial", &font)))
	{
		return false;
	}

	return true;
}

// D3D ��ü �� ��ġ �ʱ�ȭ
bool InitD3D(HWND hWnd)
{
	// D3D ��ü
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
	{
		return false;
	}

	// D3D��ġ�� �����ϴµ� �ʿ��� ����ü�� ä���ִ´�.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth = WIN_WIDTH;
	d3dpp.BackBufferHeight = WIN_HEIGHT;
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
		D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dpp, &d3dDevice)))
	{
		return false;
	}

	return true;
}

bool LoadAssets()
{
	// �ؽ�ó �ε�
	stoneDM = LoadTexture("Fieldstone_DM.tga");
	if (!stoneDM)
	{
		return false;
	}
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

	// ���̴� �ε�
	environmentMappingShader = LoadShader("EnvironmentMapping.fx");
	if (!environmentMappingShader)
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

	// �� �ε�
	teapot = LoadModel("TeapotWithTangent.x");
	if (!teapot)
	{
		return false;
	}

	return true;
}

// ���̴� �ε�
LPD3DXEFFECT LoadShader(const char * filename)
{
	LPD3DXEFFECT ret = NULL;

	LPD3DXBUFFER pError = NULL;
	DWORD dwShaderFlags = 0;

#if _DEBUG
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

	D3DXCreateEffectFromFile(d3dDevice, filename,
		NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

	// ���̴� �ε��� ������ ��� outputâ�� ���̴� ������ ������ ����Ѵ�.
	if (!ret && pError)
	{
		int size = pError->GetBufferSize();
		void *ack = pError->GetBufferPointer();

		if (ack)
		{
			char* str = new char[size];
			sprintf(str, (const char*)ack, size);
			OutputDebugString(str);
			delete[] str;
		}
	}

	return ret;
}

// �� �ε�
LPD3DXMESH LoadModel(const char * filename)
{
	LPD3DXMESH ret = NULL;
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, d3dDevice, NULL, NULL, NULL, NULL, &ret)))
	{
		OutputDebugString("�� �ε� ����: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	};

	return ret;
}

// �ؽ�ó �ε�
LPDIRECT3DTEXTURE9 LoadTexture(const char * filename)
{
	LPDIRECT3DTEXTURE9 ret = NULL;
	if (FAILED(D3DXCreateTextureFromFile(d3dDevice, filename, &ret)))
	{
		OutputDebugString("�ؽ�ó �ε� ����: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}

	return ret;
}

void Cleanup()
{
	// ��Ʈ�� release �Ѵ�.
	if (font)
	{
		font->Release();
		font = NULL;
	}

	// ���� release �Ѵ�.
	if (teapot)
	{
		teapot->Release();
		teapot = NULL;
	}

	// ���̴��� release �Ѵ�.
	if (environmentMappingShader)
	{
		environmentMappingShader->Release();
		environmentMappingShader = NULL;
	}
	if (noEffect)
	{
		noEffect->Release();
		noEffect = NULL;
	}
	if (grayScale)
	{
		grayScale->Release();
		grayScale = NULL;
	}
	if (sepia)
	{
		sepia->Release();
		sepia = NULL;
	}

	// �ؽ�ó�� release �Ѵ�.
	if (stoneDM)
	{
		stoneDM->Release();
		stoneDM = NULL;
	}
	if (stoneSM)
	{
		stoneSM->Release();
		stoneSM = NULL;
	}
	if (stoneNM)
	{
		stoneNM->Release();
		stoneNM = NULL;
	}
	if (snowENV)
	{
		snowENV->Release();
		snowENV = NULL;
	}

	// ȭ��ũ�� �簢���� �����Ѵ�
	if (fullScreenQuad)
	{
		fullScreenQuad->Release();
		fullScreenQuad = NULL;
	}
	if (fullScreenQuadVB)
	{
		fullScreenQuadVB->Release();
		fullScreenQuadVB = NULL;
	}
	if (fullScreenQuadIB)
	{
		fullScreenQuadIB->Release();
		fullScreenQuadIB = NULL;
	}

	//����Ÿ���� �����Ѵ�
	if (sceneRenderTarget)
	{
		sceneRenderTarget->Release();
		sceneRenderTarget = NULL;
	}

	// D3D�� release �Ѵ�.
	if (d3dDevice)
	{
		d3dDevice->Release();
		d3dDevice = NULL;
	}
	if (d3d)
	{
		d3d->Release();
		d3d = NULL;
	}
}

void InitFullScreenQuad()
{
	// ���� ������ �����
	D3DVERTEXELEMENT9 vertices[3];
	int offset = 0;
	int i = 0;

	// ��ġ
	vertices[i].Stream = 0;
	vertices[i].Offset = offset;
	vertices[i].Type = D3DDECLTYPE_FLOAT3;
	vertices[i].Method = D3DDECLMETHOD_DEFAULT;
	vertices[i].Usage = D3DDECLUSAGE_POSITION;
	vertices[i].UsageIndex = 0;

	offset += sizeof(float) * 3;
	++i;

	// UV��ǥ 0
	vertices[i].Stream = 0;
	vertices[i].Offset = offset;
	vertices[i].Type = D3DDECLTYPE_FLOAT2;
	vertices[i].Method = D3DDECLMETHOD_DEFAULT;
	vertices[i].Usage = D3DDECLUSAGE_TEXCOORD;
	vertices[i].UsageIndex = 0;

	offset += sizeof(float) * 2;
	++i;

	// ���������� ������ ǥ�� (D3DDECL_END())
	vertices[i].Stream = 0xFF;
	vertices[i].Offset = 0;
	vertices[i].Type = D3DDECLTYPE_UNUSED;
	vertices[i].Method = 0;
	vertices[i].Usage = 0;
	vertices[i].UsageIndex = 0;

	d3dDevice->CreateVertexDeclaration(vertices, &fullScreenQuad);

	// �������۸� �����.
	d3dDevice->CreateVertexBuffer(offset * 4, 0, 0, D3DPOOL_MANAGED, &fullScreenQuadVB, NULL);
	void * vertexData = NULL;
	fullScreenQuadVB->Lock(0, 0, &vertexData, 0);
	{
		float * data = (float*)vertexData;
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

	// ���ι��۸� �����.
	d3dDevice->CreateIndexBuffer(sizeof(short) * 6, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &fullScreenQuadIB, NULL);
	void * indexData = NULL;
	fullScreenQuadIB->Lock(0, 0, &indexData, 0);
	{
		unsigned short * data = (unsigned short*)indexData;
		*data++ = 0;	*data++ = 1;	*data++ = 3;
		*data++ = 3;	*data++ = 1;	*data++ = 2;
	}
	fullScreenQuadIB->Unlock();
}
