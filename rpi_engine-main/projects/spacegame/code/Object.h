#pragma once
#include "render/model.h"
#include "render/physics.h"
#include <render/renderdevice.h>
#include <glm.hpp>

enum class objectType
{
	CASTLE,
	END,
	HOLE_SQUARE,
	STRAIGHT,
	ROUND_CORNER,
	NARROW_SQUARE,
	TUNNEL_DOUBLE,
	WALL_RIGHT,
	WALL_LEFT,
	SPLIT,
	EMPTY,
	FLAG,
};

class Object
{
public:
	objectType objType;
	Render::ModelId model;
	Physics::ColliderMeshId colliderMesh;
	Physics::ColliderId colliderID;
	glm::mat4 transform;
};