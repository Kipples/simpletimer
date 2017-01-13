#include <stdlib.h>

typedef unsigned int (*tick_func)();

typedef enum { RUNNING, STOPPED, PAUSED } timer_state_t;

typedef struct simple_timer_t {
  unsigned int start_tick;
  unsigned int pause_ticks;
  unsigned int prev_update_tick;
  unsigned int cur_tick;
  tick_func func;
  timer_state_t state;
} simple_timer_t;

void start_timer(simple_timer_t *timer);

void pause_timer(simple_timer_t *timer);

void stop_timer(simple_timer_t *timer);

void reset_timer(simple_timer_t *timer);

void update_timer(simple_timer_t *timer);

unsigned int elapsed_time(simple_timer_t *timer);

simple_timer_t *create_timer(tick_func func);

void destroy_timer(simple_timer_t *timer);
