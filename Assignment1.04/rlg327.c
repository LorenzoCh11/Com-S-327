#include <stdio.h>
#include <string.h>
#include <sys/time.h>
//Added by LC
#include <math.h>

/* Very slow seed: 686846853 */

#include "dungeon.h"
#include "path.h"


 /* Variables added by Lorenzo Chavarria*/
  uint32_t  numMonster = 10;
 



void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n",
          name);

  exit(-1);
}


/* This is a comparator made for the heap.
   added by LC */
static int32_t monster_cmp(const void *key, const void *with) {
  return ((player_t *) key)->turn - ((player_t *) with)->turn;
}


/* This changes the coordinates for a monster, allowing it to move.
   Added by LC */
void moveMonster(dungeon_t *d, uint8_t idnum){
  uint8_t x2;
  uint8_t y2;
  uint8_t iny;
  uint8_t inx;
  int moved = 0;
  int lowestplace = INT8_MAX;


  //Find the monster that is going to move in the dungeon
  for(y2 = 0; y2 < DUNGEON_Y; y2++){
      for(x2 = 0; x2 < DUNGEON_X; x2++){
	

	if(d->monster[y2][x2].id == idnum && moved == 0){
        
	  //checks left side
	  if (d->pc_tunnel[y2][x2 - 1] < lowestplace){
	    lowestplace = d->pc_tunnel[y2][x2 - 1];
	    iny = 0;
	    inx = -1;
	  }

	  // Checks top side 
	  if (d->pc_tunnel[y2 - 1][x2] < lowestplace){
	    lowestplace = d->pc_tunnel[y2 - 1][x2];
	    iny = -1;
	    inx = 0;
	  }

	  // Checks right side 
	  if (d->pc_tunnel[y2][x2 + 1] < lowestplace){
	   lowestplace = d->pc_tunnel[y2][x2 + 1];
	    iny = 0;
	    inx = 1;
	  }

	  // Checks bot side 
	  if (d->pc_tunnel[y2 + 1][x2] < lowestplace){
	    lowestplace = d->pc_tunnel[y2 + 1][x2];
	    iny = 1;
	    inx = 0;
	  }

	  d->monster[y2 + iny][x2 + inx].y = d->monster[y2][x2].y;
	  d->monster[y2 + iny][x2 + inx].x = d->monster[y2][x2].x;
	  d->monster[y2 + iny][x2 + inx].speed = d->monster[y2][x2].speed;
	  d->monster[y2 + iny][x2 + inx].turn = d->monster[y2][x2].turn;
	  d->monster[y2 + iny][x2 + inx].id = d->monster[y2][x2].id;
	  d->monster[y2 + iny][x2 + inx].type = d->monster[y2][x2].type;
	  d->monster[y2 + iny][x2 + inx].tun = d->monster[y2][x2].tun; 
	  d->monster[y2 + iny][x2 + inx].pic = d->monster[y2][x2].pic;

	  d->monster[y2][x2].id = 0;
	  d->monster[y2][x2].pic = '.';
	  d->monster[y2][x2].x = 0;
	  d->monster[y2][x2].y = 0;
	  d->monster[y2][x2].speed = 0;
	  d->monster[y2][x2].turn = 0;
	  d->monster[y2][x2].tun = 0;
	  d->monster[y2][x2].type = 0;
	  //-------------------------------------------------
	  
	  moved = 1;
	}

	
	
      }
  
  }
  
    

}



/* This changes the coordinates for pc, allowing it to move.
   Added by LC */ 
