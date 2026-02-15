#include "example_platformer.hpp"

void PlatformerExample::Init() {

  // Setup camera
  camera = {0};
  camera.zoom = 1.0f;
  camera.offset = {0, 0};
  camera.target = {0, 0};

  // Initialize render texture for scene
  currentScreen = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

  // Track initial screen size
  lastScreenWidth = GetScreenWidth();
  lastScreenHeight = GetScreenHeight();

  // Initialize enemy tracking variables
  oldEnemyPos = {0, 0};
  timerDelta = 0.0f;

  // Load background
  bgTex = Fumbo::Assets::LoadTexture("");

  // Setup physics
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  physics.Clear();
  physics.SetGravity({0, 980.0f});
  physics.SetDebugDraw(true);
  physics.SetFixedTimeStep(60.0f);

  // Create platforms using platformer extension
  ground = Fumbo::Platformer::CreatePlatform({0, 680}, {9000, 80}, DARKGRAY);
  platform1 = Fumbo::Platformer::CreatePlatform({1000, 500}, {200, 20}, GREEN);
  platform2 = Fumbo::Platformer::CreatePlatform({640, 400}, {200, 20}, GREEN);

  // Create player
  player = Fumbo::Platformer::CreateCharacter({100, 600}, {40, 40}, RED);

  // Create controller using platformer extension
  controller = Fumbo::Platformer::CreateController(player, 350.0f, 600.0f);

  // Create some pushable crates
  for (int i = 0; i < 3; i++) {
    auto crate = new Fumbo::Graphic2D::Object();
    crate->SetPosition({300.0f + i * 200.0f, 420.0f - i * 100.0f});
    crate->SetRectangle(40 + i * 10, 40 + i * 10);
    crate->SetMass(5.0f + i * 2.0f); // Heavier crates: 5, 7, 9 kg
    crate->SetRestitution(0.0f);
    crate->SetFriction(0.5f); // High friction for realistic box behavior

    Color colors[] = {BROWN, ORANGE, YELLOW};
    crate->SetColor(colors[i]);

    physics.AddObject(crate);
    crates.push_back(crate);
  }

  // Create collectible coins (non-collidable, triggers only)
  float coinPositions[][2] = {{300, 400}, {500, 300}, {700, 250},
                              {900, 350}, {400, 200}, {600, 450}};

  for (auto &pos : coinPositions) {
    auto coin = new Fumbo::Graphic2D::Object();
    coin->SetPosition({pos[0], pos[1]});
    coin->SetCircle(8); // Smaller coins
    coin->SetColor(YELLOW);
    coin->SetGravityScale(0.0f); // Floating coins
    coin->SetCollidable(false);  // No physics, just trigger
    physics.AddObject(coin);
    coins.push_back(coin);
  }

  // Create enemy (moving horizontally)
  enemy = new Fumbo::Graphic2D::Object();
  enemy->SetPosition({800, 620});
  enemy->SetRectangle(40, 40);
  enemy->SetColor(RED);
  enemy->SetMass(1.0f);
  enemy->SetGravityScale(0.0f); // Floats in air
  enemy->SetFriction(0.0f);
  physics.AddObject(enemy);

  // Give enemy initial velocity
  enemy->SetVelocity({50, 0}); // Moving right
}

void PlatformerExample::Cleanup() {
  delete controller;
  controller = nullptr; // Ensure controller is nullified after deletion

  auto &physics = Fumbo::Graphic2D::Physics::Instance();

  // Clean up crates
  for (auto &crate : crates) {
    physics.RemoveObject(crate);
    delete crate;
  }
  crates.clear();

  // Clean up coins
  for (auto &coin : coins) {
    physics.RemoveObject(coin);
    delete coin;
  }
  coins.clear();

  // Clean up enemy
  if (enemy) {
    physics.RemoveObject(enemy);
    delete enemy;
    enemy = nullptr;
  }

  // Clean up platforms and player
  if (ground) {
    physics.RemoveObject(ground);
    delete ground;
    ground = nullptr;
  }
  if (platform1) {
    physics.RemoveObject(platform1);
    delete platform1;
    platform1 = nullptr;
  }
  if (platform2) {
    physics.RemoveObject(platform2);
    delete platform2;
    platform2 = nullptr;
  }
  if (player) {
    physics.RemoveObject(player);
    delete player;
    player = nullptr;
  }

  // Clean up render texture
  if (currentScreen.id != 0) {
    UnloadRenderTexture(currentScreen);
    currentScreen = {0};
  }

  UnloadTexture(bgTex);

  // Clean up cached god rays resources
  if (maskRenderTarget.id != 0) {
    UnloadRenderTexture(maskRenderTarget);
    maskRenderTarget = {0};
  }
  if (maskShader.id != 0) {
    UnloadShader(maskShader);
    maskShader = {0};
  }
  if (blurRenderTarget.id != 0) {
    UnloadRenderTexture(blurRenderTarget);
    blurRenderTarget = {0};
  }
  if (blurShader.id != 0) {
    UnloadShader(blurShader);
    blurShader = {0};
  }

  // Reset game state variables
  gameOver = false;
  score = 0;
  coinCounter = 0;
}

