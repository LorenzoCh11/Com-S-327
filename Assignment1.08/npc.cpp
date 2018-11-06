#include <stdlib.h>

#include "utils.h"
#include "npc.h"
#include "dungeon.h"
#include "character.h"
#include "move.h"
#include "path.h"
#include "event.h"
#include "pc.h"

//added by LC
#include "descriptions.h"

/*
//added by LC
static void addMonsters(dungeon *d) {

}
*/

//changed this by LC
/*
static uint32_t max_monster_cells(dungeon *d)
{
  uint32_t i;
  uint32_t sum;

  for (i = sum = 0; i < d->num_rooms; i++) {
    if (!pc_in_room(d, i)) {
      sum += d->rooms[i].size[dim_y] * d->rooms[i].size[dim_x];
    }
  }

  return sum;
}
*/
void gen_monsters(dungeon *d)
{
  uint32_t i;
  npc *m;
  uint32_t room;
  pair_t p;
  // int x,y;
  //const static char symbol[] = "0123456789abcdef";//changed by LC
  //uint32_t num_cells; //Changed by LC
  //added by LC
  //int index = 0;

  //num_cells = max_monster_cells(d); changed by LC
  //d->num_monsters = d->max_monsters < num_cells ? d->max_monsters : num_cells; changed by LC
  d->num_monsters = d->monster_descriptions.size();
  for (i = 0; i < d->num_monsters; i++) {
    m = new npc;
    memset(m, 0, sizeof (*m));
    
    do {
      room = rand_range(1, d->num_rooms - 1);
      p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                            (d->rooms[room].position[dim_y] +
                             d->rooms[room].size[dim_y] - 1));
      p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                            (d->rooms[room].position[dim_x] +
                             d->rooms[room].size[dim_x] - 1));
    } while (d->character_map[p[dim_y]][p[dim_x]]);
    //added by LC
    d->moncolor[p[dim_y]][p[dim_x]] = d->monster_descriptions.at(i).get_npc().get_color(); //added by LC
    m->position[dim_y] = p[dim_y];
    m->position[dim_x] = p[dim_x];
    d->character_map[p[dim_y]][p[dim_x]] = m;
    // m->speed = rand_range(5, 20);
    m->speed = d->monster_descriptions.at(i).get_npc().get_speed(); //added by LC
    m->color = d->monster_descriptions.at(i).get_npc().get_color(); //added by LC
    m->alive = 1;
    m->sequence_number = ++d->character_sequence_number;
    //m->characteristics = rand() & 0x0000000f;
    m->characteristics = d->monster_descriptions.at(i).get_npc().get_abilities(); //added by LC
    /*    m->npc->characteristics = 0xf;*/
    //m->symbol = symbol[m->characteristics];
    m->symbol = d->monster_descriptions.at(i).get_npc().get_symbol();//added b LC
    m->have_seen_pc = 0;
    m->kills[kill_direct] = m->kills[kill_avenged] = 0;

    d->character_map[p[dim_y]][p[dim_x]] = m;
    d->moncolor[p[dim_y]][p[dim_x]] = d->monster_descriptions.at(i).get_npc().get_color(); //added by LC

    heap_insert(&d->events, new_event(d, event_characterurn, m, 0));
    //index++;
  }
}

void npc_next_pos_rand_tunnel(dungeon *d, npc *c, pair_t next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n[dim_y] = next[dim_y];
    n[dim_x] = next[dim_x];
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n[dim_y]--;
      } else {
        n[dim_y]++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n[dim_x]--;
      } else {
        n[dim_x]++;
      }
    }
  } while (mappair(n) == ter_wall_immutable);

  if (hardnesspair(n) <= 85) {
    if (hardnesspair(n)) {
      hardnesspair(n) = 0;
      mappair(n) = ter_floor_hall;

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    }

    next[dim_x] = n[dim_x];
    next[dim_y] = n[dim_y];
  } else {
    hardnesspair(n) -= 85;
  }
}

void npc_next_pos_rand(dungeon *d, npc *c, pair_t next)
{
  pair_t n;
  union {
    uint32_t i;
    uint8_t a[4];
  } r;

  do {
    n[dim_y] = next[dim_y];
    n[dim_x] = next[dim_x];
    r.i = rand();
    if (r.a[0] > 85 /* 255 / 3 */) {
      if (r.a[0] & 1) {
        n[dim_y]--;
      } else {
        n[dim_y]++;
      }
    }
    if (r.a[1] > 85 /* 255 / 3 */) {
      if (r.a[1] & 1) {
        n[dim_x]--;
      } else {
        n[dim_x]++;
      }
    }
  } while (mappair(n) < ter_floor);

  next[dim_y] = n[dim_y];
  next[dim_x] = n[dim_x];
}

