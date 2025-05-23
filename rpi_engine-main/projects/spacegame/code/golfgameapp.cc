//------------------------------------------------------------------------------
// spacegameapp.cc
// (C) 2022 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "golfgameapp.h"
#include <cstring>
#include "imgui.h"
#include "render/renderdevice.h"
#include "render/shaderresource.h"
#include <vector>
#include "render/textureresource.h"
#include "render/model.h"
#include "render/cameramanager.h"
#include "render/lightserver.h"
#include "render/debugrender.h"
#include "core/random.h"
#include "render/input/inputserver.h"
#include "core/cvar.h"
#include "render/physics.h"
#include <chrono>
#include "spaceship.h"



using namespace Display;
using namespace Render;

namespace Game
{

//------------------------------------------------------------------------------
/**
*/
GolfGameApp::GolfGameApp()
{
    
}

//------------------------------------------------------------------------------
/**
*/
GolfGameApp::~GolfGameApp()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
bool
GolfGameApp::Open()
{
	App::Open();
	this->window = new Display::Window;
    this->window->SetSize(1280, 720);

    if (this->window->Open())
	{
		// set clear color to gray
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        RenderDevice::Init();

		// set ui rendering function
		this->window->SetUiRender([this]()
		{
			this->RenderUI();
		});
        this->window->SetNanoFunc([this](NVGcontext* vg)
        {
            this->RenderNanoVG(vg);
        });
        
        return true;
	}
	return false;
}

//------------------------------------------------------------------------------
/**
*/

void UpdateFreeFlyCamera(Camera* cam, float dt, const Input::Gamepad& pad,glm::vec3& position,float& yaw,float& pitch,glm::vec3& customUp)
{
  


    float moveSpeed = 5.0f;
    float turnSpeed = 150.5f;

    glm::vec2 look(pad.rightStick.x, pad.rightStick.y);
    yaw += look.x * turnSpeed * dt;
    pitch += look.y * turnSpeed * dt;

    // Clamp pitch
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    // Reconstruct front vector from yaw/pitch
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, customUp));
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    glm::vec2 stick(pad.leftStick.x, pad.leftStick.y);
    position += (front * stick.y + right * stick.x) * dt * moveSpeed;

    // Rebuild view matrix
    cam->view = glm::lookAt(position, position + front, up);

}
void
GolfGameApp::Run()
{
    int w;
    int h;
    this->window->GetSize(w, h);
    
    // camera thingy
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), float(w) / float(h), 0.01f, 1000.f);
    Camera* cam = CameraManager::GetCamera(CAMERA_MAIN);
    cam->projection = projection;
    
    Render::CameraManager::UpdateCamera(cam);

    std::vector<const char*> skybox
    {
        "assets/Minigolf/Models/Textures/variation-a.png",
        "assets/Minigolf/Models/Textures/variation-a.png",
        "assets/Minigolf/Models/Textures/variation-a.png",
        "assets/Minigolf/Models/Textures/variation-a.png",
        "assets/Minigolf/Models/Textures/variation-a.png",
        "assets/Minigolf/Models/Textures/variation-a.png"
    };
    TextureResourceId skyboxId = TextureResource::LoadCubemap("skybox", skybox, true);
    RenderDevice::SetSkybox(skyboxId);
   
    Map map1(5, 5);
    map1.readMapFromFile("assets/Minigolf/Maps/Map1.txt");
    map1.settingUpMap1();
    map1.highScorePathName = "assets/Minigolf/Highscore/HighscoreM1.txt";
    map1.highScoreManager.loadFromFile(map1.highScorePathName);


    Map map2(5, 5);
    map2.readMapFromFile("assets/Minigolf/Maps/Map2.txt");
    map2.settingUpMap2();
    map2.highScorePathName = "assets/Minigolf/Highscore/HighscoreM2.txt";
    map2.highScoreManager.loadFromFile(map2.highScorePathName);
    
    Map map3(5, 5);
    map3.readMapFromFile("assets/Minigolf/Maps/Map3.txt");
    map3.settingUpMap3();
    map3.highScorePathName = "assets/Minigolf/Highscore/HighscoreM2.txt";
    map3.highScoreManager.loadFromFile(map3.highScorePathName);
   

    maps.push_back(map1);
    maps.push_back(map2);
    maps.push_back(map3);

    
    
    bool selectedNewMap = false;
    
    Input::Keyboard* kbd = Input::GetDefaultKeyboard();
    
    Input::Gamepad pad;
    const int numLights = 10;
    Render::PointLightId lights[numLights];
    // Setup lights
    for (int i = 0; i < numLights; i++)
    {
        glm::vec3 translation = glm::vec3(
            Core::RandomFloatNTP() * 20.0f,
            Core::RandomFloatNTP() * 20.0f,
            Core::RandomFloatNTP() * 20.0f
        );
        glm::vec3 color = glm::vec3(
            Core::RandomFloat(),
            Core::RandomFloat(),
            Core::RandomFloat()
        );
        lights[i] = Render::LightServer::CreatePointLight(translation, color, Core::RandomFloat() * 4.0f, 1.0f + (15 + Core::RandomFloat() * 10.0f));
    }

    
    
    
    ball.model = LoadModel("assets/Minigolf/Models/GLB format/ball-blue.glb");
    ball.collider = Physics::CreateCollider(Physics::LoadColliderMesh("assets/Minigolf/Models/GLB format/ball-blue.glb"), ball.transform,1);

    
    std::clock_t c_start = std::clock();
    double dt = 0.01667f;
    glm::vec3 freeCamPos;
    glm::vec3 freeCamFront;
    glm::vec3 freeCamUp = glm::vec3(0,1,0);
    bool wasInFreeCam = false;
    float yaw;
    float pitch;

    //init first map spawnPos
    ball.position = maps[mapIndex].spawnPoint;
    ball.transform[3] = glm::vec4(ball.position, 1);
    for (auto& map : maps)
    {
        for (auto& tile : map.objects)
        {
            if (tile.objType != objectType::EMPTY)
            {
                Physics::SetActive(tile.colliderID, false);
            }
            if (tile.objType == objectType::FLAG)
            {
                Physics::SetActive(tile.colliderID, false);
            }
           
        }

    }

    for (auto& tile : maps[mapIndex].objects)
    {
        if (tile.objType != objectType::EMPTY)
        {
            Physics::SetActive(tile.colliderID, true);
        }
        if (tile.objType == objectType::FLAG)
        {
            Physics::SetActive(tile.colliderID, false);
        }
        
    }
    debugControlMode = false;
    bool selectHeldDown = false;
    HighScoreManager highScoreManager;
    highScoreManager.loadFromFile("assets/Minigolf/Highscore/HighscoreM1.txt");
   
    // game loop
    while (this->window->IsOpen())
	{

        
        pad.UpdateGamePad();
      

        auto timeStart = std::chrono::steady_clock::now();
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
        
        this->window->Update();

        if (kbd->pressed[Input::Key::Code::End])
        {
            ShaderResource::ReloadShaders();
        }
        if (pad.pressed[Input::Buttons::B])
        {
            ball.position = maps[mapIndex].spawnPoint;
            ball.transform[3] = glm::vec4(ball.position, 1);
            ball.shotTracker = 0;
            ball.linearVelocity = glm::vec3(0);
        }
        if (!debugControlMode)
        {
            //changing map buttons
            if (pad.pressed[Input::Buttons::Select]&&selectHeldDown==false)
            {

                for (auto& tile : maps[mapIndex].objects)
                {
                    if (tile.objType != objectType::EMPTY)
                    {
                        Physics::SetActive(tile.colliderID, false);
                    }

                }
                mapIndex++;
                if (mapIndex > 2)
                {
                    mapIndex = 0;
                }
                for (auto& tile : maps[mapIndex].objects)
                {
                    if (tile.objType != objectType::EMPTY)
                    {
                        Physics::SetActive(tile.colliderID, true);
                    }

                }
                selectHeldDown = true;
                selectedNewMap = true;
            }
            if (pad.released[Input::Buttons::Select])
            {
                selectHeldDown = false;
            }

            ball.Update(dt, pad);
            ballPos = ball.position;
            aimMode = ball.camLock;
            debugShotCounter = ball.shotTracker;
            bool enteringFreeCam = (!aimMode && ball.isStationary && !wasInFreeCam);
            wasInFreeCam = (!aimMode && ball.isStationary);
            float deltaHoledist;

            if (enteringFreeCam)
            {
                // Capture last known cam transform from camLock mode
                freeCamPos = ball.camPos;
                freeCamFront = glm::normalize(ball.position - freeCamPos); // looking at ball

                if (ball.fromLock)
                {
                    glm::vec3 right = glm::normalize(glm::cross(ball.aimDirection, freeCamFront));
                    yaw = glm::degrees(atan2(ball.aimDirection.z, ball.aimDirection.x));
                    pitch = -90.0f; // Or whatever slight downward tilt feels right
                }
                else
                {
                    yaw = glm::degrees(atan2(freeCamFront.z, freeCamFront.x));
                    pitch = glm::degrees(asin(freeCamFront.y));
                }

                freeCamUp = glm::vec3(0, 1, 0);
            }
            if (!aimMode && ball.isStationary) //free fly mode
            {
                UpdateFreeFlyCamera(cam, dt, pad, freeCamPos, yaw, pitch, freeCamUp);
            }


            // Physics::DebugDrawColliders();


            RenderDevice::Draw(ball.model, ball.transform);

            for (auto& tile : maps[mapIndex].objects)
            {

                RenderDevice::Draw(tile.model, tile.transform);
            }
            if (selectedNewMap)
            {
                ball.position = maps[mapIndex].spawnPoint;
                ball.transform[3] = glm::vec4(ball.position, 1);
                ball.shotTracker = 0;
                ball.linearVelocity = glm::vec3(0);
                selectedNewMap = false;
            }

            deltaHoledist = glm::length(ball.position - (maps[mapIndex].holePos));
            
            if (deltaHoledist < 0.3f)
            {
                HighScore highScore;
                highScore.score = debugShotCounter;
                maps[mapIndex].highScoreManager.addScore(highScore);
                maps[mapIndex].highScoreManager.saveToFile(maps[mapIndex].highScorePathName);
                maps[mapIndex].highScoreManager.loadFromFile(maps[mapIndex].highScorePathName);
                isDisplayWin = true;
                ball.position = maps[mapIndex].spawnPoint;
                ball.transform[3] = glm::vec4(ball.position, 1);
                ball.shotTracker = 0;
                ball.linearVelocity = glm::vec3(0);
             
            }
            if (winTimer <= 0.0f)
            {
                isDisplayWin = false;
                winTimer = 5.0f;
                for (auto& tile : maps[mapIndex].objects)
                {
                    if (tile.objType != objectType::EMPTY)
                    {
                        Physics::SetActive(tile.colliderID, false);
                    }

                }
                mapIndex++;
                if (mapIndex > 2)
                {
                    mapIndex = 0;
                }
                for (auto& tile : maps[mapIndex].objects)
                {
                    if (tile.objType != objectType::EMPTY)
                    {
                        Physics::SetActive(tile.colliderID, true);
                    }

                }
                ball.position = maps[mapIndex].spawnPoint;
                ball.transform[3] = glm::vec4(ball.position, 1);
                ball.shotTracker = 0;
                ball.linearVelocity = glm::vec3(0);
            }

            if (isDisplayWin)
            {
                winTimer -= dt;
            }
       

        }
        

       

       
        // Execute the entire rendering pipeline
        RenderDevice::Render(this->window, dt);

		// transfer new frame to window
		this->window->SwapBuffers();
    
        auto timeEnd = std::chrono::steady_clock::now();
        dt = std::min(0.04, std::chrono::duration<double>(timeEnd - timeStart).count());
     
        if (kbd->pressed[Input::Key::Code::Escape])
            this->Exit();
	}
}

