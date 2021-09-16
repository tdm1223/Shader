#pragma once
#include "DirectManager.h"
class ColorShaderDirectManager : public DirectManager
{
public:
    ColorShaderDirectManager() noexcept = default;
    ColorShaderDirectManager(D3DXVECTOR4 d3dvector) noexcept;
    ~ColorShaderDirectManager() = default;

    void RenderScene() override;
};

