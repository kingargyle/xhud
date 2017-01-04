#include "game.h"
#include "imagegen.h"
#include <stdio.h>
#include <cctype>
#include <iostream>

Game::Game(std::array<Squad, 2>& p)
  : players(p), isRunning(true) {
}

void Game::Run() {
  GenerateImage(this->players[0], "p1.png");
  GenerateImage(this->players[1], "p2.png");
  do {
    std::string line;
    printf("xhud> ");
    std::getline(std::cin, line);
    if(this->ParseCommand(line)) {
      GenerateImage(this->players[0], "p1.png");
      GenerateImage(this->players[1], "p2.png");
    }
  } while(this->isRunning);
}

enum class PState {
  GetPlayer,
  GetShip,
  GetShipCommand,
  GetUpgradeCommand
};

struct PTarget {
  uint8_t player;
  uint8_t ship;
  uint8_t upgrade;
};

// return is whether or not to redraw the images
bool Game::ParseCommand(std::string cmd) {

  if(cmd == "qqq") {
    this->isRunning = false;
    return false;
  }

  if(cmd == "?") {
    printf("Commands:\n");
    printf("  ?      - help\n");
    printf("  qqq    - quit\n");
    printf("  <PSC>  - modify ship stats\n");
    printf("  <PSUC> - modify upgrade status\n");
    printf("   P - player number (1 or 2)\n");
    printf("   S - ship number (1..n counting down)\n");
    printf("   U - upgrade number (1..n left to right, top to bottom)\n");
    printf("   C - command(s)\n");
    printf("     s - shield down\n");
    printf("     S - shield up\n");
    printf("     h - hull down\n");
    printf("     H - hull up\n");
    printf("     e - disable\n");
    printf("     E - enable\n");
    printf("  Examples:\n");
    printf("    13s     - player 1, ship 3 loses a shield\n");
    printf("    21ssh   - player 2, ship 1 loses 2 shield and a hull\n");
    printf("    11hhe   - player 1, ship 1 loses 2 hull and is disabled\n");
    printf("    231e    - player 2, ship 3, upgrade 1 is disabled\n");
    printf("    11h 12h - player 1, ships 1 and 2 each lose a hull\n");
    return false;
  }
  
  PState ps = PState::GetPlayer;
  PTarget pt;
  for(char c : cmd) {
    switch(ps) {
    case PState::GetPlayer:
      pt = {0,0,0};
      pt.player = c-48;
      if((pt.player < 1) || (pt.player > 2)) {
	printf("Invalid player\n");
	return false;
      }
      ps = PState::GetShip;
      break;

    case PState::GetShip:
      pt.ship = c-48;
      if((pt.ship < 1) || (pt.ship > this->players[pt.player-1].GetPilots().size())) {
	printf("Invalid ship\n");
	return false;
      }
      ps = PState::GetShipCommand;
      break;

    case PState::GetShipCommand:
      if(isdigit(c)) {
	pt.upgrade = c-48;
	if((pt.upgrade < 1) || (pt.upgrade > this->players[pt.player-1].GetPilots()[pt.ship-1].GetAppliedUpgrades().size()+1)) {
	  printf("Invalid upgrade\n");
	  return false;
	}
	ps = PState::GetUpgradeCommand;
      } else {
	std::string pn = this->players[pt.player-1].GetPilots()[pt.ship-1].GetPilotName();
	switch(c) {
	case 's': this->players[pt.player-1].GetPilots()[pt.ship-1].ShieldDn(); printf("  Player %d - Ship %d (%s) - Shield Down\n", pt.player, pt.ship, pn.c_str()); break;
	case 'S': this->players[pt.player-1].GetPilots()[pt.ship-1].ShieldUp(); printf("  Player %d - Ship %d (%s) - Shield Up\n",   pt.player, pt.ship, pn.c_str()); break;
	case 'h': this->players[pt.player-1].GetPilots()[pt.ship-1].HullDn();   printf("  Player %d - Ship %d (%s) - Hull Down\n",   pt.player, pt.ship, pn.c_str()); break;
	case 'H': this->players[pt.player-1].GetPilots()[pt.ship-1].HullUp();   printf("  Player %d - Ship %d (%s) - Hull Up\n",     pt.player, pt.ship, pn.c_str()); break;
	case 'e': this->players[pt.player-1].GetPilots()[pt.ship-1].Disable();  printf("  Player %d - Ship %d (%s) - Disabled\n",    pt.player, pt.ship, pn.c_str()); break;
	case 'E': this->players[pt.player-1].GetPilots()[pt.ship-1].Enable();   printf("  Player %d - Ship %d (%s) - Enabled\n",     pt.player, pt.ship, pn.c_str()); break;
	case ' ': ps = PState::GetPlayer;                                       break;
	  //case 'D': break;
	}
      }
      break;

    case PState::GetUpgradeCommand:
      std::string pn = this->players[pt.player-1].GetPilots()[pt.ship-1].GetPilotName();
      std::string un = this->players[pt.player-1].GetPilots()[pt.ship-1].GetAppliedUpgrades()[pt.upgrade-1].GetUpgradeName();
      switch(c) {
      case 'e':
	this->players[pt.player-1].GetPilots()[pt.ship-1].GetAppliedUpgrades()[pt.upgrade-1].Disable();
	printf("  Player %d - Ship %d (%s) - Upgrade %d (%s) - Disabled\n", pt.player, pt.ship, pn.c_str(), pt.upgrade, un.c_str());
	break;
      case 'E':
	this->players[pt.player-1].GetPilots()[pt.ship-1].GetAppliedUpgrades()[pt.upgrade-1].Enable();
	printf("  Player %d - Ship %d (%s) - Upgrade %d (%s) - Enabled\n",  pt.player, pt.ship, pn.c_str(), pt.upgrade, un.c_str());
	break;
      case ' ':
	ps = PState::GetPlayer;
	break;
      }
      break;
    }
  }

  return true;
}
