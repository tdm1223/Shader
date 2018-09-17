//**********************************************************************
//
// ShadowSpecularMapping.cpp
//
//**********************************************************************

#include "ShadowSpecularMapping.h"
#include <stdio.h>

// 전역변수
#define PI				3.14159265f
#define FOV				(PI/4.0f)
#define ASPECT_RATIO	(WIN_WIDTH/(float)WIN_HEIGHT)
#define NEAR_PLANE		1
#define FAR_PLANE		10000

// 회전값
float					rotationY = 0.0f;

// D3D 관련
LPDIRECT3D9             d3d = NULL;				// D3D
LPDIRECT3DDEVICE9       d3dDevice = NULL;				// D3D 장치
			
//폰트
ID3DXFont*              font = NULL;

// 모델
LPD3DXMESH				torus = NULL;
LPD3DXMESH				disc = NULL;

// 쉐이더
LPD3DXEFFECT			applyShadowShader = NULL;
LPD3DXEFFECT			createShadowShader = NULL;
LPD3DXEFFECT			applyDisc = NULL;

// 텍스처
LPDIRECT3DTEXTURE9		stoneDM = NULL;
LPDIRECT3DTEXTURE9		stoneSM = NULL;

// 프로그램 이름
const char*				appName = "그림자+법선 매핑";

//카메라 위치
D3DXVECTOR4				worldCameraPosition(200.0f, 200.0f,200.0f, 1.0f);

//빛의 위치
D3DXVECTOR4				worldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

//빛의 색상
D3DXVECTOR4				lightColor(0.7f, 0.7f, 1.0f, 1.0f);

//물체의 색상
D3DXVECTOR4				discColor(0.0f, 1.0f, 1.0f, 1.0f);

//그림자맵 렌더 타깃
LPDIRECT3DTEXTURE9		shadowRenderTarget = NULL;
LPDIRECT3DSURFACE9		shadowDepthStencil = NULL; //그림자 맵을 그릴 때 사용할 깊이 버퍼

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
	case 'W':
		worldCameraPosition.x -= 10.0f;
		break;
	case 'S':
		worldCameraPosition.x += 10.0f;
		break;
	case 'A':
		worldCameraPosition.z -= 10.0f;
		break;
	case 'D':
		worldCameraPosition.z += 10.0f;
		break;
		// ESC 키가 눌리면 프로그램을 종료한다.
	case VK_ESCAPE:
		PostMessage(hWnd, WM_DESTROY, 0L, 0L);
		break;
	}
}

//게임 루프
void PlayDemo()
{
	Update();
	RenderFrame();
}

// 게임로직 업데이트
void Update()
{
}

//렌더링
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
#pragma region 광원-뷰 행렬을 만든다.
	D3DXMATRIXA16 lightViewMatrix;
	{
		D3DXVECTOR3		eye(worldLightPosition.x, worldLightPosition.y, worldLightPosition.z);
		D3DXVECTOR3		at(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3		up(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&lightViewMatrix, &eye, &at, &up);
	}
#pragma endregion

#pragma region 광원-투영 행렬을 만든다.
	D3DXMATRIXA16 lightProjectionMatrix;
	{
		D3DXMatrixPerspectiveFovLH(&lightProjectionMatrix, D3DX_PI / 4.0f, 1, 1, 1000);
	}
#pragma endregion

#pragma region 뷰-투영 행렬을 만든다.
	D3DXMATRIXA16 viewProjectionMatrix;
	{
		//뷰 행렬을 만든다.
		D3DXMATRIXA16 viewMatrix;
		D3DXVECTOR3		eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
		D3DXVECTOR3		at(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3		up(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);
		//투영 행렬을 만든다.
		D3DXMATRIXA16 projectionMatrix;
		D3DXMatrixPerspectiveFovLH(&projectionMatrix, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);
		//뷰-투영행렬 = 뷰 행렬 X 투영 행렬
		D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);
	}
#pragma endregion

#pragma region torus의 월드 행렬을 만든다.
	D3DXMATRIXA16 torusWorldMatrix;
	{
		rotationY += 0.4f*PI / 180.0f;
		if (rotationY > 2 * PI)
		{
			rotationY -= 2 * PI;
		}
		D3DXMatrixRotationY(&torusWorldMatrix, rotationY);
	}
#pragma endregion

#pragma region disc의 월드 행렬을 만든다.
	D3DXMATRIXA16 discWorldMatrix;
	{
		D3DXMATRIXA16 scaleMatrix;
		D3DXMatrixScaling(&scaleMatrix, 2.5f, 2.5f, 2.5f);

		D3DXMATRIXA16 translationMatrix;
		D3DXMatrixTranslation(&translationMatrix, 0.0f, -40.0f, 0.0f);

		D3DXMatrixMultiply(&discWorldMatrix, &scaleMatrix, &translationMatrix);
	}
#pragma endregion

	//백버퍼와 깊이버퍼
	LPDIRECT3DSURFACE9 backBuffer = NULL;
	LPDIRECT3DSURFACE9 depthStencilBuffer = NULL;

	d3dDevice->GetRenderTarget(0, &backBuffer);
	d3dDevice->GetDepthStencilSurface(&depthStencilBuffer);

#pragma region 그림자 만들기
	LPDIRECT3DSURFACE9 shadowSurface = NULL;
	if (SUCCEEDED(shadowRenderTarget->GetSurfaceLevel(0, &shadowSurface)))
	{
		d3dDevice->SetRenderTarget(0, shadowSurface);
		shadowSurface->Release();
		shadowSurface = NULL;
	}
	d3dDevice->SetDepthStencilSurface(shadowDepthStencil);

	//저번 프레임에 그렸던 그림자 정보를 지운다.
	d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0xFFFFFFFF, 1.0f, 0);

	//createShadowShader 전역 변수들을 설정
	createShadowShader->SetMatrix("worldMatrix", &torusWorldMatrix);
	createShadowShader->SetMatrix("lightViewMatrix", &lightViewMatrix);
	createShadowShader->SetMatrix("lightProjectionMatrix", &lightProjectionMatrix);

	//createShadowShader 시작
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
#pragma endregion
	
	//하드웨어 백/깊이 버퍼를 사용
	d3dDevice->SetRenderTarget(0, backBuffer);
	d3dDevice->SetDepthStencilSurface(depthStencilBuffer);

	backBuffer->Release();
	backBuffer = NULL;
	depthStencilBuffer->Release();
	depthStencilBuffer = NULL;

