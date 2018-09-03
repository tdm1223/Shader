//**********************************************************************
//
// EdgeDetection.cpp
// 
//
//**********************************************************************

#include "EdgeDetection.h"
#include <stdio.h>

#define PI           3.14159265f
#define FOV          (PI/4.0f)							// 시야각
#define ASPECT_RATIO (WIN_WIDTH/(float)WIN_HEIGHT)		// 화면의 종횡비
#define NEAR_PLANE   1									// 근접 평면
#define FAR_PLANE    10000								// 원거리 평면


//----------------------------------------------------------------------
// 전역변수
//----------------------------------------------------------------------

// D3D 관련
LPDIRECT3D9             d3d = NULL;				// D3D
LPDIRECT3DDEVICE9       d3dDevice = NULL;		// D3D 장치

														
ID3DXFont*              font= NULL;             // 폰트

// 모델
LPD3DXMESH				teapot = NULL;

// 쉐이더
LPD3DXEFFECT			environmentMappingShader = NULL;
LPD3DXEFFECT			noEffect = NULL;
LPD3DXEFFECT			grayScale = NULL;
LPD3DXEFFECT			sepia = NULL;
LPD3DXEFFECT			edgeDetection = NULL;
LPD3DXEFFECT			emboss = NULL;
LPD3DXEFFECT			laplacian = NULL;

// 텍스처
LPDIRECT3DTEXTURE9		stoneDM = NULL;
LPDIRECT3DTEXTURE9		stoneSM = NULL;
LPDIRECT3DTEXTURE9		stoneNM = NULL;
LPDIRECT3DCUBETEXTURE9	snowENV = NULL;

// 프로그램 이름
const char*				appName = "EdgeDetection 프레임워크";

// 회전값
float					rotationY = 0.0f;

// 빛의 위치
D3DXVECTOR4				worldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

// 빛의 색상
D3DXVECTOR4				lightColor(0.7f, 0.7f, 1.0f, 1.0f);

// 카메라 위치
D3DXVECTOR4				worldCameraPosition(0.0f, 0.0f, -200.0f, 1.0f);

// 화면을 가득 채우는 사각형
LPDIRECT3DVERTEXDECLARATION9	fullScreenQuad = NULL;
LPDIRECT3DVERTEXBUFFER9			fullScreenQuadVB = NULL;
LPDIRECT3DINDEXBUFFER9			fullScreenQuadIB = NULL;

// 장면 렌더타깃
LPDIRECT3DTEXTURE9		sceneRenderTarget = NULL;

// 사용할 포스트프로세스 쉐이더의 색인
int postProcessIndex = 0;

// 진입점
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스를 등록한다.
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		appName, NULL };
	RegisterClassEx(&wc);

	// 프로그램 창을 생성한다.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(appName, appName,
		style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
		GetDesktopWindow(), NULL, wc.hInstance, NULL);

	// Client Rect 크기가 WIN_WIDTH, WIN_HEIGHT와 같도록 크기를 조정한다.
	POINT ptDiff;
	RECT rcClient, rcWindow;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd, rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// D3D를 비롯한 모든 것을 초기화한다.
	if (!InitEverything(hWnd))
		PostQuitMessage(1);

	// 메시지 루프
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else // 메시지가 없으면 게임을 업데이트하고 장면을 그린다
		{
			PlayDemo();
		}
	}

	UnregisterClass(appName, wc.hInstance);
	return 0;
}

// 메시지 처리기
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

// 키보드 입력처리
void ProcessInput(HWND hWnd, WPARAM keyPress)
{
	switch (keyPress)
	{
		// ESC 키가 눌리면 프로그램을 종료한다.
	case VK_ESCAPE:
		PostMessage(hWnd, WM_DESTROY, 0L, 0L);
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		postProcessIndex = keyPress - '0' - 1;
		break;
	}
}

// 게임루프
void PlayDemo()
{
	Update();
	RenderFrame();
}

// 게임로직 업데이트
void Update()
{
}

// 렌더링
void RenderFrame()
{
	D3DCOLOR bgColour = 0x000000FF;	

	d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

	d3dDevice->BeginScene();
	{
		RenderScene();				// 3D 물체등을 그린다.
		RenderInfo();				// 디버그 정보 등을 출력한다.
	}
	d3dDevice->EndScene();

	d3dDevice->Present(NULL, NULL, NULL, NULL);
}


