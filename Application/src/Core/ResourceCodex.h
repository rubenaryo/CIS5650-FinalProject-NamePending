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
#include <Core/Buffers.h>

#include <unordered_map>

namespace Muon
{
struct MeshFactory;
struct ShaderFactory;
struct TextureFactory;
}

namespace Muon
{

typedef uint32_t id_type;
typedef id_type ShaderID;
typedef id_type MeshID;
typedef id_type TextureID;
typedef id_type MaterialTypeID;

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
    static MeshID AddMeshFromFile(const char* fileName, const VertexBufferDescription* vertAttr);
    
    // Singleton Stuff
    static void Init();
    static void Destroy();

    static ResourceCodex& GetSingleton();

    const Mesh* GetMesh(MeshID UID) const;
    
    const VertexShader* GetVertexShader(ShaderID UID) const;
    const PixelShader* GetPixelShader(ShaderID UID) const;
    const MaterialType* GetMaterialType(MaterialTypeID UID) const;
    Muon::UploadBuffer& GetMeshStagingBuffer() { return mMeshStagingBuffer; }
    Muon::UploadBuffer& GetMatParamsStagingBuffer() { return mMaterialParamsStagingBuffer; }

private:
    std::unordered_map<ShaderID, VertexShader>  mVertexShaders;
    std::unordered_map<ShaderID, PixelShader>   mPixelShaders;
    std::unordered_map<MeshID, Mesh>            mMeshMap;
    //std::unordered_map<TextureID, ResourceBindChord>   mTextureMap;
    std::unordered_map<MaterialTypeID, MaterialType> mMaterialTypeMap;

    // Materials are queried by index rather than by ID since it's done at runtime 
    // TODO: Need to do this for meshes as well.
    // TODO: Use fixed_vector?
    //std::vector<Material> mMaterials;

    // An intermediate upload buffer used for uploading vertex/index data to the GPU
    Muon::UploadBuffer mMeshStagingBuffer;


    UploadBuffer mMaterialParamsStagingBuffer;
private:
    friend struct TextureFactory;
    //void InsertTexture(TextureID hash, UINT slot, ID3D11ShaderResourceView* pSRV);
    
    friend struct MaterialFactory;
    //MaterialIndex PushMaterial(const Material& material);
    MaterialType* InsertMaterialType(const wchar_t* name);

    friend struct ShaderFactory;
    void AddVertexShader(ShaderID hash, const wchar_t* path);
    void AddPixelShader(ShaderID hash, const wchar_t* path);
};
}
#endif