void PlatformerExample::Update() {
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  float deltaTime = GetFrameTime();

  // Check for window resize (e.g., fullscreen toggle)
  int currentWidth = GetScreenWidth();
  int currentHeight = GetScreenHeight();
  if (currentWidth != lastScreenWidth || currentHeight != lastScreenHeight) {
    // Recreate render textures with new size
    if (currentScreen.id != 0) {
      UnloadRenderTexture(currentScreen);
    }
    currentScreen = LoadRenderTexture(currentWidth, currentHeight);

    // Force recreation of god rays resources with new size
    if (maskRenderTarget.id != 0) {
      UnloadRenderTexture(maskRenderTarget);
      maskRenderTarget = {0};
    }
    if (blurRenderTarget.id != 0) {
      UnloadRenderTexture(blurRenderTarget);
      blurRenderTarget = {0};
    }

    // Update tracked size
    lastScreenWidth = currentWidth;
    lastScreenHeight = currentHeight;
  }

  // Handle game over state
  if (gameOver) {
    if (IsKeyPressed(KEY_R)) {
      // Cleanup();
      gameOver = false;
      score = 0;
      coinCounter = 0;
      Init();
    }
    return; // Don't process input or physics
  }

  if (IsKeyPressed(KEY_R)) {
    // Cleanup();
    Init();
  }

  // Player input
  if (controller && player) {
    controller->Update(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT),
                       IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT),
                       IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W));
  }

  // Toggle debug
  if (IsKeyPressed(KEY_F3)) {
    showDebug = !showDebug;
    Fumbo::Graphic2D::Physics::Instance().SetDebugDraw(showDebug);
  }

  // Toggle render mode
  if (IsKeyPressed(KEY_F1)) {
    renderMode =
        static_cast<RenderMode>((static_cast<int>(renderMode) + 1) % 3);
  }

  // Update physics simulation
  Fumbo::Graphic2D::Physics::Instance().Update(deltaTime);

  // Check coin collection
  for (auto it = coins.begin(); it != coins.end();) {
    if (player && player->IsCollidingWith(*it)) {
      score += 10;
      coinCounter++;
      auto &physics = Fumbo::Graphic2D::Physics::Instance();
      physics.RemoveObject(*it);
      delete *it;
      it = coins.erase(it);
    } else {
      ++it;
    }
  }

  // Check enemy collision
  if (player && enemy && player->IsCollidingWith(enemy)) {
    Vector2 playerPos = player->GetPosition();
    Vector2 enemyPos = enemy->GetPosition();

    // If player is above enemy (jumping on head), kill enemy
    if (playerPos.y < enemyPos.y - 10) {
      score += 50; // Bonus for killing enemy
      auto &physics = Fumbo::Graphic2D::Physics::Instance();
      physics.RemoveObject(enemy);
      delete enemy;
      enemy = nullptr;

      // Bounce player up a bit
      player->SetVelocity({player->GetVelocity().x, -300});
    } else {
      // Hit from side/below = game over
      gameOver = true;
    }
  }

  // Enemy AI
  if (enemy) {
    Vector2 enemyPos = enemy->GetPosition();
    Vector2 enemyVel = enemy->GetVelocity();

    timerDelta += deltaTime;

    bool hitBoundary = (enemyPos.x > 1100 || enemyPos.x < 0);

    float distanceMoved = fabsf(enemyPos.x - oldEnemyPos.x);
    bool isStuck = (distanceMoved < 0.1f && timerDelta >= 2.0f);

    if (hitBoundary || isStuck) {
      float newDir = (enemyVel.x > 0) ? -1.0f : 1.0f;
      enemy->SetVelocity({newDir * 50, enemyVel.y});

      timerDelta = 0.0f;
    }

    if (timerDelta >= 1.0f) {
      oldEnemyPos = enemyPos;
    }
  }

  // Camera follows player
  if (player) {
    Vector2 playerPos = player->GetPosition();
    Rectangle playerRect = {playerPos.x - 20, playerPos.y - 20, 40, 40};
    Fumbo::Utils::Camera2DFollow(&camera, playerRect, 200, 0, 25.0f);
  }

  if (player) {
    Vector2 playerPos = player->GetPosition();
    if (playerPos.y >= 3000) {
      gameOver = true;
    }
  }
}

