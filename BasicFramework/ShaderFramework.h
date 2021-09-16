#pragma once

#include<d3d9.h>
#include<d3dx9.h>
#include<string>

//선언
constexpr int WIN_WIDTH = 800;
constexpr int WIN_HEIGHT = 600;

// 메시지 처리기 관련
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
void ProcessInput(HWND hWnd, WPARAM keyPress) noexcept;

// 초기화 관련
bool InitEverything(HWND hWnd);
bool InitD3D(HWND hWnd) noexcept;
bool LoadAssets();

LPD3DXEFFECT LoadShader(std::string filename);
LPDIRECT3DTEXTURE9 LoadTexture(std::string filename) noexcept;
LPD3DXMESH LoadModel(std::string filename) noexcept;

// 게임루프 관련
void PlayDemo();
void Update() noexcept;

// 렌더링 관련
void RenderFrame();
void RenderScene();
void RenderInfo() noexcept;

// 메모리 해제 관련
void Cleanup() noexcept;