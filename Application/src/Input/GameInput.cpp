/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/11
Description : Implementation of GameInput method overrides
----------------------------------------------*/
#include <Core/WinApp.h>

#include "InputSystem.h"
#include "GameInput.h"

namespace Input {

    GameInput::GameInput()
    {
        SetDefaultKeyMap();
    }

    GameInput::~GameInput()
    {}

    void GameInput::Frame(float dt, Renderer::Camera* pCamera)
    {
        using namespace DirectX;

        static const float kSpeed = 5.0f;

        // Act on user input:
        // - Iterate through all active keys
        // - Check for commands corresponding to activated chords
        // - Do something based on those commands
        for (auto const& pair : mActiveKeyMap)
        {
            switch (pair.first)
            {
            case GameCommands::Quit:
                PostQuitMessage(0);
                break;
            case GameCommands::MoveForward:
                pCamera->MoveForward(kSpeed * dt);
                break;
            case GameCommands::MoveBackward:
                pCamera->MoveForward(-kSpeed * dt);
                break;
            case GameCommands::MoveLeft:
                pCamera->MoveRight(-kSpeed * dt);
                break;
            case GameCommands::MoveRight:
                pCamera->MoveRight(kSpeed * dt);
                break;
            case GameCommands::MoveUp:
                pCamera->MoveUp(kSpeed * dt);
                break;
            case GameCommands::RollLeft:
                pCamera->Rotate(XMQuaternionRotationAxis(pCamera->mForward, kSpeed*dt));
                break;
            case GameCommands::RollRight:
                pCamera->Rotate(XMQuaternionRotationAxis(pCamera->mForward, -kSpeed * dt));
                break;
            case GameCommands::CameraRotation:
            {
                std::pair<float, float> delta = this->GetMouseDelta();

                const float mouseSensitivity = pCamera->GetSensitivity();

                const float yaw  = delta.first * mouseSensitivity * dt;
                const float pitch = delta.second * mouseSensitivity * dt;

                XMVECTOR horizontalQuat = XMQuaternionRotationAxis(pCamera->mUp, yaw);
                XMVECTOR verticalQuat   = XMQuaternionRotationAxis(pCamera->mRight, pitch);
                
                pCamera->Rotate(XMQuaternionMultiply(horizontalQuat, verticalQuat));
                break;
            }
            }
        }
        
        GetInput();
    }

    void GameInput::SetDefaultKeyMap()
    {
        mKeyMap.clear();
        mKeyMap[GameCommands::Quit]         = new Chord(L"Quit", VK_ESCAPE, KeyState::JustReleased);
        mKeyMap[GameCommands::MoveForward]  = new Chord(L"Move Forward", 'W', KeyState::StillPressed);
        mKeyMap[GameCommands::MoveBackward] = new Chord(L"Move Backward", 'S', KeyState::StillPressed);
        mKeyMap[GameCommands::MoveLeft]     = new Chord(L"Move Left", 'A', KeyState::StillPressed);
        mKeyMap[GameCommands::MoveRight]    = new Chord(L"Move Right", 'D', KeyState::StillPressed);
        mKeyMap[GameCommands::MoveUp]       = new Chord(L"Move Up", VK_SPACE, KeyState::StillPressed);
        mKeyMap[GameCommands::RollLeft]     = new Chord(L"Roll Left", 'Q', KeyState::StillPressed);
        mKeyMap[GameCommands::RollRight]    = new Chord(L"Roll Right", 'E', KeyState::StillPressed);

        mKeyMap[GameCommands::CameraRotation] = new Chord(L"Camera Rotation", VK_RBUTTON, KeyState::StillPressed);
    }
}
