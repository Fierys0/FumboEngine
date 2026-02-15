#include "example_topdown.hpp"
#include "fumbo.hpp"

int main(int argc, char **argv) {
  // Initialize Engine
  Fumbo::Instance().Init(1280, 720, "Example TopDown", 0);

  // Start Game with Splash State
  Fumbo::Instance().Run(std::make_shared<TopDown>());

  // Quit The Program
  Fumbo::Instance().Quit();
}
