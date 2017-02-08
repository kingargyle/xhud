# xhud
Overlay image generator for live streaming X-Wing Miniatures.

**Does:**
* Dumps squad info from an xws file.
* Generates squad image from xws file.
* Runs a game from 2 xws files and allows manipulation (add/remove shields/hull, enable/disable upgrades/ships).

**Does Not (yet):**
* Have a particularly user-friendly UI.

**Uses:**
* libxwing (my other project - https://github.com/sirjorj/libxwing.git)
* xwing-miniatures-font (https://github.com/geordanr/xwing-miniatures-font)

**Requires:**
* libgd (package manager for unix, homebrew for OS X, not yet sure for windows)
* Some additional fonts (see README in fonts directory)

***Getting Started (macOS-centric)***

Setup
  - dev environment
    - from terminal, run 'git' to trigger OS X to install the command line dev tools - XCode NOT required!
  - libgd
    - install homebrew from http://brew.sh (just copy the string on the front page and run it in terminal)
    - install libgd (the graphics drawing library) by running 'brew install gd' in the terminal

Get Code
  - make a directory for it called 'xhud'
  - 'git clone https://github.com/sirjorj/xhud.git xhud'
  - set up the libxwing submodule
    - browse into your xhud directory with 'cd xhud'
    - run 'git submodule init'
    - run 'git submodule update'

Build code
  - browse to libxwing submodule with 'cd libxwing'
  - build it with 'make'
  - browse back to xhud with 'cd ..'
  - build it with 'make'

Setup xhud
  - look at the README in xhud/fonts
  - follow the directions there to get the 4 required fonts and put them in xhud/fonts

Verify it works
  - cd to xhud and run './xhud check' to have it do an internal check

Use it
  - './xhud' to show all options
  - './xhud ships' to show all ships
  - './xbus ship tiefighter' to show all info on a TIE Fighter
  - './xhud dump list.xws' do have it dump the contents of 'list.xws' to the terminal
  - './xhud gen list.xws img.png' to have it create 'img.png' from 'list.xws'
  - './xhud run p1.xws p2.xws ./' to run a game with the 2 specified lists.
                                  this generates 'p1.png' and 'p2.png' in the same location as the program
                                  from the xhud> prompt, enter '?' for help on commands
  
