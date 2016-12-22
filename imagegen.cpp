#include "imagegen.h"
#include <gd.h>

//   fonts
// title: BankGothic Md BT
// text:  Eurostile Std Condensed
// stats: Kimberley Bold Regular
const std::string iconsFont = "./fonts/xwing-miniatures.ttf";
const std::string shipsFont = "./fonts/xwing-miniatures-ships.ttf";
const std::string titleFont = "./fonts/Bank Gothic Medium BT.ttf";
const std::string statsFont = "./fonts/kimberley bl.ttf";

static int WIDTH  =  381;
static int HEIGHT = 1080;



class Box {
public:
  static Box FromTLBR(int top, int left, int bottom, int right) {
    return Box(top, left, right-left+1, bottom-top+1);
  }
  static Box FromTLWH(int top, int left, int width, int height) {
    return Box(top, left, width, height);
  }
  int Top()    { return this->top; }
  int Left()   { return this->left; }
  int Width()  { return this->width; }
  int Height() { return this->height; }
  int Bottom() { return this->height + this->top - 1; }
  int Right()  { return this->width + this->left - 1; }
  
private:
  Box() { }
  Box(int t, int l, int w, int h) : top(t), left(l), width(w), height(h) { }
  int top, left, width, height;
};



struct ColorPalette {
  // background
  int bg;
  // basics
  int white;
  int black;
  // functions
  int skill;
  int attack;
  int agility;
  int hull;
  int shield;
  int hitHull;
  int hitShield;
  int upgrade;
  // disabled functions
  int skillD;
  int attackD;
  int agilityD;
  int hullD;
  int shieldD;
  int hitHullD;
  int hitShieldD;
  int upgradeD;
};

static Box GetTextSize(std::string text, std::string font, double size) {
  // [0,1] lower-left  X,Y
  // [2,3] lower-right X,Y
  // [4,5] upper-right X,Y
  // [6,7] upper-left  X,Y
  int brect[8];
  char *err = gdImageStringFT(0,         // img
			      &brect[0], // rect
			      0,         // color
			      (char*)font.c_str(), // font
			      size,      // size
			      0.0,       // angle
			      0,         // x
			      0,         // y
			      (char*)text.c_str()); // string
  if(err) { printf("%s\n", err); return Box::FromTLBR(0,0,0,0); }
  return Box::FromTLBR(brect[7], brect[6], brect[3], brect[2]);
}



static Box DrawText(std::string text, std::string font, double size, gdImagePtr img, int color, int x, int y) {
  int brect[8];
  char *err = gdImageStringFT(img,       // img
			      &brect[0], // rect
			      color,     // color
			      (char*)font.c_str(), // font
			      size,      // size
			      0.0,       // angle
			      x,         // x
			      y,         // y
			      (char*)text.c_str()); // string
  if(err) { printf("%s\n", err); return Box::FromTLBR(0,0,0,0); }
  return Box::FromTLBR(brect[7], brect[6], brect[3], brect[2]);
}

static std::string GetNatModString(uint8_t nat, uint8_t mod) {
  char s[8];
  if(nat == mod) {
    snprintf(s, sizeof(s), "%hhu", nat);
  } else {
    snprintf(s, sizeof(s), "%hhu(%hhu)", nat, mod);
  }
  return s;
}

