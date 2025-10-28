/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/12
Description : Implementation of Message Loop
----------------------------------------------*/
#include <Muon/Core/Game.h>

#include "GameWindow.h"

namespace Core
{
    GameWindow::GameWindow()
    {
        m_pGame = new Core::Game();
    }

    GameWindow::~GameWindow()
    {
        delete m_pGame;
        m_pGame = nullptr;
    }

    LRESULT GameWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_MOUSEMOVE:
            POINTS pt = MAKEPOINTS(lParam);
            m_pGame->OnMouseMove(pt.x, pt.y);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
        case WM_MOVE:
        case WM_SIZE:
            // If the window has just been minimized
            if (wParam == SIZE_MINIMIZED)
            {
                if (!m_Minimized)
                {
                    // Set minimized flag, suspend the game
                    m_Minimized = true;
                    if (!m_Suspended)
                        m_pGame->OnSuspending();
                    m_Suspended = true;
                }
            }
            else if (m_Minimized) // if we're minimized already, must be maximizing
            {
                // Set maximized flag, resume the game
                m_Minimized = false;
                if (m_Suspended)
                    m_pGame->OnResuming();
                m_Suspended = false;
            }
            break;
        case WM_ENTERSIZEMOVE:
            m_ResizeMove = true;
            break;
        case WM_EXITSIZEMOVE:
        {
            m_ResizeMove = false;
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            m_pGame->OnResize(static_cast<int>(rc.right - rc.left), static_cast<int>(rc.bottom - rc.top));
        }
        break;
        case WM_GETMINMAXINFO:
        {
            // lParam represents default positions, dimensions, tracking sizes
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);

            // Override these values so the client area doesn't get too small
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;
        case WM_ACTIVATEAPP:
            if (m_pGame)
            {
                if (wParam)
                    m_pGame->OnActivated();
                else
                    m_pGame->OnDeactivated();
            }
            break;
        case WM_POWERBROADCAST:
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                if (!m_Suspended)
                    m_pGame->OnSuspending();
                m_Suspended = true;
                return TRUE;
            case PBT_APMRESUMESUSPEND:
                if (!m_Minimized)
                {
                    if (m_Suspended)
                        m_pGame->OnResuming();
                    m_Suspended = false;
                }
                return TRUE;
            }
            break;
        case WM_MENUCHAR:
            return MAKELRESULT(0, 1);
        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
        }
        return TRUE;
    }

    bool GameWindow::InitGame(HWND hwnd, int width, int height)
    {
#if defined(MN_DEBUG)
        bool result = false;
        try
        {
            result = m_pGame->InitDX12(hwnd, width, height);
        }
        catch (std::exception const& e)
        {
            MessageBoxA(hwnd, e.what(), "Fatal Exception!", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
            DestroyWindow(hwnd);
        }
        return result;
#else
        return m_pGame->InitDX12(hwnd, width, height);
#endif
    }

    // Here we invoke the window procedure to handle windows messages
    // if no message is received, we process a frame
    void GameWindow::RunGame()
    {
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            // Process one gameplay frame
            m_pGame->Frame();
        }
    }

}