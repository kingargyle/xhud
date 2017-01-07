#pragma once
//#include "xwinglist.h"
#include "./libxwing/squad.h"
#include <array>



class Game {
 public:
  Game(std::array<Squad, 2>& p, std::string op);
  void Run();

 private:
  std::array<Squad, 2>& players;
  std::string outPath;
  bool isRunning;
  bool ParseCommand(std::string cmd);
};