//------------------------------------------------------------------------------
/**
*/
void
GolfGameApp::Exit()
{
    this->window->Close();
}

//------------------------------------------------------------------------------
/**
*/
void
GolfGameApp::RenderUI()
{
	if (this->window->IsOpen())
	{
        

        //Debug::DispatchDebugTextDrawing();
	}
}

//------------------------------------------------------------------------------
/**
*/
std::string FromUnsignedCharP(const unsigned char* c_str) {
    std::string str;
    int i{ 0 };
    while (c_str[i] != '\0') { str += c_str[i++]; }
    return str;
}
const char* ButtonToString(Input::Buttons::ButtonName btn)
{
    switch (btn)
    {
    case Input::Buttons::A: return "A";
    case Input::Buttons::B: return "B";
    case Input::Buttons::X: return "X";
    case Input::Buttons::Y: return "Y";
    case Input::Buttons::Select: return "Select";
    case Input::Buttons::Start: return "Start";
    case Input::Buttons::LB: return "Left Bumper";
    case Input::Buttons::RB: return "Right Bumper";
    case Input::Buttons::D_PADLeft: return "DPad Left";
    case Input::Buttons::D_PADRight: return "DPad Right";
    case Input::Buttons::D_PADUp: return "DPad Up";
    case Input::Buttons::D_PADDown: return "DPad Down";
    case Input::Buttons::LS: return "Left Stick ";
    case Input::Buttons::RS: return "Right Stick";
    case Input::Buttons::LT: return "Left Trigger";
    case Input::Buttons::RT: return "Right Trigger";
    default: return "Unknown";
    }
}
void DebugGamepadStatus(NVGcontext* vg, const Input::Gamepad& pad) {
    // Loop through buttons and display their state (pressed or not)
    for (int i = 0; i < Input::Buttons::NumButtons; ++i) {
        const char* buttonName = ButtonToString(static_cast<Input::Buttons::ButtonName>(i));

        if (pad.pressed[i]) {
            nvgText(vg, 0, 60 + (i * 30), (std::string(buttonName) + " pressed").c_str(), NULL);
        }
        else {
            nvgText(vg, 0, 60 + (i * 30), (std::string(buttonName) + " not pressed").c_str(), NULL);
        }
    }

    // Display trigger values
    std::string triggers = "Left Trigger: " + std::to_string(pad.leftTrigger) +
        " | Right Trigger: " + std::to_string(pad.rightTrigger);
    nvgText(vg, 0, 60 + (Input::Buttons::NumButtons * 30), triggers.c_str(), NULL);

    // Display joystick axis values
    std::string leftStick = "Left Stick: " + std::to_string(pad.leftStick.x) + ", " +
        std::to_string(pad.leftStick.y);
    std::string rightStick = "Right Stick: " + std::to_string(pad.rightStick.x) + ", " +
        std::to_string(pad.rightStick.y);
    nvgText(vg, 0, 90 + (Input::Buttons::NumButtons * 30), leftStick.c_str(), NULL);
    nvgText(vg, 0, 120 + (Input::Buttons::NumButtons * 30), rightStick.c_str(), NULL);
}
void
GolfGameApp::RenderNanoVG(NVGcontext* vg)
{
    nvgSave(vg);

    nvgBeginPath(vg);
   // nvgCircle(vg, 10, 10, 50);
    NVGpaint paint;
    paint = nvgLinearGradient(vg, 600, 100, 650, 150, nvgRGBA(255, 0, 0, 255), nvgRGBA(0, 255, 0, 255));
    nvgFillPaint(vg, paint);
    nvgFill(vg);
    Input::Gamepad pad;
   
    pad.UpdateGamePad();
    if (debugControlMode)
    {
        DebugGamepadStatus(vg, pad);
    }
    
   
    std::string s = "Shots " + std::to_string(debugShotCounter);
    nvgText(vg, 120, 120, s.c_str(), NULL);
    // Header
    nvgBeginPath(vg);
    
    std::string scoreText = "HighScores: ";
    std::string highScoresStr;
    nvgText(vg, 0, 120, scoreText.c_str(), NULL);
    for (int i = 0; i < maps[mapIndex].highScoreManager.scores.size(); i++)
    {
        highScoresStr = std::to_string(maps[mapIndex].highScoreManager.scores[i].score);
        nvgText(vg, 0, 150 + i * 20, highScoresStr.c_str(), NULL);
    }
   
    if (isDisplayWin && winTimer <= 5.0f)
    {
        
        std::string winStr = "YOU WIN";
        nvgText(vg, window->width * 0.5f, window->height * 0.5f, winStr.c_str(), NULL);

        std::string nextMap = "next map begins in:";
       

        std::ostringstream stream;
        stream << std::fixed << std::setprecision(1) << winTimer;
        std::string sec = stream.str();
        nvgText(vg, window->width * 0.5f, window->height * 0.5f + 20.0f, nextMap.c_str(), NULL);
        nvgText(vg, window->width * 0.5f, window->height * 0.5f + 50.0f, sec.c_str(), NULL);

        

    }

    nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 32));
    nvgStroke(vg);

    nvgFontSize(vg, 16.0f);
    nvgFontFace(vg, "sans");
    nvgFillColor(vg, nvgRGBA(200, 200, 255, 128));
    if (aimMode)
    {
        nvgText(vg, 20, 45, "Press Y button to change to free cam mode", NULL);

    }
    else
    {
        nvgText(vg, 20, 45, "Press Y button to change to aim mode", NULL);
    }
    nvgText(vg, 20, 90, "Press B to RESET", NULL);

   //nvgText(vg, 0, 30, "Testing, testing... Everything seems to be in order.", NULL);

    nvgRestore(vg);
}

} // namespace Game