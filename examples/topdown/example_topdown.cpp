#include "example_topdown.hpp"
#include "raylib.h"
#include "raymath.h"

void TopDown::Init() {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  // Load background
  bgTex = Fumbo::Assets::LoadTexture("assets/background.png");

  // Setup camera
  camera.target = {640, 360};
  camera.offset = {640, 360};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  // Create boundary walls (room perimeter)
  walls.push_back(
      Fumbo::TopDown::CreateWall({640, 40}, {1280, 80}, DARKGRAY)); // Top
  walls.push_back(
      Fumbo::TopDown::CreateWall({640, 680}, {1280, 80}, DARKGRAY)); // Bottom
  walls.push_back(
      Fumbo::TopDown::CreateWall({40, 360}, {80, 720}, DARKGRAY)); // Left
  walls.push_back(
      Fumbo::TopDown::CreateWall({1240, 360}, {80, 720}, DARKGRAY)); // Right

  // Create some interior obstacles
  walls.push_back(Fumbo::TopDown::CreateWall({300, 200}, {100, 100}, GRAY));
  walls.push_back(Fumbo::TopDown::CreateWall({800, 400}, {150, 80}, GRAY));
  walls.push_back(Fumbo::TopDown::CreateWall({500, 500}, {120, 60}, GRAY));

  // Create player in center
  player = Fumbo::TopDown::CreateCharacter({640, 360}, {40, 40}, BLUE);

  // Create top-down controller
  controller = Fumbo::TopDown::CreateController(player, 125.0f);

  // Disable gravity globally for top-down
  physics.SetGravity({0, 0});
  physics.SetDebugDraw(showDebug);
}

void TopDown::Cleanup() {
  delete controller;

  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  for (auto &wall : walls) {
    physics.RemoveObject(wall);
    delete wall;
  }
  walls.clear();

  if (player) {
    physics.RemoveObject(player);
    delete player;
    player = nullptr;
  }

  // Re-enable gravity for other areas
  physics.SetGravity({0, 800.0f});

  UnloadTexture(bgTex);
}

void TopDown::Update() {
  float deltaTime = GetFrameTime();

  // Player input
  if (controller && player) {
    controller->Update(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT),
                       IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT),
                       IsKeyDown(KEY_W) || IsKeyDown(KEY_UP),
                       IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN));
  }

  // Toggle debug
  if (IsKeyPressed(KEY_F3)) {
    showDebug = !showDebug;
    Fumbo::Graphic2D::Physics::Instance().SetDebugDraw(showDebug);
  }

  // Update physics
  Fumbo::Graphic2D::Physics::Instance().Update(deltaTime);

  // Camera follows player
  if (player) {
    Vector2 playerPos = player->GetPosition();
    Rectangle playerRect = {playerPos.x - 20, playerPos.y - 20, 40, 40};
    Fumbo::Utils::Camera2DFollow(&camera, playerRect, 0, 0, 10.0f);
  }
}

void TopDown::DrawClean() { Fumbo::Graphic2D::DrawBackground(bgTex); }

void TopDown::DrawDirty() {
  BeginMode2D(camera);

  // Draw physics objects
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  for (const auto &obj : physics.GetObjects()) {
    obj->Render();
  }

  // Draw debug if enabled
  if (showDebug) {
    physics.DrawDebug();
  }

  EndMode2D();

  // UI
  DrawText("AREA 2 - TOP-DOWN VIEW", 10, 10, 30, WHITE);
  DrawText("WASD or Arrow Keys: Move", 10, 50, 20, LIGHTGRAY);
  DrawText("F3: Toggle debug view", 10, 75, 20, LIGHTGRAY);

  DrawText(TextFormat("FPS: %d", GetFPS()), 10, 110, 20, GREEN);

  if (player) {
    Vector2 vel = player->GetVelocity();
    Vector2 pos = player->GetPosition();
    DrawText(TextFormat("Player Velocity: (%.1f, %.1f)", vel.x, vel.y), 10, 135,
             20, RED);
    DrawText(TextFormat("Player Position: (%.1f, %.1f)", pos.x, pos.y), 10, 165,
             20, YELLOW);
  }
}
