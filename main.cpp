#include "imagegen.h"
#include "game.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <map>
#include <string.h>
#include <unistd.h>

#define NORMAL "\x1B[0m"
#define GRAY   "\e[0;37m"
#define WHITE  "\e[1;37m"
#define BROWN  "\e[0;33m"
#define RED    "\e[1;31m"
#define GREEN  "\e[1;32m"
#define YELLOW "\e[1;33m"
#define BLUE   "\e[1;34m"
#define CYAN   "\e[1;36m"



static void printOptions() {
    printf("Options:\n");
    printf("  check             - check for required files\n");;
    printf("  ships             - prints all the ships\n");
    printf("  dump {L}          - dump the list to terminal\n");
    printf("  dump {P} {F} {S}  - dump the specified pilot/faction/ship (xws keys)\n");
    printf("  verify (L)        - verify the list (L)\n");
    printf("  gen {L} {I}       - generate image (I) for the list (L)\n");
    printf("  run {L1} {L2} {P} - run a game with the 2 specified lists, outputting images to the specified path\n");
}



static bool fileExists(const std::string file) {
  struct stat buffer;
  return (stat (file.c_str(), &buffer) == 0);
}



static std::vector<std::pair<std::string,bool>> CheckFonts() {
  std::vector<std::string> fontFiles = {
    { "Bank Gothic Medium BT.ttf"  },
    { "kimberley bl.ttf"           },
    { "xwing-miniatures-ships.ttf" },
    { "xwing-miniatures.ttf"       }
  };
  std::vector<std::pair<std::string,bool>> ret;
  for(auto ff : fontFiles) {
    std::string filename = "./fonts/" + ff;
    bool hasIt = false;
    if(fileExists(filename)) {
      hasIt = true;
    }
    ret.push_back({ff, hasIt});
  }
  return ret;
}



bool VerifyList(std::string listFile) {
  //printf("Verifying %-36s - ", listFile.c_str());
  fflush(stdout);
  try {    
    Squad sq = Squad(listFile);
    std::vector<std::string> issues = sq.Verify();
    if(issues.size() == 0) {
      printf("Ok\n");
    } else {
      printf("INVALID\n");
      for(std::string s : issues) {
	printf("  \e[1;31m%s\x1B[0m\n", s.c_str());
      }
    }
    if(issues.size() > 0) {
      return false;
    } else {
      return true;
    }
  }
  catch(std::invalid_argument e) {
    printf("EXCEPTION\n");
    printf("  \e[1;31m%s\x1B[0m\n", e.what());
    return false;
  }
}



static bool FindManeuver(Maneuvers maneuvers, uint8_t speed, Bearing bearing, Maneuver &maneuver) {
  for(auto m : maneuvers) {
    if((m.speed == speed) && (m.bearing == bearing)) {
      maneuver = m;
      return true;
    }
  }
  return false;
}

static std::string GetDifficultyColor(Difficulty d) {
  switch(d) {
  case Difficulty::Green: return GREEN;
  case Difficulty::White: return WHITE;
  case Difficulty::Red:   return RED;
  }
}

static std::string GetBearingSymbol(Bearing b) {
  switch(b) {
  case Bearing::LTurn:      return "↰";
  case Bearing::LBank:      return "↖";
  case Bearing::Straight:   return "↑";
  case Bearing::Stationary: return "■";
  case Bearing::RBank:      return "↗";
  case Bearing::RTurn:      return "↱";
  case Bearing::KTurn:      return "K";
  case Bearing::LSloop:     return "↖";
  case Bearing::RSloop:     return "↗";
  case Bearing::LTroll:     return "↰";
  case Bearing::RTroll:     return "↱";
  default:                  return "?";
  }
}

void PrintManeuverChart(Maneuvers maneuvers) {
  int8_t min = 10;
  int8_t max = -10;
  for(auto a : maneuvers) { if(a.speed > max) {max = a.speed;} if(a.speed < min) {min=a.speed;} }

  Maneuver m;
  for(int i=max; i>=min; i--) {
    printf(WHITE"%2d|", i);

    // standard maneuvers
    for(Bearing b : {Bearing::LTurn, Bearing::LBank, (i==0) ? Bearing::Stationary : Bearing::Straight, Bearing::RBank, Bearing::RTurn}) {
      if(FindManeuver(maneuvers, i, b, m)) {
        printf(" %s%s%s", GetDifficultyColor(m.difficulty).c_str(), GetBearingSymbol(m.bearing).c_str(), NORMAL);
      } else {
        printf("  ");
      }
    }

    // special maneuvers
    for(Bearing b : {Bearing::KTurn, Bearing::LSloop, Bearing::RSloop, Bearing::LTroll, Bearing::RTroll}) {
      if(FindManeuver(maneuvers, i, b, m)) {
        printf(" %s%s%s", GetDifficultyColor(m.difficulty).c_str(), GetBearingSymbol(m.bearing).c_str(), NORMAL);
      }
    }

    printf("\n");
  }
}