void movePC(dungeon_t *d){
  
    if(d->map[d->pc.position[dim_y] - 1][d->pc.position[dim_x]] == ter_floor_room){
      d->pc.position[dim_y] = d->pc.position[dim_y] - 1;
    }
    else if(d->map[d->pc.position[dim_y] + 1][d->pc.position[dim_x]] == ter_floor_room){
      d->pc.position[dim_y] = d->pc.position[dim_y] + 1;
    }
    else if(d->map[d->pc.position[dim_y]][d->pc.position[dim_x] - 1] == ter_floor_room){
      d->pc.position[dim_x] = d->pc.position[dim_x] - 1;
    }
    else if(d->map[d->pc.position[dim_y]][d->pc.position[dim_x] + 1] == ter_floor_room){
      d->pc.position[dim_x] = d->pc.position[dim_x] + 1;
    }
  
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
	  /* Case added by LC */
        case 'n':
	  if ((argc > i + 1) && argv[i + 1][0] != '-') {
	    sscanf(argv[++i], "%u", &numMonster);
	    printf("%u", numMonster);
	    printf("\n");
          }
	  else{
	    numMonster = 10;
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

  printf("Seed is %ld.\n", seed);
  srand(seed);

  init_dungeon(&d);

  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  if (!do_load) {
    /* Set a valid position for the PC */
    d.pc.position[dim_x] = (d.rooms[0].position[dim_x] +
                            (rand() % d.rooms[0].size[dim_x]));
    d.pc.position[dim_y] = (d.rooms[0].position[dim_y] +
                            (rand() % d.rooms[0].size[dim_y]));
  }


  printf("PC is at (y, x): %d, %d\n",
         d.pc.position[dim_y], d.pc.position[dim_x]);

 


 /*----------- This is added by LC ---------------------------------------------------------------*/
  d.pc.speed = 10; 
  d.pc.turn = 0;
  d.pc.id = 0;

  int mon;
  uint8_t type;
  uint8_t id = 1;

  uint8_t x2, y2;

  heap_t character;

  for (mon = 0; mon < numMonster; mon++){
    x2 = rand() % 80;
    y2 = rand() % 21;
    while (d.map[y2][x2] != ter_floor_room){
     x2 = rand() % 80;
     y2 = rand() % 21;
    }

    type = (rand() % 16)+1;

    d.monster[y2][x2].x = x2;
    d.monster[y2][x2].y = y2;
    d.monster[y2][x2].speed = 5 + (rand() % 15);
    d.monster[y2][x2].turn = 0;
    d.monster[y2][x2].id = id;
    d.monster[y2][x2].type = type;
    d.monster[y2][x2].tun = rand() % 1; ///debug

    switch (type) {
    case 1:
      d.monster[y2][x2].pic = 'a';
      break;
    case 2:
      d.monster[y2][x2].pic = 'b';
      break;
    case 3:
      d.monster[y2][x2].pic = 'c';
      break;
    case 4:
      d.monster[y2][x2].pic = 'd';
      break;
    case 5:
      d.monster[y2][x2].pic = 'e';
      break;
    case 6:
      d.monster[y2][x2].pic = 'f';
      break;
    case 7:
      d.monster[y2][x2].pic = 'g';
      break;
    case 8:
      d.monster[y2][x2].pic = 'h';
      break;
    case 9:
      d.monster[y2][x2].pic = 'i';
      break;
    case 10:
      d.monster[y2][x2].pic = 'j';
      break;
    case 11:
      d.monster[y2][x2].pic = 'k';
      break;
    case 12:
      d.monster[y2][x2].pic = 'l';
      break;
    case 13:
      d.monster[y2][x2].pic = 'm';
      break;
    case 14:
      d.monster[y2][x2].pic = 'n';
      break;
    case 15:
      d.monster[y2][x2].pic = 'o';
      break;
    case 16:
      d.monster[y2][x2].pic = 'p';
      break;
    }


    id++;
  }

 
  //  int c;
  



  dijkstra(&d);
  dijkstra_tunnel(&d);
  render_distance_map(&d);
  render_tunnel_distance_map(&d);
  render_hardness_map(&d);
  render_movement_cost_map(&d);

  // for(c = 0; c<3;c++){
  
     uint8_t pholder = 0;
     uint8_t x, y;
     struct player players[numMonster+1];

 
     struct player *temp;
     players[pholder].turn = d.pc.turn;
     players[pholder].id = pholder;

 
 
 
     heap_init(&character,monster_cmp,  NULL);
     heap_insert(&character, &players[pholder]);

     pholder++;
  
     for(y = 0; y < DUNGEON_Y; y++){
       for(x = 0; x < DUNGEON_X; x++){
	 if(d.monster[y][x].id > 0){
	   players[pholder].turn = d.monster[y][x].turn;
	   players[pholder].id = d.monster[y][x].id;
	   players[pholder].tun = d.monster[y][x].tun;

	   heap_insert(&character, &players[pholder]);

	   d.monster[y][x].turn = 1000 /d.monster[y][x].speed;
	   players[pholder].turn = d.monster[y][x].turn; 
	   pholder++;
	 }
       }
     }
  

 
     d.pc.turn = 1000/d.pc.speed;
  
    
     for(i = 0; i < numMonster+1; i++){
       temp = heap_remove_min(&character);
       printf("ID:\n");
       printf("%u  ", temp->id);

       if(temp->id == 0){
	 movePC(&d);
       }
       // else{
	 // moveMonster(&d, temp->id);
	 // }


       render_dungeon(&d);
       // }

   

   }

  

  if (do_save) {
    if (do_save_seed) {
       /* 10 bytes for number, plus dot, extention and null terminator. */
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

  delete_dungeon(&d);

  return 0;
}