void PlatformerExample::DrawClean() { Fumbo::Graphic2D::DrawBackground(bgTex); }

void PlatformerExample::DrawScene() {
  BeginTextureMode(currentScreen);
  ClearBackground(BLANK);
  BeginMode2D(camera);
  auto &physics = Fumbo::Graphic2D::Physics::Instance();
  for (const auto &obj : physics.GetObjects()) {
    obj->Render(); // Use object's render method (handles circles, rectangles,
                   // etc.)
  }

  // Draw debug info if enabled
  if (showDebug) {
    physics.DrawDebug();
  }
  EndMode2D();
  EndTextureMode();
}

void PlatformerExample::DrawDirty() {

  // Render the scene to texture
  DrawScene();

  // Render based on current mode
  if (renderMode == RenderMode::NORMAL) {
    // Normal rendering - just draw the scene
    DrawTexturePro(
        currentScreen.texture,
        (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
        (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
        (Vector2){0, 0}, 0.0f, WHITE);
  } else if (renderMode == RenderMode::MASK) {
    // Mask rendering - white background with black objects
    Texture2D sceneMask = Fumbo::Utils::DrawMask(
        currentScreen, maskRenderTarget, maskShader, WHITE, BLACK);
    DrawTexturePro(
        sceneMask,
        (Rectangle){0, 0, (float)sceneMask.width, -(float)sceneMask.height},
        (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
        (Vector2){0, 0}, 0.0f, WHITE);
  } else if (renderMode == RenderMode::GODRAYS) {
    // God rays rendering - apply radial blur to mask
    Texture2D sceneMask =
        Fumbo::Utils::DrawMask(currentScreen, maskRenderTarget, maskShader,
                               {255, 255, 255, 120}, BLACK);

    Vector2 blurCenter = {(float)GetScreenWidth() / 2.0f,
                          (float)GetScreenHeight() / 2.0f};
    Texture2D godRays = Fumbo::Utils::ApplyRadialBlur(
        sceneMask, blurRenderTarget, blurShader, 0.5f, {100, 600});

    DrawTexturePro(
        currentScreen.texture,
        (Rectangle){0, 0, (float)GetScreenWidth(), -(float)GetScreenHeight()},
        (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
        (Vector2){0, 0}, 0.0f, WHITE);

    BeginBlendMode(BLEND_ADDITIVE);
    DrawTexturePro(
        godRays, (Rectangle){0, 0, (float)godRays.width, (float)godRays.height},
        (Rectangle){0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()},
        (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
  }
  // UI
  Fumbo::Graphic2D::DrawText("AREA 1 - 2D PLATFORMER", {10, 10}, {}, 30, WHITE);
  Fumbo::Graphic2D::DrawText(
      "A/D: Move | SPACE: Jump | F1: Render Mode | F3: Debug", {10, 50}, {}, 20,
      LIGHTGRAY);

  // Show current render mode
  const char *modeText = "NORMAL";
  if (renderMode == RenderMode::MASK)
    modeText = "MASK";
  else if (renderMode == RenderMode::GODRAYS)
    modeText = "GODRAYS";
  Fumbo::Graphic2D::DrawText(TextFormat("Render Mode: %s", modeText), {10, 205},
                             {}, 20, ORANGE);

  // Score and stats
  Fumbo::Graphic2D::DrawText(TextFormat("Score: %d", score), {10, 85}, {}, 25,
                             YELLOW);
  Fumbo::Graphic2D::DrawText(TextFormat("Coins: %d", coinCounter), {10, 115},
                             {}, 20, GOLD);

  if (player) {
    Vector2 vel = player->GetVelocity();
    Vector2 pos = player->GetPosition();
    Fumbo::Graphic2D::DrawText(
        TextFormat("Velocity: (%.0f, %.0f)", vel.x, vel.y), {10, 145}, {}, 20,
        SKYBLUE);
    Fumbo::Graphic2D::DrawText(
        TextFormat("Position: (%.0f, %.0f)", pos.x, pos.y), {10, 175}, {}, 20,
        SKYBLUE);
  }

  // Game over message
  if (gameOver) {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, 0.7f));
    Fumbo::Graphic2D::DrawText("GAME OVER!", {440, 300}, {}, 60, RED);
    Fumbo::Graphic2D::DrawText(TextFormat("Final Score: %d", score), {480, 380},
                               {}, 30, YELLOW);
    Fumbo::Graphic2D::DrawText("Press R to Retry", {500, 430}, {}, 25, WHITE);
    Fumbo::Graphic2D::DrawText("Press ESC to exit", {500, 465}, {}, 20,
                               LIGHTGRAY);
  }
}
