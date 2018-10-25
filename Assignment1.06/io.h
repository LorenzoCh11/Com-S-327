#ifndef IO_H
# define IO_H

typedef struct dungeon dungeon_t;
typedef struct pc pc_t;

void io_init_terminal(void);
void io_reset_terminal(void);
//Edited by LC
void io_display(pc_t *pd, dungeon_t *d);
//Edited by LC
void io_handle_input(pc_t *pd, dungeon_t *d);
void io_queue_message(const char *format, ...);

#endif