#pragma region 그림자 입히기
	//applyShadowShader 전역 변수들을 설정
	applyShadowShader->SetVector("lightColor", &lightColor);
	applyShadowShader->SetVector("worldLightPosition", &worldLightPosition);
	applyShadowShader->SetVector("worldCameraPosition", &worldCameraPosition);
	applyShadowShader->SetMatrix("worldMatrix", &torusWorldMatrix);

	applyShadowShader->SetMatrix("lightViewMatrix", &lightViewMatrix);
	applyShadowShader->SetMatrix("lightProjectionMatrix", &lightProjectionMatrix);
	applyShadowShader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);
	
	applyShadowShader->SetTexture("diffuseMap_Tex", stoneDM);
	applyShadowShader->SetTexture("specularMap_Tex", stoneSM);
	applyShadowShader->SetTexture("ShadowMap_Tex", shadowRenderTarget);
	//applyShadowShader 시작
	{
		UINT numPasses = 0;
		applyShadowShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; i++)
		{
			applyShadowShader->BeginPass(i);
			torus->DrawSubset(0);
			applyShadowShader->EndPass();
		}
		applyShadowShader->End();
	}
#pragma endregion

#pragma region 디스크 그리기
	//applyDisc 전역 변수들을 설정
	applyDisc->SetVector("worldLightPosition", &worldLightPosition);
	applyDisc->SetMatrix("worldMatrix", &discWorldMatrix);
	applyDisc->SetMatrix("lightViewMatrix", &lightViewMatrix);
	applyDisc->SetMatrix("lightProjectionMatrix", &lightProjectionMatrix);
	applyDisc->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);
	applyDisc->SetTexture("ShadowMap_Tex", shadowRenderTarget);
	applyDisc->SetVector("objectColor", &discColor);

	//applyDisc 시작
	{
		UINT numPasses = 0;
		applyDisc->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; i++)
		{
			applyDisc->BeginPass(i);
			disc->DrawSubset(0);
			applyDisc->EndPass();
		}
		applyDisc->End();
	}
#pragma endregion
}

// 디버그 정보 등을 출력.
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// 텍스트를 출력할 위치
	RECT rct;
	rct.left = 5;
	rct.right = WIN_WIDTH;
	rct.top = 5;
	rct.bottom = WIN_HEIGHT / 3;

	//출력할 텍스트
	char string[100];
	sprintf(string, "ESC: 데모종료\n\n");

	// 키 입력 정보를 출력
	font->DrawText(NULL, string, -1, &rct, 0, fontColor);
}

//초기화 코드
bool InitEverything(HWND hWnd)
{
	// D3D를 초기화
	if (!InitD3D(hWnd))
	{
		return false;
	}

	//그림자맵의 크기 설정
	const int shadowMapSize = 2048;

	//렌더 타깃 생성
	if (FAILED(d3dDevice->CreateTexture(shadowMapSize, shadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &shadowRenderTarget, NULL)))
	{
		return false;
	}

	//그림자맵과 동일한 크기의 깊이 버퍼를 만든다.
	if (FAILED(d3dDevice->CreateDepthStencilSurface(shadowMapSize, shadowMapSize, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, true, &shadowDepthStencil, NULL)))
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
	if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dpp, &d3dDevice)))
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
	// 쉐이더 로딩
	applyShadowShader = LoadShader("ApplyShadow.fx");
	if (!applyShadowShader)
	{
		return false;
	}
	createShadowShader = LoadShader("CreateShadow.fx");
	if (!applyShadowShader)
	{
		return false;
	}
	applyDisc = LoadShader("ApplyDisc.fx");
	if (!applyDisc)
	{
		return false;
	}
	// 모델 로딩
	torus = LoadModel("Torus.x");
	if (!torus)
	{
		return false;
	}
	disc = LoadModel("Disc.x");
	if (!disc)
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

//메모리 해제 코드
void Cleanup()
{
	// 폰트를 release 한다.
	if (font)
	{
		font->Release();
		font = NULL;
	}

	// 모델을 release 한다.
	if (torus)
	{
		torus->Release();
		torus = NULL;
	}
	if (disc)
	{
		disc->Release();
		disc = NULL;
	}

	// 쉐이더를 release 한다.
	if (applyShadowShader)
	{
		applyShadowShader->Release();
		applyShadowShader = NULL;
	}
	if (createShadowShader)
	{
		createShadowShader->Release();
		createShadowShader = NULL;
	}
	if (applyDisc)
	{
		applyDisc->Release();
		applyDisc = NULL;
	}

	// 텍스처를 release 한다.
	if (shadowRenderTarget)
	{
		shadowRenderTarget->Release();
		shadowRenderTarget = NULL;
	}
	if (shadowDepthStencil)
	{
		shadowDepthStencil->Release();
		shadowDepthStencil = NULL;
	}
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