static int DrawPilot(gdImagePtr img, Pilot& pilot, int yOffset, ColorPalette const &colors) {
  double skillFontSize = 20.0;
  double pilotFontSize = 20.0;
  double  costFontSize = 14.0;
  double statsFontSize = 20.0;
  double shipsFontSize = 26.0;
  std::string shipString    = pilot.GetShipGlyph();
  std::string pilotString   = pilot.GetPilotNameShort();
  std::string costString    = std::to_string(pilot.GetModCost());
  std::string skillString   = GetNatModString(pilot.GetNatSkill(), pilot.GetModSkill());
  std::string attackString  = GetNatModString(pilot.GetNatAttack(), pilot.GetModAttack());
  std::string agilityString = GetNatModString(pilot.GetNatAgility(), pilot.GetModAgility());
  std::string hullString    = GetNatModString(pilot.GetNatHull(), pilot.GetModHull());
  std::string shieldString  = GetNatModString(pilot.GetNatShield(), pilot.GetModShield());
  bool en = pilot.GetIsEnabled();

  // first we need to draw a box for this pilot.
  // height will be mostly constant but will vary a bit based on the number of upgrades.
  // we can still pre-calculate it all given that we have fixed sizes for everything...

  int upgHeight = ((pilot.GetAppliedUpgrades().size()+1)/2) * 21;
  int pilotHeight = 60;
  pilotHeight += upgHeight;
  pilotHeight += 20; // shield/hull
  pilotHeight += 5; // footer

  int yName = yOffset;
  int yStat = yOffset + 30;
  int yUpg  = yOffset + 60;
  int yHp   = yOffset + 60 + upgHeight + 5;

  // background transparent image to darken background
  Box bsPilot = Box::FromTLWH(yName, 0, 381, pilotHeight);
  gdImageFilledRectangle(img, bsPilot.Left(), bsPilot.Top(), bsPilot.Right(), bsPilot.Bottom(), colors.bg);

  // pilot
  Box bsShip = Box::FromTLWH(yName, 10, 30, 31);
  Box bsName = Box::FromTLWH(yName, 40, 310, 31); //GetTextSize(pilotString, titleFont, pilotFontSize);
  DrawText(shipString, shipsFont, shipsFontSize, img, colors.white, bsShip.Left(), bsShip.Top()+25);
  DrawText(pilotString, titleFont, pilotFontSize, img, colors.white, bsName.Left(), bsName.Top()+25);

  // cost
  Box bsCost = GetTextSize(costString, statsFont, costFontSize);
  DrawText(costString, statsFont, costFontSize, img, colors.skillD, 380-bsCost.Width(), yName+20);

  // skill
  Box bsSkill = Box::FromTLWH(yStat, 10, 60, 31);//GetTextSize(skillString, statsFont, skillFontSize);
  DrawText(skillString, statsFont, skillFontSize, img, en?colors.skill:colors.skillD, bsSkill.Left(), bsSkill.Top()+25);

  // attack
  Box bsAttack = Box::FromTLWH(yStat, 70, 50, 31); //GetTextSize(attackString, statsFont, statsFontSize);
  DrawText(attackString, statsFont, statsFontSize, img, en?colors.attack:colors.attackD, bsAttack.Left(), bsAttack.Top()+25);

  // agility
  Box bsAgility = Box::FromTLWH(yStat, 120, 50, 31); //GetTextSize(agilityString, statsFont, statsFontSize);
  DrawText(agilityString, statsFont, statsFontSize, img, en?colors.agility:colors.agilityD, bsAgility.Left(), bsAgility.Top()+25);

  // hull
  Box bsHull = Box::FromTLWH(yStat, 170, 50, 31); //GetTextSize(hullString, statsFont, statsFontSize);
  DrawText(hullString, statsFont, statsFontSize, img, en?colors.hull:colors.hullD, bsHull.Left(),  bsHull.Top()+25);

  // shield
  Box bsShield = Box::FromTLWH(yStat, 220, 50, 31); //GetTextSize(shieldString, statsFont, statsFontSize);
  DrawText(shieldString, statsFont, statsFontSize, img, en?colors.shield:colors.shieldD, bsShield.Left(), bsShield.Top()+25);

  // actions
  std::string actionString;
  ForEachAction(pilot.GetModActions(), [&actionString](Act a){
      actionString += GetActGlyph(a);
      actionString += " ";
    });
  actionString.pop_back();
  DrawText(actionString, iconsFont, 12, img, colors.white, 275, yStat+19);

  // upgrades
  int uCount = 0;
  for(auto u : pilot.GetAppliedUpgrades()) {
    int uRow = uCount / 2;
    Box bsIcon = Box::FromTLWH(yUpg+(uRow*20), !(uCount%2) ? 5 : 190, 22, 21);
    Box bsText = Box::FromTLWH(yUpg+(uRow*20), bsIcon.Right()+2, 160, 21);
    bool en = true; //u.GetIsEnabled();
    DrawText(GetUpgGlyph(u.GetType()).c_str(), iconsFont, 15, img, en ? colors.upgrade : colors.upgradeD, bsIcon.Left(), bsIcon.Top()+17);
    DrawText(u.GetUpgradeNameShort().c_str(), titleFont, 14, img, en ? colors.upgrade : colors.upgradeD, bsText.Left(), bsText.Top()+17);
    if(!u.GetIsEnabled()) {
      int y = ((bsIcon.Top() + bsIcon.Bottom()) / 2) + 3;
      gdImageLine(img, bsIcon.Left(), y, bsText.Right(), y, colors.white);
    }
    uCount++;
  }


  // current shield/hull
  int hpWidth = 360;
  int segments = pilot.GetModShield() + pilot.GetModHull();
  int segWidth =  hpWidth / segments;
  for(int i=0; i<pilot.GetModHull(); i++) {
    Box dummyBox = Box::FromTLWH(yHp, (segWidth*i)+10+2, segWidth-4, 10);
    gdImageFilledRectangle(img, dummyBox.Left(), dummyBox.Top(), dummyBox.Right(), dummyBox.Bottom(),
			   (i < (pilot.GetCurHull())) ? en?colors.hull:colors.hullD : en?colors.hitHull:colors.hitHullD);
  }
  for(int i=0; i<pilot.GetModShield(); i++) {
    Box dummyBox = Box::FromTLWH(yHp, (pilot.GetModHull()*segWidth)+(segWidth*i)+10+2, segWidth-4, 10);
    gdImageFilledRectangle(img, dummyBox.Left(), dummyBox.Top(), dummyBox.Right(), dummyBox.Bottom(),
			   (i < (pilot.GetCurShield())) ? en?colors.shield:colors.shieldD : en?colors.hitShield:colors.hitShieldD);
  }

  return pilotHeight;
}



