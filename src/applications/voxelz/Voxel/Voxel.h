#ifndef VOXEL_H_INCLUDED
#define VOXEL_H_INCLUDED

#include "VoxelTypes.h"

struct Voxel
{
    Voxel()
    {
        active=false;
        transparent=false;
        solid=false;
        color=VecRGBToIntRGB(u8vec3(255));
    }

    bool operator ==(const Voxel& other) const
    {
        return active==other.active && transparent==other.transparent && solid==other.solid && color==other.color;
    }

    bool operator !=(const Voxel& other) const
    {
        return active!=other.active || transparent!=other.transparent || solid!=other.solid || color!=other.color;
    }

    bool active;
    bool transparent;
    bool solid;
    intRGBA color;
};

#endif // VOXEL_H_INCLUDED
