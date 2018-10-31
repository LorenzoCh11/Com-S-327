#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

//added by LC
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/* Very slow seed: 686846853 */

#include "dungeon.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "io.h"

using namespace std;


const char *victory =
  "\n                                       o\n"
  "                                      $\"\"$o\n"
  "                                     $\"  $$\n"
  "                                      $$$$\n"
  "                                      o \"$o\n"
  "                                     o\"  \"$\n"
  "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
  "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
  "\"oo   o o o o\n"
  "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
  "   \"o$$\"    o$$\n"
  "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
  "     o\"\"\n"
  "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
  "   o\"\n"
  "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
  "\"$$$  $\n"
  "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
  "\"\"      \"\"\" \"\n"
  "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
  "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
  "\"$$$$\n"
  "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
  "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
  "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
  "              $\"                                                 \"$\n"
  "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
  "$\"$\"$\"$\"$\"$\"$\"$\n"
  "                                   You win!\n\n";

const char *tombstone =
  "\n\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
  "               /     /         \\             __\n"
  "              /     /           \\            ||\n"
  "             /____ /   Rest in   \\           ||\n"
  "            |     |    Pieces     |          ||\n"
  "            |     |               |          ||\n"
  "            |     |   A. Luser    |          ||\n"
  "            |     |               |          ||\n"
  "            |     |     * *   * * |         _||_\n"
  "            |     |     *\\/* *\\/* |        | TT |\n"
  "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
  "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
  "            |     |         \\/..\"\"\"\"\"...\"\"\""
  "\\ || /.\"\"\".......\"\"\"\"...\n"
  "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
  "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
  "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
  "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
  "            You're dead.  Better luck in the next life.\n\n\n";

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n"
          "          [-n|--nummon <count>]",
          name);

  exit(-1);
}


//A Dice class that outputs 3 different integers: base, number of dices, and number of sides
class Dice{
public:
 int base;
 int dices;
 int sides;

 int getVal(){
  int total = 0;
  int i;
  for(i=0;i<dices;i++){
   total += rand() % sides;
  }
  return total + base;
 }

};

//A mononster description class that keeps the information of each monster
class monsterDesc{
public:
    string name;
    string desc;
    string symbol;
    string color;
    int speed;
    string abil;
    int hp;
    int dam;
    int rrty;

};



