#include "UVAnimation.h"
#include <cstdio>

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
ID3DXFont*              font1 = NULL;
ID3DXFont*              font2 = NULL;

// 모델
LPD3DXMESH				disc = NULL;

// 셰이더
LPD3DXEFFECT			uvAnimationShader = NULL;

// 텍스처
LPDIRECT3DTEXTURE9		stoneDM = NULL;
LPDIRECT3DTEXTURE9		stoneSM = NULL;

// 프로그램 이름
const char*				appName = "UVAnimation Framework";

//카메라 위치
D3DXVECTOR4				worldCameraPosition(200.0f, 100.0f, -200.0f, 1.0f);

//빛의 위치
D3DXVECTOR4				worldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

//빛의 색상
D3DXVECTOR4				lightColor(0.7f, 0.7f, 1.0f, 1.0f);

//셰이더에 넘겨줄 전역변수
float					power = 20.0f; //정반사광 계산시 지수로 사용할 수 
float					waveHeight = 3.0f; //출렁이는 웨이브 높이
float					speed = 2.0f; //출렁이는 웨이브 속도
float					waveFrequency = 10.0f; //웨이브 빈도
float					uvSpeed = 0.25f; //웨이브의 이동 속도

// 진입점
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	// 윈도우 클래스를 등록한다.
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,appName, NULL };
	RegisterClassEx(&wc);

	// 프로그램 창을 생성한다.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(appName, appName, style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
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
	case 'Q':
		power += 0.1f;
		break;
	case 'A':
		power -= 0.1f;
		break;
	case 'W':
		waveHeight += 0.1f;
		break;
	case 'S':
		waveHeight -= 0.1f;
		break;
	case 'E':
		speed += 0.1f;
		break;
	case 'D':
		speed -= 0.1f;
		break;
	case 'R':
		waveFrequency += 0.1f;
		break;
	case 'F':
		waveFrequency -= 0.1f;
		break;
	case 'T':
		uvSpeed += 0.1f;
		break;
	case 'G':
		uvSpeed -= 0.1f;
		if (uvSpeed <= 0) uvSpeed = 0;
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
	D3DCOLOR bgColour = 0xFF0000FF;	// 배경색상 - 파랑

	d3dDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColour, 1.0f, 0);

	d3dDevice->BeginScene();
	{
		RenderScene();				// 3D 물체등을 그린다.
		RenderInfo();				// 디버그 정보 등을 출력한다.
	}
	d3dDevice->EndScene();

	d3dDevice->Present(NULL, NULL, NULL, NULL);
}

// 3D 물체를 그린다.
void RenderScene()
{
	// 월드행렬을 만든다.
	D3DXMATRIXA16			worldMatrix;
	D3DXMatrixRotationY(&worldMatrix, rotationY);

	// 뷰 행렬을 만든다.
	D3DXMATRIXA16			viewMatrix;
	D3DXVECTOR3 eye(worldCameraPosition.x, worldCameraPosition.y, worldCameraPosition.z);
	D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&viewMatrix, &eye, &lookAt, &up);

	// 투영행렬을 만든다.
	D3DXMATRIXA16			projectionMatrix;
	D3DXMatrixPerspectiveFovLH(&projectionMatrix, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

	//뷰 X 투영행렬을 만든다.
	D3DXMATRIXA16			viewProjectionMatrix;
	D3DXMatrixMultiply(&viewProjectionMatrix, &viewMatrix, &projectionMatrix);

	// 정점 셰이더 전역변수들을 설정
	uvAnimationShader->SetMatrix("worldMatrix", &worldMatrix);
	uvAnimationShader->SetMatrix("viewProjectionMatrix", &viewProjectionMatrix);
	uvAnimationShader->SetVector("worldLightPosition", &worldLightPosition);
	uvAnimationShader->SetVector("worldCameraPosition", &worldCameraPosition);
	//시스템 시간을 구한다.
	ULONGLONG tick = GetTickCount64();
	uvAnimationShader->SetFloat("time", tick / 1000.0f);
	uvAnimationShader->SetFloat("waveHeight", waveHeight);
	uvAnimationShader->SetFloat("speed", speed);
	uvAnimationShader->SetFloat("waveFrequency", waveFrequency);
	uvAnimationShader->SetFloat("uvSpeed", uvSpeed);
	
	// 픽셀 셰이더 전역변수들을 설정
	uvAnimationShader->SetFloat("power", power);
	uvAnimationShader->SetTexture("diffuseMap_Tex", stoneDM);
	uvAnimationShader->SetTexture("specularMap_Tex", stoneSM);
	uvAnimationShader->SetVector("lightColor", &lightColor);

	UINT numPasses = 0;
	uvAnimationShader->Begin(&numPasses, NULL);
	{
		for (UINT i = 0; i < numPasses; i++)
		{
			uvAnimationShader->BeginPass(i);
			{
				disc->DrawSubset(0);
			}
			uvAnimationShader->EndPass();
		}
	}
	uvAnimationShader->End();
}

// 디버그 정보 출력.
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// 텍스트를 출력할 위치
	RECT rect1;
	rect1.left = 5;
	rect1.right = WIN_WIDTH;
	rect1.top = 5;
	rect1.bottom = WIN_HEIGHT;

	//출력할 텍스트
	char string[200];
	sprintf_s(string,
		"Power(Q,A) : %.1f\n\n\
		WaveHeight(W,S) : %.1f\n\n\
		Speed(E,D) : %.1f\n\n\
		WaveFrequency(R,F) : %.1f\n\n\
		UVSpeed(T,G) : %.1f\n\n\
		ESC: 종료",
		power, waveHeight, speed, waveFrequency, uvSpeed);

	// 키 입력 정보를 출력
	font1->DrawText(NULL, string, -1, &rect1, 0, fontColor);
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
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_DONTCARE), "Arial", &font1)))
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
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice)))
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

	// 셰이더 로딩
	uvAnimationShader = LoadShader("UVAnimation.fx");
	if (!uvAnimationShader)
	{
		return false;
	}

	// 모델 로딩
	disc = LoadModel("disc.x");
	if (!disc)
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

	D3DXCreateEffectFromFile(d3dDevice, filename, NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

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
	if (font1)
	{
		font1->Release();
		font1 = NULL;
	}
	if (font2)
	{
		font2->Release();
		font2 = NULL;
	}

	// 모델을 release 한다.
	if (disc)
	{
		disc->Release();
		disc = NULL;
	}

	// 셰이더를 release 한다.
	if (uvAnimationShader)
	{
		uvAnimationShader->Release();
		uvAnimationShader = NULL;
	}

	// 텍스처를 release 한다.
	if (stoneSM)
	{
		stoneSM->Release();
		stoneSM = NULL;
	}
	if (stoneDM)
	{
		stoneDM->Release();
		stoneDM = NULL;
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

