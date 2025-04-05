/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/10
Description : Loads and distributes all static resources (materials, textures, etc)
----------------------------------------------*/
#ifndef RESOURCECODEX_H
#define RESOURCECODEX_H

#include "DXCore.h"

#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include <Muon/Core/UploadBuffer.h>

#include <unordered_map>

namespace Renderer {

struct MeshFactory;
struct ShaderFactory;
struct TextureFactory;
}

namespace Renderer {

typedef uint32_t id_type;
typedef id_type ShaderID;
typedef id_type MeshID;
typedef id_type TextureID;

enum MaterialIndex
{
    MI_LUNAR = 0,
    MI_SKY,
    MI_WIREFRAME,
    MI_COUNT
};

class alignas(8) ResourceCodex
{
public:
    static MeshID AddMeshFromFile(const char* fileName, const VertexBufferDescription* vertAttr, ID3D11Device* pDevice);
    
    // Singleton Stuff
    static void Init();
    static void Destroy();

    inline static ResourceCodex& GetSingleton() { static ResourceCodex codexInstance; return codexInstance; }

    //const Mesh* GetMesh(MeshID UID) const;
    const Mesh_DX12* GetMesh(MeshID UID) const;
    const Material* GetMaterial(uint8_t materialIndex) const;
    const ResourceBindChord* GetTexture(TextureID UID) const;
    const VertexShader* GetVertexShader(ShaderID UID) const;
    const PixelShader* GetPixelShader(ShaderID UID) const;
    Muon::UploadBuffer& GetStagingBuffer() { return mMeshStagingBuffer; }

private:

    std::unordered_map<ShaderID, VertexShader>  mVertexShaders;
    std::unordered_map<ShaderID, PixelShader>   mPixelShaders;
    std::unordered_map<MeshID, Mesh_DX12>            mMeshMap;
    std::unordered_map<TextureID, ResourceBindChord>   mTextureMap;

    // Materials are queried by index rather than by ID since it's done at runtime 
    // TODO: Need to do this for meshes as well.
    // TODO: Use fixed_vector?
    std::vector<Material> mMaterials;

    // An intermediate upload buffer used for uploading vertex/index data to the GPU
    Muon::UploadBuffer mMeshStagingBuffer;

    // Singleton stuff
    static ResourceCodex* CodexInstance;

private:
    friend struct TextureFactory;
    void InsertTexture(TextureID hash, UINT slot, ID3D11ShaderResourceView* pSRV);
    
    friend struct MaterialFactory;
    MaterialIndex PushMaterial(const Material& material);

    friend struct ShaderFactory;
    void AddVertexShader(ShaderID hash, const wchar_t* path);
    void AddPixelShader(ShaderID hash, const wchar_t* path);
};
}
#endif