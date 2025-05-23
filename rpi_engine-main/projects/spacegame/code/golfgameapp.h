#pragma once
//------------------------------------------------------------------------------
/**
	Space game application

	(C) 20222 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------
#include "core/app.h"
#include "render/window.h"
#include "map.h"
#include "Ball.h"

namespace Game
{
class GolfGameApp : public Core::App
{
public:
	/// constructor
	GolfGameApp();
	/// destructor
	~GolfGameApp();

	/// open app
	bool Open();
	/// run app
	void Run();
	/// exit app
	void Exit();
public:

	/// show some ui things
	void RenderUI();
	/// render some nanovg stuff
	void RenderNanoVG(NVGcontext* vg);

	Display::Window* window;
	glm::vec3 ballPos;
	bool aimMode;
	float debugYaw;
	float debugPitch;
	bool debugControlMode = false;
	int debugHighscore = 0;
	int debugShotCounter = 0;

	Ball ball;
	std::vector<Map> maps;
	int mapIndex = 0;


	bool isDisplayWin = false;
	float winTimer = 5.0f;
};
} // namespace Game