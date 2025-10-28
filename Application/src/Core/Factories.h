#ifndef FACTORIES_H
#define FACTORIES_H

#include "DXCore.h"
#include "ResourceCodex.h"
#include "Shader.h"

#include <utility>

namespace Muon
{

struct ShaderFactory final
{
    friend class ResourceCodex;

    // Main initialization function: Takes a codex, which then calls the static functions from ShaderFactory to populate its own hashtables
    static void LoadAllShaders(ResourceCodex& codex);
};

//struct TextureFactory final
//{
//    //typedef std::pair<TextureID, const ResourceBindChord> TexturePair;
//    static void LoadAllTextures(ID3D11Device* device, ID3D11DeviceContext* context, ResourceCodex& codex);
//};

struct MeshFactory final
{
    static MeshID CreateMesh(const char* fileName, const VertexBufferDescription* vertAttr, Mesh& out_meshDX12);
    static void LoadAllMeshes(ResourceCodex& codex);
};

struct MaterialFactory final
{
    static bool CreateAllMaterials(ResourceCodex& codex);
};

}
#endif