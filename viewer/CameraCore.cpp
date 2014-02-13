#include "CameraCore.h"

void CameraCore::pan(const Vector& vec)
{
    Position -= vec;
    Focal -= vec;
}
