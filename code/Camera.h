#if !defined(CAMERA_H_)
#define CAMERA_H_

struct camera_t
{
    v3 Position;
    v3 Direction;
    v3 ForwardSpeed;
    r32 Yaw;
    r32 Pitch;
};

extern camera_t Camera;
m4x4 LookAt(v3 Eye, v3 Target, v3 Up);

#endif