void npc_next_pos_line_of_sight(dungeon *d, npc *c, pair_t next)
{
  pair_t dir;

  dir[dim_y] = d->PC->position[dim_y] - c->position[dim_y];
  dir[dim_x] = d->PC->position[dim_x] - c->position[dim_x];
  if (dir[dim_y]) {
    dir[dim_y] /= abs(dir[dim_y]);
  }
  if (dir[dim_x]) {
    dir[dim_x] /= abs(dir[dim_x]);
  }

  if (mapxy(next[dim_x] + dir[dim_x],
            next[dim_y] + dir[dim_y]) >= ter_floor) {
    next[dim_x] += dir[dim_x];
    next[dim_y] += dir[dim_y];
  } else if (mapxy(next[dim_x] + dir[dim_x], next[dim_y]) >= ter_floor) {
    next[dim_x] += dir[dim_x];
  } else if (mapxy(next[dim_x], next[dim_y] + dir[dim_y]) >= ter_floor) {
    next[dim_y] += dir[dim_y];
  }
}

void npc_next_pos_line_of_sight_tunnel(dungeon *d,
                                       npc *c,
                                       pair_t next)
{
  pair_t dir;

  dir[dim_y] = d->PC->position[dim_y] - c->position[dim_y];
  dir[dim_x] = d->PC->position[dim_x] - c->position[dim_x];
  if (dir[dim_y]) {
    dir[dim_y] /= abs(dir[dim_y]);
  }
  if (dir[dim_x]) {
    dir[dim_x] /= abs(dir[dim_x]);
  }

  dir[dim_x] += next[dim_x];
  dir[dim_y] += next[dim_y];

  if (hardnesspair(dir) <= 85) {
    if (hardnesspair(dir)) {
      hardnesspair(dir) = 0;
      mappair(dir) = ter_floor_hall;

      /* Update distance maps because map has changed. */
      dijkstra(d);
      dijkstra_tunnel(d);
    }

    next[dim_x] = dir[dim_x];
    next[dim_y] = dir[dim_y];
  } else {
    hardnesspair(dir) -= 85;
  }
}

