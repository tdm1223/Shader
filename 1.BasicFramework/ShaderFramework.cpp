#include"ShaderFramework.h"
#include<cstdio>

// 전역변수
#define PI				3.14159265f
#define FOV				(PI/4.0f)
#define ASPECT_RATIO	(WIN_WIDTH/(float)WIN_HEIGHT)
#define NEAR_PLANE		1
#define FAR_PLANE		1000

// D3D 관련
LPDIRECT3D9             d3d = NULL;				// D3D
LPDIRECT3DDEVICE9       d3dDevice = NULL;				// D3D 장치
	
// 폰트
ID3DXFont*              font = NULL;

// 모델
LPD3DXMESH				sphere = NULL;

// 셰이더
LPD3DXEFFECT			colorShader = NULL;

// 물체 색상
D3DXVECTOR4				objectColor(1.0f, 0.0f, 0.0f, 1);

// 프로그램 이름
const char*				appName = "셰이더 데모 프레임워크";

// 진입점
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스를 등록한다.
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,appName, NULL };
	RegisterClassEx(&wc);

	// 프로그램 창을 생성한다.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(appName, appName,style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
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
	{
		PostQuitMessage(1);
	}

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
		else // 메시지가 없으면 업데이트하고 장면을 그린다
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
	if (GetAsyncKeyState('Q') < 0)
	{
		objectColor.x += 0.01f;
		if (objectColor.x >= 1)
		{
			objectColor.x = 1.0f;
		}
	}
	if (GetAsyncKeyState('A') < 0)
	{
		objectColor.x -= 0.01f;
		if (objectColor.x <= 0)
		{
			objectColor.x = 0.0f;
		}
	}
	if (GetAsyncKeyState('W') < 0)
	{
		objectColor.y += 0.01f;
		if (objectColor.y >= 1)
		{
			objectColor.y = 1.0f;
		}
	}
	if (GetAsyncKeyState('S') < 0)
	{
		objectColor.y -= 0.01f;
		if (objectColor.y <= 0)
		{
			objectColor.y = 0.0f;
		}
	}
	if (GetAsyncKeyState('E') < 0)
	{
		objectColor.z += 0.01f;
		if (objectColor.z >= 1)
		{
			objectColor.z = 1.0f;
		}
	}
	if (GetAsyncKeyState('D') < 0)
	{
		objectColor.z -= 0.01f;
		if (objectColor.z <= 0)
		{
			objectColor.z = 0.0f;
		}
	}
}

//렌더링
void RenderFrame()
{
	D3DCOLOR bgColour = 0xFF0000FF;	// 배경색상 - 파랑

	d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

	d3dDevice->BeginScene();
	{
		RenderScene();				// 3D 물체를 그린다.
		RenderInfo();				// 디버그 정보를 출력한다.
	}
	d3dDevice->EndScene();

	d3dDevice->Present(NULL, NULL, NULL, NULL);
}


// 3D 물체를 그린다.
void RenderScene()
{
	//월드 행렬
	D3DXMATRIXA16 worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);

	//뷰 행렬
	D3DXMATRIXA16 viewMatrix;
	D3DXVECTOR3 eye(0.0f, 0.0f, -200.0f);
	D3DXVECTOR3 at(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

	//투영 행렬
	D3DXMATRIXA16 projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

	//위의 셋을 곱해서 월드-뷰-투영 행렬을 만듦.
	D3DXMATRIXA16 worldViewMatrix;
	D3DXMATRIXA16 worldViewProjectionMatrix;
	D3DXMatrixMultiply(&worldViewMatrix, &worldMatrix, &viewMatrix);
	D3DXMatrixMultiply(&worldViewProjectionMatrix, &worldViewMatrix, &projectionMatrix);

	//셰이더의 전역변수 설정
	colorShader->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
	colorShader->SetVector("objectColor", &objectColor);

	UINT numPasses = 0;
	colorShader->Begin(&numPasses, NULL);
	for (UINT i = 0; i < numPasses; i++)
	{
		colorShader->BeginPass(i);
		sphere->DrawSubset(0);
		colorShader->EndPass();
	}
	colorShader->End();
}

// 디버그 정보를 출력.
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// 텍스트를 출력할 위치
	RECT rct;
	rct.left = 5;
	rct.right = WIN_WIDTH / 3;
	rct.top = 5;
	rct.bottom = WIN_HEIGHT / 3;

	//출력할 텍스트
	char string[100];
	sprintf(string, "R.G.B : (%.1f,%.1f,%.1f)\nESC: 종료\nQ: R증가\nA: R감소\nW: G증가\nS: G감소\nE: R증가\nD: R감소", objectColor.x, objectColor.y, objectColor.z);

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

	// 모델, 셰이더, 텍스처등을 로딩
	if (!LoadAssets())
	{
		return false;
	}

	// 폰트를 로딩
	if (FAILED(D3DXCreateFont(d3dDevice, 20, 10, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE),"Arial", &font)))
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
		D3DCREATE_HARDWARE_VERTEXPROCESSING,&d3dpp, &d3dDevice)))
	{
		return false;
	}

	return true;
}

bool LoadAssets()
{
	// 셰이더 로딩
	colorShader = LoadShader("ColorShader.fx");
	if (!colorShader)
	{
		return false;
	}

	// 모델 로딩
	sphere = LoadModel("sphere.x");
	if (!sphere)
	{
		return false;
	}

	return true;
}

// 셰이더 로딩
LPD3DXEFFECT LoadShader(const char * filename)
{
	LPD3DXEFFECT ret = NULL;
	LPD3DXBUFFER pError = NULL;
	DWORD dwShaderFlags = 0;

#if _DEBUG
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

	D3DXCreateEffectFromFile(d3dDevice, filename,NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

	// 셰이더 로딩에 실패한 경우 output창에 셰이더 컴파일 에러를 출력한다.
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
	if (sphere)
	{
		sphere->Release();
		sphere = NULL;
	}

	// 셰이더를 release 한다.
	if (colorShader)
	{
		colorShader->Release();
		colorShader = NULL;
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

