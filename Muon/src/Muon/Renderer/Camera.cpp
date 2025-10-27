/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/11
Description : Implementation of Camera Class
----------------------------------------------*/
#include "Camera.h"
#include <DirectXMath.h>

namespace Renderer {

using namespace DirectX;

Camera::Camera(XMFLOAT3& pos, float aspectRatio, float nearPlane, float farPlane) :
    mNear(nearPlane),
    mFar(farPlane),
    mSensitivity(0.0f),
    mForward(DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f)),
    mUp(DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f)),
    mRight(DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f)),
    mCameraMode(CM_PERSPECTIVE)
{   
    mPosition = XMLoadFloat3(&pos);
    mTarget = XMVectorZero();

    // Initialize the camera's target and 
    mPosition = XMVectorSet(pos.x, pos.y, pos.z, 0);
    mTarget = XMVectorZero();

    const float CAMERA_DIST = 1.0f;
    XMVECTOR initialPos = XMVectorSet(-CAMERA_DIST, 0.0f, -CAMERA_DIST, 0.0f);
    XMVECTOR initialForward = -initialPos;
    XMVECTOR initialRight = XMVectorSet(-1.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR initialUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    initialPos = XMVector3Rotate(initialPos, XMQuaternionRotationAxis(initialRight, XM_PI / 6));
    initialForward = XMVector3Rotate(initialForward, XMQuaternionRotationAxis(-initialRight, XM_PI / 6));
    initialUp = XMVector3Rotate(initialUp, XMQuaternionRotationAxis(initialRight, XM_PI / 6));

    XMVECTOR verticalOffset = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    mPosition = XMVectorAdd(initialPos, verticalOffset);
    mForward = XMVectorAdd(initialForward, verticalOffset);
    mRight = XMVectorAdd(initialRight, verticalOffset);
    mUp = XMVectorAdd(initialUp, verticalOffset);

    mConstantBuffer.Create(L"CameraConstantBuffer", Muon::GetConstantBufferSize(sizeof(cbCamera)));

    UpdateView();
    UpdateProjection(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::UpdateView()
{
    // Create view matrix
    mView = XMMatrixLookToLH(
        mPosition,
        mForward,
        mUp);

    UpdateConstantBuffer();
}

void Camera::UpdateProjection(float aspectRatio)
{
    switch (mCameraMode)
    {
    case CM_ORTHOGRAPHIC:
    {
        mProjection = XMMatrixOrthographicLH(
            30,
            30,
            mNear,          // Near clip plane
            mFar);          // Far clip plane

        break;
    }
    case CM_PERSPECTIVE:
    {
        mProjection = XMMatrixPerspectiveFovLH(
            XM_PIDIV4,      // FOV
            aspectRatio,    // Screen Aspect ratio
            mNear,          // Near clip plane
            mFar);          // Far clip plane

        break;
    }
    default:
    {
        // Camera mode not set.
        assert(false);
        break;
    }
    }
}

void Camera::GetPosition3A(XMFLOAT3A* out_pos) const
{
    XMStoreFloat3A(out_pos, mPosition);
}

XMVECTOR Camera::GetPosition() const
{
    return mPosition;
}

void Camera::SetTarget(DirectX::XMVECTOR target)
{
    mTarget = target;
}

void Camera::MoveForward(float dist)
{
    MoveAlongAxis(dist, mForward);
}

void Camera::MoveRight(float dist)
{
    MoveAlongAxis(dist, mRight);
}

void Camera::MoveUp(float dist)
{
    MoveAlongAxis(dist, mUp);
}

void Camera::MoveAlongAxis(float dist, XMVECTOR axis)
{
    mPosition = XMVectorAdd(mPosition, XMVectorScale(axis, dist));
}

void Camera::Rotate(XMVECTOR quatRotation)
{
    // Calculate Forward vector
    mForward    = XMVector3Rotate(mForward, quatRotation);
    mUp         = XMVector3Rotate(mUp, quatRotation);
    mRight      = XMVector3Rotate(mRight, quatRotation);
}

void Camera::UpdateConstantBuffer()
{
    cbCamera cb;
    XMStoreFloat4x4(&cb.viewProjection, XMMatrixMultiply(mView, mProjection));

    void* pMappedMemory = mConstantBuffer.Map();
    memcpy(pMappedMemory, &cb, mConstantBuffer.GetBufferSize());
    mConstantBuffer.Unmap(0, mConstantBuffer.GetBufferSize());
}

}
