#pragma once
namespace mirror {
enum class CameraAccess { Unknown, Allowed, Blocked };
CameraAccess cameraAccess();
void requestCameraAccess();
}
