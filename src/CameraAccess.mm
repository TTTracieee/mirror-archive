#include "CameraAccess.h"
#import <AVFoundation/AVFoundation.h>

namespace mirror {
CameraAccess cameraAccess(){
    const auto status=[AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    if(status==AVAuthorizationStatusAuthorized)return CameraAccess::Allowed;
    if(status==AVAuthorizationStatusNotDetermined)return CameraAccess::Unknown;
    return CameraAccess::Blocked;
}
void requestCameraAccess(){
    // The native app owns the visible permission prompt. Inference starts only
    // after the OS reports consent, never from an unattended worker request.
    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted){}];
}
}
