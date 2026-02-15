#include "example_platformer.hpp"
#include "fumbo.hpp"

int main(int argc, char **argv) {
  // Initialize Engine
  Fumbo::Instance().Init(1280, 720, "Example Platformer", 0);

  // Start Game with Splash State
  Fumbo::Instance().Run(std::make_shared<PlatformerExample>());

  // Quit The Program
  Fumbo::Instance().Quit();
}
