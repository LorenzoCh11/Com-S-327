#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "event.h"

#include "ncurses.h"

void do_combat(dungeon_t *d, character_t *atk, character_t *def)
{
  if (def->alive) {
    def->alive = 0;
    if (def != &d->pc) {
      d->num_monsters--;
    }
    atk->kills[kill_direct]++;
    atk->kills[kill_avenged] += (def->kills[kill_direct] +
                                  def->kills[kill_avenged]);
  }
}

void move_character(dungeon_t *d, character_t *c, pair_t next)
{
  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x]))) {
    do_combat(d, c, charpair(next));
  } else {
    /* No character in new position. */

    d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->character[c->position[dim_y]][c->position[dim_x]] = c;
  }
}

void do_moves(dungeon_t *d, int key)
{
  pair_t next;
  character_t *c;
  event_t *e;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    /* The PC always goes first one a tie, so we don't use new_event().  *
     * We generate one manually so that we can set the PC sequence       *
     * number to zero.                                                   */
    e = malloc(sizeof (*e));
    e->type = event_character_turn;
    /* Hack: New dungeons are marked.  Unmark and ensure PC goes at d->time, *
     * otherwise, monsters get a turn before the PC.                         */
    if (d->is_new) {
      d->is_new = 0;
      e->time = d->time;
    } else {
      e->time = d->time + (1000 / d->pc.speed);
    }
    e->sequence = 0;
    e->c = &d->pc;
    heap_insert(&d->events, e);
  }

  while (pc_is_alive(d) &&
         (e = heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != &d->pc))) {
    d->time = e->time;
    if (e->type == event_character_turn) {
      c = e->c;
    }
    if (!c->alive) {
      if (d->character[c->position[dim_y]][c->position[dim_x]] == c) {
        d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
      }
      if (c != &d->pc) {
        event_delete(e);
      }
      continue;
    }

    npc_next_pos(d, c, next);
    move_character(d, c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / c->speed));
  }

  if (pc_is_alive(d) && e->c == &d->pc) {
    c = e->c;
    d->time = e->time;
    /* Kind of kludgey, but because the PC is never in the queue when   *
     * we are outside of this function, the PC event has to get deleted *
     * and recreated every time we leave and re-enter this function.    */
    e->c = NULL;
    event_delete(e);
    // pc_next_pos(d, next);
    // next[dim_x] += c->position[dim_x];
    //next[dim_y] += c->position[dim_y];
    // if (mappair(next) <= ter_floor) {
    // mappair(next) = ter_floor_hall;
    // }

    //Added by LC
    //Changed move_character(d, c, next);
    // move_pc(d, c, key);
    //top-left
     if (key == 'y' || key == '7'){
      move_pc(d, c, -1, -1);
    }
     else if (key == 'k' || key == '8'){
      move_pc(d, c, -1, 0);
    }
    // top-right
    else if (key == 'u' || key == '9'){
      move_pc(d, c, -1, 1);
    }
    // right
    else if (key == 'l' || key == '6'){
      move_pc(d, c, 0, 1);
    }
    // bot-right
    else if (key == 'n' || key == '3'){
      move_pc(d, c, 1, 1);
    }
    // bot
    else if (key == 'j' || key == '2'){
      move_pc(d, c, 1, 0);
    }
    // bot-left
    else if (key == 'b' || key == '1'){
      move_pc(d, c, 1, -1);
    }
    // left
    else if (key == 'h' || key == '4'){
      move_pc(d, c, 0, -1);
    }

    dijkstra(d);
    dijkstra_tunnel(d);
  }
}

void dir_nearest_wall(dungeon_t *d, character_t *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (c->position[dim_x] != 1 && c->position[dim_x] != DUNGEON_X - 2) {
    dir[dim_x] = (c->position[dim_x] > DUNGEON_X - c->position[dim_x] ? 1 : -1);
  }
  if (c->position[dim_y] != 1 && c->position[dim_y] != DUNGEON_Y - 2) {
    dir[dim_y] = (c->position[dim_y] > DUNGEON_Y - c->position[dim_y] ? 1 : -1);
  }
}

uint32_t against_wall(dungeon_t *d, character_t *c)
{
  return ((mapxy(c->position[dim_x] - 1,
                 c->position[dim_y]    ) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x] + 1,
                 c->position[dim_y]    ) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x]    ,
                 c->position[dim_y] - 1) == ter_wall_immutable) ||
          (mapxy(c->position[dim_x]    ,
                 c->position[dim_y] + 1) == ter_wall_immutable));
}

uint32_t in_corner(dungeon_t *d, character_t *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(c->position[dim_x] - 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x] + 1,
                          c->position[dim_y]    ) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] - 1) == ter_wall_immutable);
  num_immutable += (mapxy(c->position[dim_x]    ,
                          c->position[dim_y] + 1) == ter_wall_immutable);

  return num_immutable > 1;
}


//Changed by LC uint32_t to void
void move_pc(dungeon_t *d, character_t *c, int incry, int incrx)
{
  
  pair_t next;
  next[dim_y] = c->position[dim_y] + incry;
  next[dim_x] = c->position[dim_x] + incrx;

  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x]))) {
    do_combat(d, c, charpair(next));
  } else {
     

  d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
  if(d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_debug && 
     d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_wall &&
     d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_wall_immutable){
    c->position[dim_y] += incry;
    c->position[dim_x] += incrx;
  }
  else{
    c->position[dim_y] = c->position[dim_y];
    c->position[dim_x] = c->position[dim_x];
  }

  d->character[c->position[dim_y]][c->position[dim_x]] = c;

  }

  /*
  //This makes sure that it stays in a room or hall
  d->character[c->position[dim_y]][c->position[dim_x]] = NULL;
  if(d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_debug && 
     d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_wall &&
     d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_wall_immutable &&
     d->map[c->position[dim_y ] +incry][c->position[dim_x] + incrx] != ter_floor){
    c->position[dim_y] += incry;
    c->position[dim_x] += incrx;
  }
  else{
    c->position[dim_y] = c->position[dim_y];
    c->position[dim_x] = c->position[dim_x];
  }

  d->character[c->position[dim_y]][c->position[dim_x]] = c;

  */
  
}