void GenerateImage(Squad& squad, std::string name) {
  gdImagePtr img;
  FILE *out = fopen((name+".tmp").c_str(), "wb");
  if(out == 0) {
    printf("error opening file");
    return;
  }

  // create the image
  int yOffset = 0;
  img = gdImageCreateTrueColor(WIDTH, HEIGHT);

  // prep the color palette
  // http://www.had2know.com/technology/rgb-to-gray-scale-converter.html
  ColorPalette colors;
  colors.bg         = gdImageColorAllocateAlpha(img, 0, 0, 0, 32);
  colors.white      = gdImageColorAllocate(img, 255, 255, 255);
  colors.black      = gdImageColorAllocate(img,   0,   0,   0);
  colors.skill      = gdImageColorAllocate(img, 245, 127,  32);
  colors.skillD     = gdImageColorAllocate(img, 151, 151, 151);
  colors.attack     = gdImageColorAllocate(img, 235,  26,  65);
  colors.attackD    = gdImageColorAllocate(img,  93,  93,  93);
  colors.agility    = gdImageColorAllocate(img, 135, 209,  67);
  colors.agilityD   = gdImageColorAllocate(img, 171, 171, 171);
  colors.hull       = gdImageColorAllocate(img, 244, 239,  23);
  colors.hullD      = gdImageColorAllocate(img, 216, 216, 216);
  colors.shield     = gdImageColorAllocate(img,  99, 234, 246);
  colors.shieldD    = gdImageColorAllocate(img, 195, 195, 195);
  colors.hitHull    = gdImageColorAllocate(img,  61,  60,   6);
  colors.hitHullD   = gdImageColorAllocate(img,  54,  54,  54);
  colors.hitShield  = gdImageColorAllocate(img,  25,  59,  62);
  colors.hitShieldD = gdImageColorAllocate(img,  49,  49,  49);
  colors.upgrade    = gdImageColorAllocate(img, 255, 255, 255);
  colors.upgradeD   = gdImageColorAllocate(img,  96,  96,  96);

  // set transparent backgrounds
  gdImageSaveAlpha(img, 1);
  gdImageAlphaBlending(img, 0); // clear to enable transparent background
  for(int i=0; i<WIDTH; i++) {
    int color = gdImageColorAllocateAlpha(img, 0, 0, 0, 127);
    gdImageLine(img, i, 0, i, HEIGHT-1, color);
  }
  gdImageAlphaBlending(img, 1); // now that background is drawn, set this again to make fonts prettier

  // print title
  Box boxTitle = Box::FromTLWH(5, 5, 370, 30);
  gdImageFilledRectangle(img, boxTitle.Left(), boxTitle.Top(), boxTitle.Right(), boxTitle.Bottom(), colors.bg);
  std::string titleText = squad.GetName();
  double titleSize = 16.0;
  Box boxTitleText = GetTextSize(titleText, titleFont, titleSize);
  // do some checking here to make sure that boxTitleText fits withing boxTitle... eventually...
  Box boxTitleFinal = Box::FromTLWH(boxTitle.Top()+7, (boxTitle.Width()-boxTitleText.Width())/2, boxTitleText.Width(), boxTitleText.Height());
  DrawText(titleText, titleFont, titleSize, img, colors.white, boxTitleFinal.Left(), boxTitleFinal.Top() + boxTitleFinal.Height());
  yOffset = 50;

  // draw the pilots
  for(auto& pilot : squad.GetPilots()) {
    yOffset += DrawPilot(img, pilot, yOffset, colors);
    yOffset += 10;  // some space between pilots
  }

  // save the image
  gdImagePng(img, out);
  fclose(out);
  rename((name+".tmp").c_str(), name.c_str());

  // clean up

  gdImageDestroy(img);  
}
