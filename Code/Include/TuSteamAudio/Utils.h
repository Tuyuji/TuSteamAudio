/*
* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025+ Reece Hagan
 *
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 */

#pragma once

#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>
#include <phonon.h>

namespace TuSteamAudio
{
    //! Converts an AZ::Vector3 to IPLVector3
    inline IPLVector3 ToIPL(const AZ::Vector3& v)
    {
        return IPLVector3{ v.GetX(), v.GetY(), v.GetZ() };
    }

    //! Converts an AZ::Transform to IPLCoordinateSpace3
    //! @note O3DE uses: X=right, Y=forward, Z=up
    //! @note Steam Audio uses: X=right, Y=up, Z=ahead
    //! This function handles the coordinate system conversion.
    inline IPLCoordinateSpace3 ToIPL(const AZ::Transform& transform)
    {
        IPLCoordinateSpace3 coords = {};

        // Extract basis vectors from the transform
        AZ::Vector3 basisX = transform.GetBasisX(); // right
        AZ::Vector3 basisY = transform.GetBasisY(); // forward in O3DE
        AZ::Vector3 basisZ = transform.GetBasisZ(); // up in O3DE

        // Map O3DE coordinate system to Steam Audio coordinate system
        coords.right = ToIPL(basisX);     // X axis: right (same in both)
        coords.up = ToIPL(basisZ);        // Z axis in O3DE -> Y axis in Steam Audio
        coords.ahead = ToIPL(basisY);     // Y axis in O3DE -> Z axis in Steam Audio
        coords.origin = ToIPL(transform.GetTranslation());

        return coords;
    }

    //! Computes the right vector from forward and up vectors using cross product
    //! Useful when you only have forward and up vectors (like from LabSound listener)
    inline IPLVector3 ComputeRightVector(const IPLVector3& forward, const IPLVector3& up)
    {
        // Cross product: forward × up (left-handed coordinate system)
        IPLVector3 right;
        right.x = forward.y * up.z - forward.z * up.y;
        right.y = forward.z * up.x - forward.x * up.z;
        right.z = forward.x * up.y - forward.y * up.x;
        return right;
    }

} // namespace TuSteamAudio