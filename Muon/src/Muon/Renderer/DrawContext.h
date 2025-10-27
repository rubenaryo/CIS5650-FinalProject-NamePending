#ifndef DRAWCONTEXT_H
#define DRAWCONTEXT_H

#include "DXCore.h"

#include <Muon/Core/UploadBuffer.h>

namespace Renderer {

struct InstancedDrawContext
{
    DirectX::XMFLOAT4X4*    WorldMatrices = nullptr;
    Muon::UploadBuffer      DynamicBuffer;
    MeshID                  InstancedMeshID = 0;
    UINT                    InstanceCount = 0;
    uint32_t                MaterialIndex = 0;
};

}

#endif