// 3D 물체등을 그린다.
void RenderScene()
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

	// 뷰 행렬을 만든다.
	D3DXMATRIXA16 viewMatrix;
	D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
	D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

	// 투영행렬을 만든다.
	D3DXMATRIXA16			projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

	// 프레임마다 0.4도씩 회전을 시킨다.
	rotationY += 0.4f * PI / 180.0f;
	if (rotationY > 2 * PI)
	{
		rotationY -= 2 * PI;
	}

	// 월드행렬을 만든다.
	D3DXMATRIXA16			worldMatrix;
	D3DXMatrixRotationY(&worldMatrix, rotationY);

	// 월드/뷰/투영행렬을 미리 곱한다.
	D3DXMATRIXA16 worldViewMatrix;
	D3DXMATRIXA16 worldViewProjectionMatrix;
	D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
	D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

	// 쉐이더 전역변수들을 설정
	environmentMappingShader->SetMatrix("worldMatrix", &worldMatrix);
	environmentMappingShader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);

	environmentMappingShader->SetVector("worldLightPosition", &worldLightPosition);
	environmentMappingShader->SetVector("worldCameraPosition", &worldCameraPosition);

	environmentMappingShader->SetVector("lightColor", &lightColor);
	environmentMappingShader->SetTexture("diffuseMap_Tex", stoneDM);
	environmentMappingShader->SetTexture("specularMap_Tex", stoneSM);
	environmentMappingShader->SetTexture("normalMap_Tex", stoneNM);
	environmentMappingShader->SetTexture("environmentMap_Tex", snowENV);

	// 쉐이더를 시작한다.
	UINT numPasses = 0;
	environmentMappingShader->Begin(&numPasses, NULL);
	{
		for (UINT i = 0; i < numPasses; ++i)
		{
			environmentMappingShader->BeginPass(i);
			{
				// 구체를 그린다.
				teapot->DrawSubset(0);
			}
			environmentMappingShader->EndPass();
		}
	}
	environmentMappingShader->End();
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
	
	D3DXVECTOR4 pixelOffset(1 / (float)WIN_WIDTH, 1 / (float)WIN_HEIGHT, 0, 0);
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

// 디버그 정보 등을 출력.
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// 텍스트를 출력할 위치
	RECT rect;
	rect.left = 5;
	rect.right = WIN_WIDTH / 3;
	rect.top = 5;
	rect.bottom = WIN_HEIGHT / 3;

	// 키 입력 정보를 출력
	font->DrawText(NULL, "데모 프레임워크\n\nESC: 데모종료\n1: 칼라\n2: 흑백\n3: 세피아\n4: 외곽선 찾기\n5: 양각효과\n6: 라플라시안", -1, &rect, 0, fontColor);
}

bool InitEverything(HWND hWnd)
{
	// D3D를 초기화
	if (!InitD3D(hWnd))
	{
		return false;
	}

	// 화면을 가득채우는 사각형을 하나 만든다
	InitFullScreenQuad();

	// 렌더타깃을 만든다.
	if (FAILED(d3dDevice->CreateTexture(WIN_WIDTH, WIN_HEIGHT,
		1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT, &sceneRenderTarget, NULL)))
	{
		return false;
	}

	// 모델, 쉐이더, 텍스처등을 로딩
	if (!LoadAssets())
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

// D3D 객체 및 장치 초기화
bool InitD3D(HWND hWnd)
{
	// D3D 객체
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d)
	{
		return false;
	}

	// D3D장치를 생성하는데 필요한 구조체를 채워넣는다.
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

	// D3D장치를 생성한다.
	if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &d3dDevice)))
	{
		return false;
	}

	return true;
}

bool LoadAssets()
{
	// 텍스처 로딩
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

	// 쉐이더 로딩
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

	// 모델 로딩
	teapot = LoadModel("TeapotWithTangent.x");
	if (!teapot)
	{
		return false;
	}

	return true;
}

// 쉐이더 로딩
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

	// 쉐이더 로딩에 실패한 경우 output창에 쉐이더
	// 컴파일 에러를 출력한다.
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

// 모델 로딩
LPD3DXMESH LoadModel(const char * filename)
{
	LPD3DXMESH ret = NULL;
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, d3dDevice, NULL, NULL, NULL, NULL, &ret)))
	{
		OutputDebugString("모델 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	};

	return ret;
}

// 텍스처 로딩
LPDIRECT3DTEXTURE9 LoadTexture(const char * filename)
{
	LPDIRECT3DTEXTURE9 ret = NULL;
	if (FAILED(D3DXCreateTextureFromFile(d3dDevice, filename, &ret)))
	{
		OutputDebugString("텍스처 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}

	return ret;
}

void Cleanup()
{
	// 폰트를 release 한다.
	if (font)
	{
		font->Release();
		font = NULL;
	}

	// 모델을 release 한다.
	if (teapot)
	{
		teapot->Release();
		teapot = NULL;
	}

	// 쉐이더를 release 한다.
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

	if (edgeDetection)
	{
		edgeDetection->Release();
		edgeDetection = NULL;
	}

	if (emboss)
	{
		emboss->Release();
		emboss = NULL;
	}

	if (laplacian)
	{
		laplacian->Release();
		laplacian = NULL;
	}

	// 텍스처를 release 한다.
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

	// 화면크기 사각형을 해제한다
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

	//렌더타깃을 해제한다
	if (sceneRenderTarget)
	{
		sceneRenderTarget->Release();
		sceneRenderTarget = NULL;
	}

	// D3D를 release 한다.
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

	// 색인버퍼를 만든다.
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
