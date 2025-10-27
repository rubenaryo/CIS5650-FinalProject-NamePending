/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/3
Description : Manager-level class for intelligently binding and drawing objects
----------------------------------------------*/
#ifndef RENDERER_H
#define RENDERER_H

#include <Muon/Core/Transform.h>

#include "CBufferStructs.h"
#include "ConstantBuffer.h"
#include "DXCore.h"
#include "ResourceCodex.h"

namespace Renderer
{
    class DeviceResources;
    class Camera;

    struct InstancedDrawContext;
}

namespace Renderer {

struct Entity
{
    Core::Transform mTransform;
    MeshID          mMeshID;
    uint32_t        MaterialIndex;
};

class EntityRenderer
{
public:
    EntityRenderer();
    ~EntityRenderer();

    void Init();

    // For now, the renderer will handle updating the entities, 
    // In the future, perhaps a Physics Manager or AI Manager would be a good solution?
    void Update(float dt);

    // Binds the fields necessary in the material, then draws every entity in m_EntityMap
    void Draw();

private:
    // Performs all the instanced draw steps
    void InstancedDraw();
    
    // Loads the necessary models into a collection
    void InitMeshes();

    // Populates the Entity List
    void InitEntities();

    // Creates the necessary material keys within m_Map, 
    void InitDrawContexts();

private:

    // All the Entities
    Entity*  Entities;
    UINT     EntityCount;

    // Array of Instancing Information
    InstancedDrawContext* InstancingPasses;
    UINT                  InstancingPassCount;

    // Constant Buffer that holds material parameters
    //ConstantBufferBindPacket MaterialParamsCB;

    // Constant Buffer that holds non-instanced entity world matrices
    //ConstantBufferBindPacket EntityCB;

public: // Enforce use of the default constructor
    EntityRenderer(EntityRenderer const&)               = delete;
    EntityRenderer& operator=(EntityRenderer const&)    = delete;
};

}
#endif