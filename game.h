#pragma once
//#include "xwinglist.h"
#include "./libxwing/squad.h"
#include <array>



class Game {
 public:
  Game(std::array<Squad, 2>& p);
  void Run();

 private:
  std::array<Squad, 2>& players;
  bool isRunning;
  bool ParseCommand(std::string cmd);
};