void PrintShip(std::string ship) {
  std::vector<Pilot> pilots;
  std::string name = "";
  Act         act  = Act::None;
  int nameLength = 0;
  Maneuvers maneuvers;
  for(Pilot p : Pilot::GetAllPilots()) {
    if(p.GetShipNameXws() == ship) {
      if(name=="") name = p.GetShipName();
      if(act==Act::None) act = p.GetNatActions();
      if(nameLength < p.GetPilotName().length()) nameLength = p.GetPilotName().length();
      if(maneuvers.size() == 0) maneuvers = p.GetManeuvers();
      pilots.push_back(p);
    }
  }

  std::sort(pilots.begin(), pilots.end(), [] (Pilot a, Pilot b) {
      
      return a.GetFaction() < b.GetFaction() || ((a.GetFaction() == b.GetFaction()) && (a.GetNatSkill() > b.GetNatSkill()));
    });

  printf(WHITE "%s\n" NORMAL, name.c_str());

  printf("\n");
  PrintManeuverChart(maneuvers);
  printf("\n");

  for(auto p : pilots) {
    printf(WHITE"%-6s" BROWN" %-2d" WHITE" %-*s" GRAY" [%-2d]" RED"  %-2d" GREEN" %-2d" YELLOW" %-2d" CYAN" %-2d" WHITE,
           (p.GetFaction() == Faction::Empire) ? "Empire" : (p.GetFaction() == Faction::Rebel) ? "Rebel" : "Scum",
           p.GetNatSkill(), nameLength, p.GetPilotName().c_str(), p.GetNatCost(),
           p.GetNatAttack(), p.GetNatAgility(), p.GetNatHull(), p.GetNatShield());

    printf(" - ");
    ForEachAction(p.GetNatActions(), [](Act a) {printf("[%s]", ActToString(a).c_str());});
    printf(" - ");
    for(Upg u : p.GetNatPossibleUpgrades()) {
      printf("[%s]", UpgToString(u).c_str());
    }
    printf(NORMAL"\n");

  }

}


int main(int argc, char *argv[]) {

  if(argc == 1) {
    printOptions();
  }

  else if(strcmp(argv[1], "check") == 0) {
    printf("Checking for required fonts...\n");
    std::vector<std::pair<std::string,bool>> cf = CheckFonts();
    int fontlen=0;
    for(auto f : cf) { if(f.first.length() > fontlen) fontlen = f.first.length(); };
    for(auto f : cf) { printf("  %-*s - %s\n", fontlen, f.first.c_str(), f.second ? "Ok" : "NOT FOUND"); }
  }

  else if(strcmp(argv[1], "sanity") == 0) {
    Pilot::SanityCheck();
    printf("\n");
    Upgrade::SanityCheck();
    return 0;
  }

  else if(strcmp(argv[1], "ships") == 0) {
    printf("%-30s %-30s\n", "Name", "xws key");
    std::vector<std::string> done;
    for(auto p : Pilot::GetAllPilots()) {
      if(std::find(done.begin(), done.end(), p.GetShipNameXws()) == done.end()) {
        done.push_back(p.GetShipNameXws());
        printf(" %-30s %-30s\n", p.GetShipName().c_str(), p.GetShipNameXws().c_str());
      }
    }
  }

  else if((strcmp(argv[1], "ship") == 0) && (argc == 3)) {
    PrintShip(argv[2]);
  }

  else if((strcmp(argv[1], "dump") == 0) && (argc==3)) {
    Squad sq = Squad(argv[2]);
    sq.Dump();
  }

  else if((strcmp(argv[1], "dump") == 0) && (argc==5)) {
    Pilot::GetPilot(argv[2], argv[3], argv[4]).Dump();
  }


  else if((strcmp(argv[1], "verify") == 0) && (argc==3)) {
    VerifyList(argv[2]);
  }

  else if((strcmp(argv[1], "gen") == 0) && (argc==4)) {
    Squad sq = Squad(argv[2]);
    GenerateImage(sq, argv[3]);
  }

  else if((strcmp(argv[1], "run") == 0) && (argc==5)) {
    bool cannotPlay = false;
    std::string f1 = argv[2];
    std::string f2 = argv[3];
    std::string outpath = argv[4];

    // verify we have the fonts
    printf("Checking fonts...\n");
    {
      std::vector<std::pair<std::string,bool>> cf = CheckFonts();
      int fontlen=0;
      for(auto f : cf) { if(f.first.length() > fontlen) fontlen = f.first.length(); };
      for(auto f : cf) {
        printf("  %-*s - ", fontlen, f.first.c_str());
        if(f.second) {
          printf("Ok\n");
        } else {
          printf("\e[1;31mNOT FOUND\x1B[0m\n");
          cannotPlay = true;
        }
      }
    }

    printf("\n");

    // verify the lists are present and valid
    printf("Checking lists...\n");
    {

      int listlen = f1.length();
      if(f2.length() > listlen) { listlen = f2.length(); }
      for(auto f : {f1, f2}) {
	printf("  %-*s - ", listlen, f.c_str());
	if(!fileExists(f)) {
	  printf("\e[1;31mERROR\n    File not found\x1B[0m\n");
	  cannotPlay = true;
	} else {
	  std::vector<std::string> issues = Squad(f).Verify();
	  if(issues.size() > 0) {
	    printf("\e[1;33mISSUES\x1B[0m\n");
	    for(std::string i : issues) {
	      printf("    \e[1;33m%s\x1B[0m\n", i.c_str());
	    }
	  } else {
	    printf("Ok\n");
	  }
	}
      }
      if(cannotPlay) return 0;
    }
    printf("\n");

    // run the game
    printf("Running game...\n");
    try{
      std::array<Squad, 2> players = { { Squad(f1), Squad(f2) } };
      Game g = Game(players, outpath);
      g.Run();
    }
    catch(std::invalid_argument ia) {
      printf("Error loading list '%s'\n", ia.what());
      return 1;
    }
  }

  else {
    printOptions();
  }

  return 0;
}