void npc_next_pos_gradient(dungeon *d, npc *c, pair_t next)
{
  /* Handles both tunneling and non-tunneling versions */
  pair_t min_next;
  uint16_t min_cost;
  if (c->characteristics & NPC_TUNNEL) {
    min_cost = (d->pc_tunnel[next[dim_y] - 1][next[dim_x]] +
                (d->hardness[next[dim_y] - 1][next[dim_x]] / 85));
    min_next[dim_x] = next[dim_x];
    min_next[dim_y] = next[dim_y] - 1;
    if ((d->pc_tunnel[next[dim_y] + 1][next[dim_x]    ] +
         (d->hardness[next[dim_y] + 1][next[dim_x]] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] + 1][next[dim_x]] +
                  (d->hardness[next[dim_y] + 1][next[dim_x]] / 85));
      min_next[dim_x] = next[dim_x];
      min_next[dim_y] = next[dim_y] + 1;
    }
    if ((d->pc_tunnel[next[dim_y]    ][next[dim_x] + 1] +
         (d->hardness[next[dim_y]    ][next[dim_x] + 1] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y]][next[dim_x] + 1] +
                  (d->hardness[next[dim_y]][next[dim_x] + 1] / 85));
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y];
    }
    if ((d->pc_tunnel[next[dim_y]    ][next[dim_x] - 1] +
         (d->hardness[next[dim_y]    ][next[dim_x] - 1] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y]][next[dim_x] - 1] +
                  (d->hardness[next[dim_y]][next[dim_x] - 1] / 85));
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y];
    }
    if ((d->pc_tunnel[next[dim_y] - 1][next[dim_x] + 1] +
         (d->hardness[next[dim_y] - 1][next[dim_x] + 1] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] - 1][next[dim_x] + 1] +
                  (d->hardness[next[dim_y] - 1][next[dim_x] + 1] / 85));
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y] - 1;
    }
    if ((d->pc_tunnel[next[dim_y] + 1][next[dim_x] + 1] +
         (d->hardness[next[dim_y] + 1][next[dim_x] + 1] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] + 1][next[dim_x] + 1] +
                  (d->hardness[next[dim_y] + 1][next[dim_x] + 1] / 85));
      min_next[dim_x] = next[dim_x] + 1;
      min_next[dim_y] = next[dim_y] + 1;
    }
    if ((d->pc_tunnel[next[dim_y] - 1][next[dim_x] - 1] +
         (d->hardness[next[dim_y] - 1][next[dim_x] - 1] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] - 1][next[dim_x] - 1] +
                  (d->hardness[next[dim_y] - 1][next[dim_x] - 1] / 85));
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y] - 1;
    }
    if ((d->pc_tunnel[next[dim_y] + 1][next[dim_x] - 1] +
         (d->hardness[next[dim_y] + 1][next[dim_x] - 1] / 85)) < min_cost) {
      min_cost = (d->pc_tunnel[next[dim_y] + 1][next[dim_x] - 1] +
                  (d->hardness[next[dim_y] + 1][next[dim_x] - 1] / 85));
      min_next[dim_x] = next[dim_x] - 1;
      min_next[dim_y] = next[dim_y] + 1;
    }
    if (hardnesspair(min_next) <= 85) {
      if (hardnesspair(min_next)) {
        hardnesspair(min_next) = 0;
        mappair(min_next) = ter_floor_hall;

        /* Update distance maps because map has changed. */
        dijkstra(d);
        dijkstra_tunnel(d);
      }

      next[dim_x] = min_next[dim_x];
      next[dim_y] = min_next[dim_y];
    } else {
      hardnesspair(min_next) -= 85;
    }
  } else {
    /* Make monsters prefer cardinal directions */
    if (d->pc_distance[next[dim_y] - 1][next[dim_x]    ] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x]    ] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      return;
    }
    if (d->pc_distance[next[dim_y]    ][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y]    ][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_x]--;
      return;
    }
    if (d->pc_distance[next[dim_y] - 1][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x] + 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      next[dim_x]++;
      return;
    }
    if (d->pc_distance[next[dim_y] - 1][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]--;
      next[dim_x]--;
      return;
    }
    if (d->pc_distance[next[dim_y] + 1][next[dim_x] - 1] <
        d->pc_distance[next[dim_y]][next[dim_x]]) {
      next[dim_y]++;
      next[dim_x]--;
      return;
    }
  }
}

static void npc_next_pos_00(dungeon *d, npc *c, pair_t next)
{
  /* not smart; not telepathic; not tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->PC), 0, 0)) {
    c->pc_last_known_position[dim_y] = d->PC->position[dim_y];
    c->pc_last_known_position[dim_x] = d->PC->position[dim_x];
    npc_next_pos_line_of_sight(d, c, next);
  } else {
    npc_next_pos_rand(d, c, next);
  }
}

static void npc_next_pos_01(dungeon *d, npc *c, pair_t next)
{
  /*     smart; not telepathic; not tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->PC), 0, 0)) {
    c->pc_last_known_position[dim_y] = d->PC->position[dim_y];
    c->pc_last_known_position[dim_x] = d->PC->position[dim_x];
    c->have_seen_pc = 1;
    npc_next_pos_line_of_sight(d, c, next);
  } else if (c->have_seen_pc) {
    npc_next_pos_line_of_sight(d, c, next);
  }

  if ((next[dim_x] == c->pc_last_known_position[dim_x]) &&
      (next[dim_y] == c->pc_last_known_position[dim_y])) {
    c->have_seen_pc = 0;
  }
}

static void npc_next_pos_02(dungeon *d, npc *c, pair_t next)
{
  /* not smart;     telepathic; not tunneling; not erratic */
  c->pc_last_known_position[dim_y] = d->PC->position[dim_y];
  c->pc_last_known_position[dim_x] = d->PC->position[dim_x];
  npc_next_pos_line_of_sight(d, c, next);
}

static void npc_next_pos_03(dungeon *d, npc *c, pair_t next)
{
  /*     smart;     telepathic; not tunneling; not erratic */
  npc_next_pos_gradient(d, c, next);
}

