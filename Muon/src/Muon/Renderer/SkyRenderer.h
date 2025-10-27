/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2020/9
Description : Handles the rendering of the sky backdrop
----------------------------------------------*/
#ifndef SKYRENDERER_H
#define SKYRENDERER_H

#include "DXCore.h"
#include "Material.h"

namespace Renderer
{
struct VertexShader;
struct PixelShader;
struct Mesh;
struct ResourceBindChord;
}

namespace Renderer {

struct SkyRenderer
{
public:
    void Init(ID3D11Device* device);
    void Draw(ID3D11DeviceContext* context);
    
    ~SkyRenderer();

private:
    const Mesh* CubeMesh;
    //Material    SkyMaterialCopy;
};

}
#endif