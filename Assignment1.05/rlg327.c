#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>
#include <curses.h>

/* Very slow seed: 686846853 */

#include "dungeon.h"

#include "pc.h"
#include "npc.h"
#include "move.h"


/* Added by LC */
void makeStairs(dungeon_t *d){
  uint8_t x, y;
  uint8_t x2,y2;
    x = rand() % 80;
    y = rand() % 21;
    while (d->map[y][x] != ter_floor_room){
     x = rand() % 80;
     y = rand() % 21;
    }
    d->map[y][x] = ter_stair_up;

    x2 = rand() % 80;
    y2 = rand() % 21;
    while (d->map[y2][x2] != ter_floor_room){
     x2 = rand() % 80;
     y2 = rand() % 21;
    }

    d->map[y2][x2] = ter_stair_down;

}


int isValidKey(int key){
  char validChar[] = {'y','7','k','8','u','9','l','6','n','3','j','2','b','1','h','4',32,'5'};
  int i;

  for (i = 0; i < 18; i++){
    if (validChar[i] == key){
      return 1;
    }
  }
   
    return 0;
  
}

/* Added by LC */


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

int main(int argc, char *argv[])
{
  dungeon_t d;
  time_t seed;
  struct timeval tv;
  uint32_t i;
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

  if (!do_load && !do_image) {
    printf("Seed is %ld.\n", seed);
  }
  srand(seed);

  init_dungeon(&d);

  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  /* Ignoring PC position in saved dungeons.  Not a bug. */
  config_pc(&d);
  gen_monsters(&d);

  
  /*
  //makes array of the monsters
    int row, col;
    int monholder = 0;
    character_t *mon[d.max_monsters];
    for(row = 0; row < DUNGEON_Y; row++){
	for(col = 0; col < DUNGEON_X; col++){
	  if(d.character[row][col] != NULL && d.character[row][col] != &d.pc)
	  {
	    mon[monholder] = d.character[row][col];
	    monholder++;
	  }
	     
	}
    }
 
  */

  /* This was added by Lorenzo Chavarria */



  makeStairs(&d);


  initscr();
  noecho();
  refresh();



  //keeps track of which monsters to print
  int scroll = 0;





  char key = 'a';

  while (pc_is_alive(&d) && dungeon_has_npcs(&d) && key != 'q') {

   
    render_dungeon(&d);
    key = getch();

    if(d.map[d.pc.position[dim_y]][d.pc.position[dim_x]] == ter_stair_up && key == ','){ //without '<' being shifted
      pc_delete(d.pc.pc);
      delete_dungeon(&d);
      init_dungeon(&d);
      gen_dungeon(&d);
      config_pc(&d);
      gen_monsters(&d);
      makeStairs(&d);
    }
    else if(d.map[d.pc.position[dim_y]][d.pc.position[dim_x]] == ter_stair_down && key == '.'){//without '>'  being shifted
      pc_delete(d.pc.pc);
      delete_dungeon(&d);
      init_dungeon(&d);
      gen_dungeon(&d);
      config_pc(&d);
      gen_monsters(&d);
      makeStairs(&d);
    }
    else if(key == 'm'){

      while( key != 27 && key != 'q'){

	WINDOW * win = newwin(21, 80, 0, 0);
	box(win, 0, 0);
	mvwprintw(win, 1, 1, "You Know of these monsters:");

	//Enables arrows
	keypad(win, TRUE);


	//Creates a temporary array that holds each monster
	int row, col;
	int monholder = 0;
	character_t *mon[d.num_monsters];
	for(row = 0; row < DUNGEON_Y; row++){
	  for(col = 0; col < DUNGEON_X; col++){
	    if(d.character[row][col] != NULL && d.character[row][col] != &d.pc)
	      {
		mon[monholder] = d.character[row][col];
		monholder++;
	      }
	     
	  }
	}





	int mnum;
	int pholder = 2; 
	int distancey = 0;
	int distancex = 0;
	char dirns = NULL;
	char dirwe = NULL;

	if(d.max_monsters > 19){

	  if(key == 3 && scroll + 19 < d.num_monsters){
	    scroll++;
	  }

	  if(key == 2 && scroll > 0){
	    scroll--;
	  }


	}


	for(mnum = scroll; mnum < d.num_monsters; mnum++){
	 
	         mvwprintw( win, pholder, 1,"%c", mon[mnum]->symbol);
	         distancey = d.pc.position[dim_y] - mon[mnum]->position[dim_y];
	         distancex = d.pc.position[dim_x] - mon[mnum]->position[dim_x];

		 //checks for direction: north or south
		 if(distancey < 0){
		   distancey = abs(distancey);
		   dirns = 'S';
		 }
		 else
		 {
		   distancey = distancey;
		   dirns = 'N';
		 }
		 //checks for direction; west or east
		 if(distancex < 0){
		   distancex = abs(distancex);
		   dirwe = 'E';
		 }
		 else
		 {
		   distancex = distancex;
		   dirwe = 'W';
		 }

		 mvwprintw(win, pholder, 3, "%c", dirns);
		 mvwprintw(win, pholder, 6,"%d", distancey);

		 mvwprintw(win, pholder, 9, "%c", dirwe);
		 mvwprintw(win, pholder, 12,"%d", distancex);

		 pholder++;  
	        
	}

	wrefresh(win); //check placement of this code
	key = wgetch(win);
  
      }
     
      endwin();

    }
    else{
      if( isValidKey(key) == 1){		  
	do_moves(&d, key);
      }
    }

  }

  endwin();




  render_dungeon(&d);

  if (do_save) {
    if (do_save_seed) {
       /* 10 bytes for number, please dot, extention and null terminator. */
      save_file = malloc(18);
      sprintf(save_file, "%ld.rlg327", seed);
    }
    if (do_save_image) {
      if (!pgm_file) {
	fprintf(stderr, "No image file was loaded.  Using default.\n");
	do_save_image = 0;
      } else {
	/* Extension of 3 characters longer than image extension + null. */
	save_file = malloc(strlen(pgm_file) + 4);
	strcpy(save_file, pgm_file);
	strcpy(strchr(save_file, '.') + 1, "rlg327");
      }
    }
    write_dungeon(&d, save_file);

    if (do_save_seed || do_save_image) {
      free(save_file);
    }
  }
  if( key != 'q'){
    printf("%s", pc_is_alive(&d) ? victory : tombstone);
    printf("You defended your life in the face of %u deadly beasts.\n"
	   "You avenged the cruel and untimely murders of %u "
	   "peaceful dungeon residents.\n",
	   d.pc.kills[kill_direct], d.pc.kills[kill_avenged]);
  }
  else{
    printf("You quit. Game Over. \n");
  }

  pc_delete(d.pc.pc);

  delete_dungeon(&d);

  return 0;
}
