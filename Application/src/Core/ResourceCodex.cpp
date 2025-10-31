/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/10
Description : Master Resource Distributor
----------------------------------------------*/
#include "ResourceCodex.h"

#include <Core/PathMacros.h>
#include <Utils/Utils.h>

#include "Factories.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"

#include "hash_util.h"

namespace Muon
{
static ResourceCodex* gCodexInstance = nullptr;

MeshID ResourceCodex::AddMeshFromFile(const char* fileName, const VertexBufferDescription* vertAttr)
{
    ResourceCodex& codexInstance = GetSingleton();

    Mesh mesh;

    MeshID id = MeshFactory::CreateMesh(fileName, vertAttr, mesh);
    auto& hashtable = codexInstance.mMeshMap;
    
    if (hashtable.find(id) == hashtable.end())
    {
        codexInstance.mMeshMap.emplace(id, mesh);
    }
    else
    {
        #if defined(MN_DEBUG)
            Muon::Print("ERROR: Tried to insert repeat mesh\n");
        #endif
        assert(false);
    }
    return id;
}

ResourceCodex& ResourceCodex::GetSingleton()
{
    return *gCodexInstance;
}

void ResourceCodex::Init()
{
    if (gCodexInstance)
    {
        Muon::Print("ERROR: Tried to initialize already initialized ResourceCodex!\n");
        return;
    }

    gCodexInstance = new ResourceCodex();
    gCodexInstance->mMeshStagingBuffer.Create(L"Mesh Staging Buffer", 64 * 1024 * 1024);
    ShaderFactory::LoadAllShaders(*gCodexInstance);
    MaterialFactory::CreateAllMaterials(*gCodexInstance);

    //TextureFactory::LoadAllTextures(codexInstance);
}

void ResourceCodex::Destroy()
{
    for (auto& m : gCodexInstance->mMeshMap)
    {
        Mesh& mesh = m.second;
        mesh.Release();
    }
    gCodexInstance->mMeshMap.clear();

    gCodexInstance->mMeshStagingBuffer.Destroy();

    for (auto& m : gCodexInstance->mMaterialTypeMap)
    {
        MaterialType& mat = m.second;
        mat.Destroy();
    }
    gCodexInstance->mMaterialTypeMap.clear();

    for (auto& s : gCodexInstance->mVertexShaders)
    {
        VertexShader& vs = s.second;
        vs.Release();
    }
    gCodexInstance->mVertexShaders.clear();

    for (auto& s : gCodexInstance->mPixelShaders)
    {
        PixelShader& ps = s.second;
        ps.Release();
    }
    gCodexInstance->mPixelShaders.clear();


    delete gCodexInstance;
    gCodexInstance = nullptr;

    // TODO: DX12-ify this.
    //for (auto const& t : codexInstance.mTextureMap)
    //    for(ID3D11ShaderResourceView* srv : t.second.SRVs)
    //        if(srv) srv->Release();
}

const Mesh* ResourceCodex::GetMesh(MeshID UID) const
{
    if(mMeshMap.find(UID) != mMeshMap.end())
        return &mMeshMap.at(UID);
    else
        return nullptr;
}

//const Material* ResourceCodex::GetMaterial(uint8_t materialIndex) const
//{
//    if (materialIndex > mMaterials.size())
//        return nullptr;
//
//    return &mMaterials.at(materialIndex);
//}
//
//const ResourceBindChord* ResourceCodex::GetTexture(TextureID UID) const
//{
//    if(mTextureMap.find(UID) != mTextureMap.end())
//        return &mTextureMap.at(UID);
//    else
//        return nullptr;
//}

const VertexShader* ResourceCodex::GetVertexShader(ShaderID UID) const
{
    if(mVertexShaders.find(UID) != mVertexShaders.end())
        return &mVertexShaders.at(UID);
    else
        return nullptr;
}

const PixelShader* ResourceCodex::GetPixelShader(ShaderID UID) const
{
    if(mPixelShaders.find(UID) != mPixelShaders.end())
        return &mPixelShaders.at(UID);
    else
        return nullptr;
}

const MaterialType* ResourceCodex::GetMaterialType(MaterialTypeID UID) const
{
    if (mMaterialTypeMap.find(UID) != mMaterialTypeMap.end())
        return &mMaterialTypeMap.at(UID);
    else
        return nullptr;
}

void ResourceCodex::AddVertexShader(ShaderID hash, const wchar_t* path)
{   
    mVertexShaders.emplace(hash, path);
}

void ResourceCodex::AddPixelShader(ShaderID hash, const wchar_t* path)
{   
    mPixelShaders.emplace(hash, path);
}

//void ResourceCodex::InsertTexture(TextureID UID, UINT slot, ID3D11ShaderResourceView* pSRV)
//{
//    auto itFind = mTextureMap.find(UID);
//    if (itFind != mTextureMap.end())
//    {
//        ResourceBindChord& chord = itFind->second;
//        if(chord.SRVs[slot])
//            chord.SRVs[slot]->Release();
//
//        mTextureMap[UID].SRVs[slot] = pSRV;
//    }
//    else
//    {
//        // TODO: This could probably avoid creating a temp variable
//        ResourceBindChord rbc = {0};
//        rbc.SRVs[slot] = pSRV;
//        mTextureMap.insert(std::pair<TextureID, ResourceBindChord>(UID, rbc));
//    }
//}

//MaterialIndex ResourceCodex::PushMaterial(const Material& material)
//{
//    mMaterials.push_back(material);
//    return (MaterialIndex)(mMaterials.size() - 1);
//}

MaterialType* ResourceCodex::InsertMaterialType(const wchar_t* name)
{
    if (!name)
        return nullptr;

    MaterialTypeID typeId = fnv1a(name);
    auto emplaceResult = mMaterialTypeMap.emplace(typeId, name);
    if (emplaceResult.second == false)
        return nullptr;

    return &emplaceResult.first->second;
}

}