#include "imagegen.h"
#include "game.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <string.h>
#include <unistd.h>


static void printOptions() {
    printf("Options:\n");
    printf("  check         - check for required files\n");;
    printf("  gen {L} {I}   - generate image I for the list L\n");
    printf("  run {L1} {L2} - run a game with the 2 specified lists\n");
}



static bool fileExists(const std::string file) {
  struct stat buffer;
  return (stat (file.c_str(), &buffer) == 0);
}



static void checkFiles() {
  std::vector<std::pair<std::string, std::string>> fontFiles = {
    { "Bank Gothic Medium BT.ttf",  "1d9f2941c1cb3bc8eaf9ade805f14421" },
    { "kimberley bl.ttf",           "220201383c9a6a82662450169bf0dd71" },
    { "xwing-miniatures-ships.ttf", "7f6e9e3eba8d769900ddb354d140cc0d" },
    { "xwing-miniatures.ttf",       "f58ed777affa1da16143a572880056fb" },
  };

  printf("Looking for required font files...\n");
  for(auto ff : fontFiles) {
    std::string filename = "./fonts/" + ff.first;
    printf("  %-40s - ", filename.c_str());
    if(fileExists(filename)) {
      printf("found\n");
      // maybe check md5's here as well...
    } else {
      printf("NOT FOUND!\n");
    }
  }
}



int main(int argc, char *argv[]) {

  if(argc == 1) {
    printOptions();
  }

  else if(strcmp(argv[1], "check") == 0) {
    checkFiles();
  }

  else if(strcmp(argv[1], "sanity") == 0) {
    Pilot::SanityCheck();
    printf("\n");
    Upgrade::SanityCheck();
    return 0;
  }

  else if((strcmp(argv[1], "dump") == 0) && (argc==3)) {
    Squad sq = Squad(argv[2]);
    sq.Dump();
  }

  else if((strcmp(argv[1], "gen") == 0) && (argc==4)) {
    Squad sq = Squad(argv[2]);
    GenerateImage(sq, argv[3]);
  }

  else if((strcmp(argv[1], "run") == 0) && (argc==4)) {
    std::string f1 = argv[2];
    std::string f2 = argv[3];
    if(!fileExists(f1)) {
      printf("ERROR: list not found - '%s'\n", f1.c_str());
      return 1;
    }
    if(!fileExists(f2)) {
      printf("ERROR: list not found - '%s'\n", f2.c_str());
      return 1;
    }
    try{
      std::array<Squad, 2> players = { { Squad(f1), Squad(f2) } };
      Game g = Game(players);
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
