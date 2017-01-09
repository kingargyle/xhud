#include "imagegen.h"
#include "game.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <string.h>
#include <unistd.h>


static void printOptions() {
    printf("Options:\n");
    printf("  check             - check for required files\n");;
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

  else if((strcmp(argv[1], "dump") == 0) && (argc==3)) {
    Squad sq = Squad(argv[2]);
    sq.Dump();
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
