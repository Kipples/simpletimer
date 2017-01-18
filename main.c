#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <ulfius.h>

#include "simple_timer.h"

#define WINDOW_WIDTH 290
#define WINDOW_HEIGHT 60
#define PORT 9010

static const int sdl_img_flags = IMG_INIT_JPG | IMG_INIT_PNG;

static const char *font_file = "Hoshi.png";
static const char *font_char_set = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";
static const unsigned int font_width = 8;
static const unsigned int font_height = 9;
static const unsigned int font_spacing = 1;

static SDL_Texture *font_texture;

static SDL_Color timer_color = { .r = 255, .g = 255, .b = 255, .a = 255};

static void timer_to_json(simple_timer_t *timer, json_t *json) {
  switch(timer->state) {
  case RUNNING:
    json_object_set_new(json, "timer_state", json_string("running"));
    break;
  case STOPPED:
    json_object_set_new(json, "timer_state", json_string("stopped"));
    break;
  case PAUSED:
    json_object_set_new(json, "timer_state", json_string("paused"));
    break;
  }
}

int timer_start_PUT(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  simple_timer_t *timer = (simple_timer_t *)user_data;
  json_t *json = json_object();

  start_timer(timer);

  timer_to_json(timer, json);
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

int timer_reset_PUT(const struct _u_request *request, struct _u_response *response, void *user_data)
{
  simple_timer_t *timer = (simple_timer_t *)user_data;
  json_t *json = json_object();

  reset_timer(timer);

  timer_to_json(timer, json);
  
  ulfius_set_json_response(response, 200, json);
  json_decref(json);
  return U_OK;
}

static int load_texture(const char *file, SDL_Texture **texture, SDL_Renderer *renderer)
{
  SDL_Surface *surface;

  surface = IMG_Load(file);
  if(!surface) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "failed to load font: %s", IMG_GetError());
    return -1;
  }
  
  if(SDL_SetColorKey(surface, SDL_TRUE, 0) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "failed to set color key: %s", SDL_GetError());
    return -1;
  }

  *texture = SDL_CreateTextureFromSurface(renderer, surface);

  if(!(*texture)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
    return -1;
  }
  SDL_FreeSurface(surface);

  return 0;
}

int render_char(char c, int x, int y, SDL_Renderer *renderer, SDL_Color *color, float scale)
{
  int i;

  SDL_Rect src;
  SDL_Rect dst;
  
  for(i = 0; font_char_set[i] != c; i++);

  src.x = (i * font_width) + (i * font_spacing);
  src.y = 0;
  src.w = font_width;
  src.h = font_height;

  dst.x = x;
  dst.y = y;
  dst.w = font_width * scale;
  dst.h = font_height * scale;

  if(color) {
    SDL_SetTextureColorMod(font_texture, color->r, color->g, color->b);
  }

  SDL_RenderCopy(renderer, font_texture, &src, &dst);
  
  return 0;
}

int render_string(char *string, int x, int y, SDL_Renderer *renderer, SDL_Color *color, float scale)
{
  int i;
  for(i = 0; string[i] != '\0'; i++) {
    render_char(string[i], x + (i * (font_width * scale)) - scale, y, renderer, color, scale);
  }
  return 0;
}

void render_timer(simple_timer_t *timer, SDL_Renderer *renderer)
{
  unsigned int time = elapsed_time(timer);
  unsigned int hr, mn, sc, ms;
  hr = time / (1000 * 60 * 60);
  mn = (time / (1000 * 60)) % 60;
  sc = (time / 1000) % 60;
  ms = time % 1000;

  char str[32];
  sprintf(str, "%02d:%02d:%02d", hr, mn, sc);

  render_string(str, 33, 10, renderer, &timer_color, 4.0);
}

int main(int argc, char **argv)
{
  SDL_Window *window;
  SDL_Renderer *renderer;

  SDL_Event e;
  
  int quit = 0;

  simple_timer_t *timer = create_timer(&SDL_GetTicks);
  
  struct _u_instance instance;

  ulfius_init_instance(&instance, PORT, NULL);

  ulfius_add_endpoint_by_val(&instance,
			     "PUT",
			     "/api",
			     "/timer/start",
			     NULL,
			     NULL,
			     NULL,
			     timer_start_PUT,
			     (void *)timer);
			     
  ulfius_add_endpoint_by_val(&instance,
			     "PUT",
			     "/api",
			     "/timer/reset",
			     NULL,
			     NULL,
			     NULL,
			     timer_reset_PUT,
			     (void *)timer);			     

  ulfius_start_framework(&instance);


  
  if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    return 3;
  }

  if(IMG_Init(sdl_img_flags) & sdl_img_flags != sdl_img_flags) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to init required jpg and png support: %s", IMG_GetError());
    return 3;
  }

  if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_HEIGHT,
				 SDL_WINDOW_OPENGL, &window, &renderer) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
    return 3;
  }

  SDL_SetWindowTitle(window, "Timer");

  load_texture(font_file, &font_texture, renderer);

  while(!quit) {
    SDL_PollEvent(&e);
    switch(e.type) {
    case SDL_QUIT:
      quit = 1;
      break;
    }
    update_timer(timer);
    SDL_RenderClear(renderer);
    render_timer(timer, renderer);
    SDL_RenderPresent(renderer);
  }

  destroy_timer(timer);
  
  IMG_Quit();
  SDL_Quit();

  return 0;
}