static void npc_next_pos_04(dungeon *d, npc *c, pair_t next)
{
  /* not smart; not telepathic;     tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->PC), 0, 0)) {
    c->pc_last_known_position[dim_y] = d->PC->position[dim_y];
    c->pc_last_known_position[dim_x] = d->PC->position[dim_x];
    npc_next_pos_line_of_sight(d, c, next);
  } else {
    npc_next_pos_rand_tunnel(d, c, next);
  }
}

static void npc_next_pos_05(dungeon *d, npc *c, pair_t next)
{
  /*     smart; not telepathic;     tunneling; not erratic */
  if (can_see(d, character_get_pos(c), character_get_pos(d->PC), 0, 0)) {
    c->pc_last_known_position[dim_y] = d->PC->position[dim_y];
    c->pc_last_known_position[dim_x] = d->PC->position[dim_x];
    c->have_seen_pc = 1;
    npc_next_pos_line_of_sight(d, c, next);
  } else if (c->have_seen_pc) {
    npc_next_pos_line_of_sight_tunnel(d, c, next);
  }

  if ((next[dim_x] == c->pc_last_known_position[dim_x]) &&
      (next[dim_y] == c->pc_last_known_position[dim_y])) {
    c->have_seen_pc = 0;
  }
}

static void npc_next_pos_06(dungeon *d, npc *c, pair_t next)
{
  /* not smart;     telepathic;     tunneling; not erratic */
  c->pc_last_known_position[dim_y] = d->PC->position[dim_y];
  c->pc_last_known_position[dim_x] = d->PC->position[dim_x];
  npc_next_pos_line_of_sight_tunnel(d, c, next);
}

static void npc_next_pos_07(dungeon *d, npc *c, pair_t next)
{
  /*     smart;     telepathic;     tunneling; not erratic */
  npc_next_pos_gradient(d, c, next);
}

static void npc_next_pos_08(dungeon *d, npc *c, pair_t next)
{
  /* not smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_00(d, c, next);
  }
}

static void npc_next_pos_09(dungeon *d, npc *c, pair_t next)
{
  /*     smart; not telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_01(d, c, next);
  }
}

static void npc_next_pos_0a(dungeon *d, npc *c, pair_t next)
{
  /* not smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_02(d, c, next);
  }
}

static void npc_next_pos_0b(dungeon *d, npc *c, pair_t next)
{
  /*     smart;     telepathic; not tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand(d, c, next);
  } else {
    npc_next_pos_03(d, c, next);
  }
}

static void npc_next_pos_0c(dungeon *d, npc *c, pair_t next)
{
  /* not smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_04(d, c, next);
  }
}

static void npc_next_pos_0d(dungeon *d, npc *c, pair_t next)
{
  /*     smart; not telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_05(d, c, next);
  }
}

static void npc_next_pos_0e(dungeon *d, npc *c, pair_t next)
{
  /* not smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_06(d, c, next);
  }
}

static void npc_next_pos_0f(dungeon *d, npc *c, pair_t next)
{
  /*     smart;     telepathic;     tunneling;     erratic */
  if (rand() & 1) {
    npc_next_pos_rand_tunnel(d, c, next);
  } else {
    npc_next_pos_07(d, c, next);
  }
}

void (*npc_move_func[])(dungeon *d, npc *c, pair_t next) = {
  /* We'll have one function for each combination of bits, so the *
   * order is based on binary counting through the NPC_* bits.    *
   * It could be very easy to mess this up, so be careful.  We'll *
   * name them according to their hex value.                      */
  npc_next_pos_00,
  npc_next_pos_01,
  npc_next_pos_02,
  npc_next_pos_03,
  npc_next_pos_04,
  npc_next_pos_05,
  npc_next_pos_06,
  npc_next_pos_07,
  npc_next_pos_08,
  npc_next_pos_09,
  npc_next_pos_0a,
  npc_next_pos_0b,
  npc_next_pos_0c,
  npc_next_pos_0d,
  npc_next_pos_0e,
  npc_next_pos_0f,
};

void npc_next_pos(dungeon *d, npc *c, pair_t next)
{
  next[dim_y] = c->position[dim_y];
  next[dim_x] = c->position[dim_x];

  npc_move_func[c->characteristics & 0x0000000f](d, c, next);
}

uint32_t dungeon_has_npcs(dungeon *d)
{
  return d->num_monsters;
}


//added by LC
void npc::set(const std::string &name,
	      const std::string &description,
	      const char symbol,
	      const uint32_t color,
	      const uint32_t speed,
	      const uint32_t abilities,
	      const uint32_t hitpoints,
	      const dice &damage,
	      const uint32_t rarity)
{
  this->symbol = symbol;
  this->speed = speed;
  this->color = color;
  this->abilities = abilities;

}

