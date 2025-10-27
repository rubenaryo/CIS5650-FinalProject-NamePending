/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Interface for central game class
This class encapsulates all app functionality
----------------------------------------------*/
#ifndef GAME_H
#define GAME_H

#include "StepTimer.h"

#include <Muon/Renderer/Mesh.h>
#include <Muon/Renderer/PipelineState.h>

namespace Renderer
{
class Camera;
}

namespace Input
{
class GameInput;
}

namespace Core {
class Game final
{
public:
    Game();
    ~Game();

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

    // Input Management
    Input::GameInput* mpInput;

    // Main Camera
    Renderer::Camera* mpCamera;

    // TEMP: For testing
    Renderer::Mesh mTriangle;
    Muon::GraphicsPipelineState mPSO;
    
    // Timer for the main game loop
    StepTimer mTimer;
};
}
#endif