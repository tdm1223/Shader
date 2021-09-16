#pragma once
#include "DirectManager.h"

class TextureMappingDirectManager : public DirectManager
{
public:
    // 회전값
    float rotationY = 0.0f;

    void RenderScene() override;
};