int main(int argc, char *argv[])
{
  dungeon d;
  time_t seed;
  struct timeval tv;
  int32_t i;
  uint32_t do_load, do_save, do_seed, do_image, do_save_seed, do_save_image;
  uint32_t long_arg;
  char *save_file;
  char *load_file;
  char *pgm_file;

  /* Quiet a false positive from valgrind. */
  memset(&d, 0, sizeof (d));
  
  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = do_save_seed = do_save_image = 0;
  do_seed = 1;
  save_file = load_file = NULL;
  d.max_monsters = MAX_MONSTERS;

  /* The project spec requires '--load' and '--save'.  It's common  *
   * to have short and long forms of most switches (assuming you    *
   * don't run out of letters).  For now, we've got plenty.  Long   *
   * forms use whole words and take two dashes.  Short forms use an *
    * abbreviation after a single dash.  We'll add '--rand' (to     *
   * specify a random seed), which will take an argument of it's    *
   * own, and we'll add short forms for all three commands, '-l',   *
   * '-s', and '-r', respectively.  We're also going to allow an    *
   * optional argument to load to allow us to load non-default save *
   * files.  No means to save to non-default locations, however.    *
   * And the final switch, '--image', allows me to create a dungeon *
   * from a PGM image, so that I was able to create those more      *
   * interesting test dungeons for you.                             */
 
 if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 'n':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-nummon")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%hu", &d.max_monsters)) {
            usage(argv[0]);
          }
          break;
        case 'r':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-rand")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        case 'l':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-load"))) {
            usage(argv[0]);
          }
          do_load = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            load_file = argv[++i];
          }
          break;
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-save"))) {
            usage(argv[0]);
          }
          do_save = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll save to it.  If it is "seed", we'll save to    *
	     * <the current seed>.rlg327.  If it is "image", we'll  *
	     * save to <the current image>.rlg327.                  */
	    if (!strcmp(argv[++i], "seed")) {
	      do_save_seed = 1;
	      do_save_image = 0;
	    } else if (!strcmp(argv[i], "image")) {
	      do_save_image = 1;
	      do_save_seed = 0;
	    } else {
	      save_file = argv[i];
	    }
          }
          break;
        case 'i':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-image"))) {
            usage(argv[0]);
          }
          do_image = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            pgm_file = argv[++i];
          }
          break;
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }

  if (do_seed) {
    /* Allows me to generate more than one dungeon *
     * per second, as opposed to time().           */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  srand(seed);
  //edited by LC
  /*
  io_init_terminal();
  init_dungeon(&d);
  */
  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  /* Ignoring PC position in saved dungeons.  Not a bug. */
  //edited by LC
  // config_pc(&d);
   //   gen_monsters(&d);
   //editded by LC
   /*
  io_display(&d);
  if (!do_load && !do_image) {
    io_queue_message("Seed is %u.", seed);
  }
  while (pc_is_alive(&d) && dungeon_has_npcs(&d) && !d.quit) {
    do_moves(&d);
  }
  io_display(&d);

  io_reset_terminal();
   */
  if (do_save) {
    if (do_save_seed) {
       /* 10 bytes for number, please dot, extention and null terminator. */
      save_file = (char *) malloc(18);
      sprintf(save_file, "%ld.rlg327", seed);
    }
    if (do_save_image) {
      if (!pgm_file) {
	fprintf(stderr, "No image file was loaded.  Using default.\n");
	do_save_image = 0;
      } else {
	/* Extension of 3 characters longer than image extension + null. */
	save_file = (char *) malloc(strlen(pgm_file) + 4);
	strcpy(save_file, pgm_file);
	strcpy(strchr(save_file, '.') + 1, "rlg327");
      }
    }
    write_dungeon(&d, save_file);

    if (do_save_seed || do_save_image) {
      free(save_file);
    }
  }
  //edited by LC
  /*
  printf("%s", pc_is_alive(&d) ? victory : tombstone);
  printf("You defended your life in the face of %u deadly beasts.\n"
         "You avenged the cruel and untimely murders of %u "
         "peaceful dungeon residents.\n",
         d.PC->kills[kill_direct], d.PC->kills[kill_avenged]);
  */
  //edited by LC
  // if (pc_is_alive(&d)) {
    /* If the PC is dead, it's in the move heap and will get automatically *
     * deleted when the heap destructs.  In that case, we can't call       *
     * delete_pc(), because it will lead to a double delete.               */
  // character_delete(d.PC);
  // }

  // delete_dungeon(&d);



  /*--------------------------------Added by Lorenzo Chavarria ----------------------------------*/
  
  vector<string> lines; //is used to print out the information of the monsters
  vector<monsterDesc> mons; //Stores the information of each monster from the file
  string line;
  Dice dice;

  string home = getenv("HOME");
  string fpath = "/.rlg327/monster_desc.txt";
  string path = home + fpath;

  ifstream myfile (path.c_str());




  if (myfile.is_open()){
    //checks to make sure the first line is correct
    getline(myfile,line);
    //first line has to be "RLG327 MONSTER DESCRIPTION 1"
    if(line ==  "RLG327 MONSTER DESCRIPTION 1"){

    while(getline(myfile,line)){

       string name = "";
       string desc = "";
       string symbol = "";
       string color = "";
       string speed = "";
       string abil = "";
       string hp = "";
       string dam = "";
       string rrty = "";
       monsterDesc tmon;
       //makes sure that there is no duplicate of each field or any other error
       int duplicate = 0;
      if(line == "BEGIN MONSTER") {
	while(line != "END"){
	  getline(myfile,line);
	    if(line.find("NAME") == 0){
	      if(name != "")
	      {
	        duplicate = 1;
	      }

	      line.erase(0,5);
	      name = line;
	      tmon.name = name;
	    }
	    else if(line.find("DESC") == 0){
	      if(desc != "")
	      {
	        duplicate = 1;
	      }

	      while(line != "."){
		getline(myfile,line);
		if(line.length() >= 77)
		{
		  duplicate = 1;
		}
		//checks if it is the last line that is a period
		if(line != ".")
		{
		  desc += line;
		  desc += "\n";
		  tmon.desc += line;
		  tmon.desc += "\n";
		}
		else
		  {
		    desc.erase(desc.size()-1, desc.size());
		    tmon.desc.erase(desc.size()-1, desc.size());
		  }
	      }
	    }
	    else if(line.find("SYMB") == 0){
	      if(symbol != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,5);
	      symbol = line;
	      tmon.symbol = symbol;
	    }
	    else if(line.find("COLOR") == 0){
	      if(color != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,6);
	      color = line;
	      tmon.color = color;
	    }
	    else if(line.find("SPEED") == 0){
	      if(speed != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,6);
	      speed = line;
	      int base  = atoi(speed.substr(0, speed.find("+")).c_str());
	      int dices = atoi(speed.substr(speed.find("+") + 1, speed.find("d")).c_str());
	      int sides = atoi(speed.substr(speed.find("d") + 1, speed.length()).c_str());
	      dice.base = base;
	      dice.dices = dices;
	      dice.sides = sides;
	      tmon.speed  = dice.getVal();
	     
	    }
	    else if(line.find("ABIL") == 0){
	      if(abil != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,5);
	      abil = line;
	      tmon.abil = abil;
	    }
	    else if(line.find("HP") == 0){
	      if(hp != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,3);
	      hp = line;
	      int base  = atoi(hp.substr(0, hp.find("+")).c_str());
	      int dices = atoi(hp.substr(hp.find("+") + 1, hp.find("d")).c_str());
	      int sides = atoi(hp.substr(hp.find("d") + 1, hp.length()).c_str());
	      dice.base = base;
	      dice.dices = dices;
	      dice.sides = sides;
	      tmon.hp  = dice.getVal();
	    }
	    else if(line.find("DAM") == 0){
	      if(dam != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,4);
	      dam = line;
	      int base  = atoi(dam.substr(0, dam.find("+")).c_str());
	      int dices = atoi(dam.substr(dam.find("+") + 1, dam.find("d")).c_str());
	      int sides = atoi(dam.substr(dam.find("d") + 1, dam.length()).c_str());
	      dice.base = base;
	      dice.dices = dices;
	      dice.sides = sides;
	      tmon.dam  = dice.getVal();
	    }
	    else if(line.find("RRTY") == 0){
	      if(rrty != "")
	      {
	        duplicate = 1;
	      }
	      line.erase(0,5);
	      rrty = line;
	      tmon.rrty = atoi(rrty.c_str());
	    }
	    else {
		continue;
	    }
	}	
      }


      //Makes sure that there is no errors
      //This then stores all of the information in a vector named lines so that it can be printed
      //This also stores the information of each monster in a vector named mons
      if(name!="" && desc != "" && symbol != "" && color != "" && speed != "" && abil != "" && hp != ""
	 && dam != "" && rrty != "" && duplicate == 0) {
	lines.push_back(name);
	lines.push_back(desc);
	lines.push_back(symbol);
	lines.push_back(color);
	lines.push_back(speed);
	lines.push_back(abil);
	lines.push_back(hp);
	lines.push_back(dam);
	lines.push_back(rrty);
	lines.push_back("\n");
	mons.push_back(tmon);
      }

      }
    }
    else{
      lines.push_back("Incorrect file, does not have RLG327 MONSTER DESCRIPTION 1 on the first line.");
    }

    myfile.close();
  }   
 else {
    cout << "Unable to open\n";
  }

  std::vector<string>::iterator iter;

  for( iter = lines.begin(); iter != lines.end(); iter++)
  {
    cout<< *iter << endl;
    
  }



  return 0;
}
