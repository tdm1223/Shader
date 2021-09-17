#include<d3d9.h>
#include<d3dx9.h>
#include<string>

namespace Util
{
    static constexpr float PI = 3.14159265f;
    static constexpr int NEAR_PLANE = 1;
    static constexpr int FAR_PLANE = 1000;
    static constexpr float FOV = PI / 4.0f;
    static constexpr int WIN_WIDTH = 800;
    static constexpr int WIN_HEIGHT = 600;
    static constexpr float ASPECT_RATIO = WIN_WIDTH / (float)WIN_HEIGHT;

    template<typename ... Args>
    std::string string_format(const std::string& format, Args ... args)
    {
        const int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
        if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
        auto size = static_cast<size_t>(size_s);
        auto buf = std::make_unique<char[]>(size);
        std::snprintf(buf.get(), size, format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
}

class DirectManager
{
public:
    DirectManager() noexcept = default;
    virtual ~DirectManager() noexcept = default;

    // Direct9 장치
    LPDIRECT3D9 d3d = nullptr;
    LPDIRECT3DDEVICE9 d3dDevice = nullptr;

    // 셰이더
    LPD3DXEFFECT shader = nullptr;

    // 물체 색상
    D3DXVECTOR4 objectColor;

    // 폰트
    ID3DXFont* font = nullptr;

    // 모델
    LPD3DXMESH model = nullptr;

    // 텍스처
    LPDIRECT3DTEXTURE9 texture = NULL;

    // 초기화 함수
    virtual bool Init(HWND hWnd, std::string shaderName = "", std::string modelName = "", std::string textureName = "");
    bool InitD3D(HWND hWnd) noexcept;

    // 로딩 관련 함수
    virtual bool LoadAssets(std::string shaderName = "", std::string modelName = "", std::string textureName = "");
    LPD3DXEFFECT LoadShader(std::string filename);
    LPD3DXMESH LoadModel(std::string filename) noexcept;
    LPDIRECT3DTEXTURE9 LoadTexture(std::string filename) noexcept;

    // 렌더링 관련 함수
    void RenderFrame(std::string str);
    void RenderInfo(std::string str) noexcept;
    virtual void RenderScene() = 0;
};

