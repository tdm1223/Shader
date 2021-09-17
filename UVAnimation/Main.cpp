#include"UVAnimationManager.h"
#include<memory>
#include<stdexcept>

// D3D ����
std::unique_ptr<UVAnimationManager> gManager;

// Ű���� �Է�ó��
void ProcessInput(HWND hWnd, WPARAM keyPress) noexcept
{
    switch (keyPress)
    {
    case 'Q':
        gManager->power += 0.5f;
        break;
    case 'A':
        gManager->power -= 0.5f;
        break;
    case 'W':
        gManager->waveHeight += 0.5f;
        break;
    case 'S':
        gManager->waveHeight -= 0.5f;
        break;
    case 'E':
        gManager->speed += 0.5f;
        break;
    case 'D':
        gManager->speed -= 0.5f;
        break;
    case 'R':
        gManager->waveFrequency += 0.5f;
        break;
    case 'F':
        gManager->waveFrequency -= 0.5f;
        break;
    case 'T':
        gManager->uvSpeed += 0.5f;
        break;
    case 'G':
        gManager->uvSpeed -= 0.5f;
        if (gManager->uvSpeed <= 0) gManager->uvSpeed = 0;
        break;
        // ESC Ű�� ������ ���α׷��� �����Ѵ�.
    case VK_ESCAPE:
        PostMessage(hWnd, WM_DESTROY, 0L, 0L);
        break;
    default:
        break;
    }
}

// �޽��� ó����
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

// ������
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
    // ���α׷� �̸�
    std::string appName = "UVAnimation";

    gManager = std::make_unique<UVAnimationManager>();

    // ������ Ŭ������ ����Ѵ�.
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL, appName.c_str(), NULL };
    RegisterClassEx(&wc);

    // ���α׷� â�� �����Ѵ�.
    constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    HWND hWnd = CreateWindow(appName.c_str(), appName.c_str(), style, CW_USEDEFAULT, 0, Util::WIN_WIDTH, Util::WIN_HEIGHT,
        GetDesktopWindow(), NULL, wc.hInstance, NULL);

    // Client Rect ũ�Ⱑ WIN_WIDTH, WIN_HEIGHT�� ������ ũ�⸦ �����Ѵ�.
    POINT ptDiff;
    RECT rcClient, rcWindow;

    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWindow);
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    MoveWindow(hWnd, rcWindow.left, rcWindow.top, Util::WIN_WIDTH + ptDiff.x, Util::WIN_HEIGHT + ptDiff.y, TRUE);

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    // Init�� shader, model � ���� ������ �������ش�.
    if (!gManager->Init(hWnd, "UVAnimation.fx", "disc.x", "Fieldstone_DM.tga"))
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
        else // �޽����� ������ ������Ʈ�ϰ� ����� �׸���
        {
            gManager->RenderFrame(Util::string_format("Power(Q, A) : % .1f\n\n\
                WaveHeight(W, S) : % .1f\n\n\
                Speed(E, D) : % .1f\n\n\
                WaveFrequency(R, F) : % .1f\n\n\
                UVSpeed(T, G) : % .1f\n\n\
                ESC: ����",
                gManager->power, gManager->waveHeight, gManager->speed, gManager->waveFrequency, gManager->uvSpeed));
        }
    }

    UnregisterClass(appName.c_str(), wc.hInstance);
    return 0;
}