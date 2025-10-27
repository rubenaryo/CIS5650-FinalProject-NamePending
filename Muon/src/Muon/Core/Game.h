/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Interface for central game class
This class encapsulates all app functionality
----------------------------------------------*/
#ifndef GAME_H
#define GAME_H

#include "StepTimer.h"

#include <Muon/Renderer/DeviceResources.h>
#include <Muon/Renderer/EntityRenderer.h>
#include <Muon/Renderer/SkyRenderer.h>

namespace Renderer
{
class Camera;
class LightingManager;
}

namespace Input
{
class GameInput;
}

namespace Core {
class Game final : public Renderer::IDeviceNotify
{
public:
    Game();
    ~Game();
    //bool Init(HWND window, int width, int height);

    bool InitDX12(HWND window, int width, int height);

    // Main Game Loop
    void Frame();

    // Implementation of IDeviceNotify
    // Handles sudden loss of device
    virtual void OnDeviceLost();
    virtual void OnDeviceRestored();

    // Callbacks for windows messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnMove();
    void OnResize(int newWidth, int newHeight);

    // Input Callbacks
    void OnMouseMove(short newX, short newY);

private:
    void Update(StepTimer const& timer);
    void Render();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources(int newWidth, int newHeight);

    // Application's Device Resources, such as the necessary buffers/views in video memory
    //Renderer::DeviceResources mDeviceResources;

    // Renderer for handling smart binding of objects
    Renderer::EntityRenderer mEntityRenderer;

    // Handles the drawing of the skybox
    Renderer::SkyRenderer mSkyRenderer;

    // Lights Manager
    Renderer::LightingManager* mpLightingManager;

    // Input Management
    Input::GameInput* mpInput;

    // Main Camera
    Renderer::Camera* mpCamera;
    
    // Timer for the main game loop
    StepTimer mTimer;
};
}
#endif