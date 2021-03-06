// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

#include "Net/UnrealNetwork.h"	

/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1

/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.PhysicsSettings] should match with this list **/

#define SURFACE_DEFAULT				SurfaceType_Default
#define SURFACE_BODY				SurfaceType1
#define SURFACE_HEAD				SurfaceType2
#define SURFACE_LIMB				SurfaceType3