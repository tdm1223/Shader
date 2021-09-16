#include"ColorShaderDirectManager.h"
#include<memory>
#include<stdexcept>

// D3D 관련
std::unique_ptr<ColorShaderDirectManager> gManager;

// 키보드 입력처리
void ProcessInput(HWND hWnd, WPARAM keyPress) noexcept
{
    switch (keyPress)
    {
        // ESC 키가 눌리면 프로그램을 종료한다.
    case VK_ESCAPE:
        PostMessage(hWnd, WM_DESTROY, 0L, 0L);
        break;
    default:
        break;
    }
}

// 메시지 처리기
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg)
    {
    case WM_KEYDOWN:
        ProcessInput(hWnd, wParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 진입점
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    // 프로그램 이름
    std::string appName = "Color Shader";

    gManager = std::make_unique<ColorShaderDirectManager>(D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f));

    // 윈도우 클래스를 등록한다.
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL, appName.c_str(), NULL };
    RegisterClassEx(&wc);

    // 프로그램 창을 생성한다.
    constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HWND hWnd = CreateWindow(appName.c_str(), appName.c_str(), style, CW_USEDEFAULT, 0, Util::WIN_WIDTH, Util::WIN_HEIGHT,
        GetDesktopWindow(), NULL, wc.hInstance, NULL);

    // Client Rect 크기가 WIN_WIDTH, WIN_HEIGHT와 같도록 크기를 조정한다.
    POINT ptDiff;
    RECT rcClient, rcWindow;

    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWindow);
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    MoveWindow(hWnd, rcWindow.left, rcWindow.top, Util::WIN_WIDTH + ptDiff.x, Util::WIN_HEIGHT + ptDiff.y, TRUE);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    // Init시 shader, model 등에 대한 정보를 전달해준다.
    if (!gManager->Init(hWnd, "ColorShader.fx", "sphere.x", ""))
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
            gManager->RenderFrame(Util::string_format("R.G.B : (% .1f, % .1f, % .1f)\nESC : 종료\nQ : R증가\nA : R감소\nW : G증가\nS : G감소\nE : R증가\nD : R감소", gManager->objectColor.x, gManager->objectColor.y, gManager->objectColor.z));
            if (GetAsyncKeyState('Q') < 0)
            {
                gManager->objectColor.x += 0.01f;
                if (gManager->objectColor.x >= 1)
                {
                    gManager->objectColor.x = 1.0f;
                }
            }
            if (GetAsyncKeyState('A') < 0)
            {
                gManager->objectColor.x -= 0.01f;
                if (gManager->objectColor.x <= 0)
                {
                    gManager->objectColor.x = 0.0f;
                }
            }
            if (GetAsyncKeyState('W') < 0)
            {
                gManager->objectColor.y += 0.01f;
                if (gManager->objectColor.y >= 1)
                {
                    gManager->objectColor.y = 1.0f;
                }
            }
            if (GetAsyncKeyState('S') < 0)
            {
                gManager->objectColor.y -= 0.01f;
                if (gManager->objectColor.y <= 0)
                {
                    gManager->objectColor.y = 0.0f;
                }
            }
            if (GetAsyncKeyState('E') < 0)
            {
                gManager->objectColor.z += 0.01f;
                if (gManager->objectColor.z >= 1)
                {
                    gManager->objectColor.z = 1.0f;
                }
            }
            if (GetAsyncKeyState('D') < 0)
            {
                gManager->objectColor.z -= 0.01f;
                if (gManager->objectColor.z <= 0)
                {
                    gManager->objectColor.z = 0.0f;
                }
            }
        }
    }

    UnregisterClass(appName.c_str(), wc.hInstance);
    return 0;
}