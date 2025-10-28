/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Implementation of Game.h
----------------------------------------------*/
#include "Game.h"

#include <Core/DXCore.h>
#include <Input/GameInput.h>

#include <Core/Camera.h>
#include <Core/COMException.h>
#include <Core/Factories.h>
#include <Core/PipelineState.h>
#include <Core/ResourceCodex.h>
#include <Core/Shader.h>
#include <Core/hash_util.h>
#include <Utils/Utils.h>

Game::Game() :
    mpInput(new Input::GameInput()),
    mpCamera(nullptr)
{
    mTimer.SetFixedTimeStep(false);
}

bool Game::Init(HWND window, int width, int height)
{
    using namespace Muon;

    bool success = Muon::InitDX12(window, width, height);
    ResourceCodex::Init();

    ResourceCodex& codex = ResourceCodex::GetSingleton();
    const ShaderID kSimpleVSID = fnv1a(L"SimpleVS.cso");
    const ShaderID kSimplePSID = fnv1a(L"SimplePS.cso");
    const VertexShader* pVS = codex.GetVertexShader(kSimpleVSID);
    const PixelShader* pPS = codex.GetPixelShader(kSimplePSID);

    mpCamera = new Camera(DirectX::XMFLOAT3(-5.0, 5.0, -5.0), width / (float)height, 0.1f, 100.0f);

    // Describe and create the graphics pipeline state object (PSO).
    mPSO.SetRootSignature(Muon::GetRootSignature());
    mPSO.SetVertexShader(*pVS);
    mPSO.SetPixelShader(*pPS);
    success = mPSO.Generate();

    struct Vertex
    {
        float Pos[4];
        float Col[4];
    };

    float aspectRatio = width / (float)height;
    Vertex triangleVertices[] =
    {
        { { 0.0f, 0.25f * aspectRatio, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.25f * aspectRatio, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f * aspectRatio, 0.0f, 1.0f}, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    Muon::ResetCommandList(mPSO.GetPipelineState());
    Muon::UploadBuffer& stagingBuffer = codex.GetStagingBuffer();
    stagingBuffer.Map();
    Muon::MeshFactory::LoadAllMeshes(codex);
    mTriangle.Init(triangleVertices, sizeof(triangleVertices), sizeof(Vertex), nullptr, 0, 0, DXGI_FORMAT_R32_UINT);
    stagingBuffer.Unmap(0, stagingBuffer.GetBufferSize());
    Muon::CloseCommandList();
    Muon::ExecuteCommandList();
    return success;
}

// On Timer tick, run Update() on the game, then Render()
void Game::Frame()
{
    mTimer.Tick([&]()
    {
        Update(mTimer);
    });

    Render();

    Muon::UpdateTitleBar(mTimer.GetFramesPerSecond(), mTimer.GetFrameCount());
}

void Game::Update(Muon::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());
    mpInput->Frame(elapsedTime, mpCamera);
    mpCamera->UpdateView();
}

void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (mTimer.GetFrameCount() == 0)
    {
        return;
    }

    Muon::ResetCommandList(mPSO.GetPipelineState());
    Muon::PrepareForRender();
    mPSO.Bind();
    mTriangle.Draw(Muon::GetCommandList());
    Muon::FinalizeRender();
    Muon::CloseCommandList();
    Muon::ExecuteCommandList();
    Muon::Present();
    Muon::FlushCommandQueue();
    Muon::UpdateBackBufferIndex();
}

void Game::CreateDeviceDependentResources()
{
}

void Game::CreateWindowSizeDependentResources(int newWidth, int newHeight)
{
    float aspectRatio = (float)newWidth / (float)newHeight;
    mpCamera->UpdateProjection(aspectRatio);
}

Game::~Game()
{
    delete mpCamera;
    mpCamera = nullptr;
    
    delete mpInput;
    mpInput = nullptr;   

    mTriangle.Release();
    mPSO.Destroy();

    Muon::ResourceCodex::Destroy();
    Muon::DestroyDX12();
}

#pragma region Game State Callbacks
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
    mTimer.ResetElapsedTime();
}

// Recreates Window size dependent resources if needed
void Game::OnMove()
{
}

// Recreates Window size dependent resources if needed
void Game::OnResize(int newWidth, int newHeight)
{
    #if defined(MN_DEBUG)
        try
        {
            CreateWindowSizeDependentResources(newWidth, newHeight);
        }
        catch (std::exception const& e)
        {
            //MessageBoxA(mDeviceResources.GetWindow(), e.what(), "Fatal Exception!", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
            //DestroyWindow(mDeviceResources.GetWindow());
        }
    #else
        CreateWindowSizeDependentResources(newWidth, newHeight);
    #endif
}

void Game::OnMouseMove(short newX, short newY)
{
    mpInput->OnMouseMove(newX, newY);
}
#pragma endregion