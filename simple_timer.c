#include "simple_timer.h"

void start_timer(simple_timer_t *timer)
{
  switch(timer->state) {
  case STOPPED:
    timer->start_tick = timer->func();
    break;
  default:
    break;
  }

  timer->state = RUNNING;
}

void pause_timer(simple_timer_t *timer)
{
  switch(timer->state) {
  case RUNNING:
    timer->state = PAUSED;
    break;
  default:
    break;
  }
}

void stop_timer(simple_timer_t *timer)
{
  timer->state = STOPPED;
}

void reset_timer(simple_timer_t *timer)
{
  stop_timer(timer);
}

void update_timer(simple_timer_t *timer)
{
  timer->cur_tick = timer->func();
  unsigned int dt = timer->cur_tick - timer->prev_update_tick;

  switch(timer->state) {
  case PAUSED:
    timer->pause_ticks += dt;
    break;
  default:
    break;
  }

  timer->prev_update_tick = timer->cur_tick;
}

unsigned int elapsed_time(simple_timer_t *timer)
{
  unsigned int time = timer->cur_tick - timer->start_tick - timer->pause_ticks;
  switch(timer->state) {
  case STOPPED:
    time = 0;
    break;
  default:
    break;
  }

  return time;
}

simple_timer_t *create_timer(tick_func func)
{
  simple_timer_t *timer = malloc(sizeof(simple_timer_t));
  timer->cur_tick = 0;
  timer->start_tick = 0;
  timer->pause_ticks = 0;
  timer->prev_update_tick = 0;
  timer->func = func;
  timer->state = STOPPED;
  return timer;
}

void destroy_timer(simple_timer_t *timer)
{
  free(timer);
}

