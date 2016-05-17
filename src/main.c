#include <pebble.h>


// Define struct to store colors for each time unit
typedef struct Palette {
  GColor seconds;
  GColor minutes;
  GColor hours;
} Palette;

static Window *s_window;
static Layer *s_layer;
static Palette *s_palette;

static uint8_t s_hour;
static uint8_t s_minute;
static uint8_t s_second;


void draw_border(GContext *ctx, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t sw, uint8_t segment, uint8_t total_segments){

   // Calculate max no of pixels in perimeter less the size of the border at the four corners
   uint16_t perimmax = ((w*2)+(h*2)) - (sw*4);

   // Calculate the corners
   uint16_t c1 = w / 2;	      // TOP LEFT
   uint16_t c2 = c1 + h - sw;	// BOTTOM RIGHT
   uint16_t c3 = c2 + w - sw;	// BOTTOM LEFT
   uint16_t c4 = c3 + h - sw;  // TOP LEFT

   // Calculate our current position around the perimiter
   uint16_t perimpos = (segment * perimmax) / total_segments;

   // Make sure we don't exceed maximum
  if (perimpos > perimmax) {perimpos = perimmax;}
  
   // Prefill any completed sides
   if (perimpos > c1) { graphics_fill_rect(ctx, GRect((w/2)+x, y, w/2, sw), 0, GCornerNone);}	  // Prefill top right
   if (perimpos > c2) { graphics_fill_rect(ctx, GRect(w-sw+x, sw+y, sw, h-sw), 0, GCornerNone);} // prefill right
   if (perimpos > c3) { graphics_fill_rect(ctx, GRect(x, h-sw+y, w-sw, sw), 0, GCornerNone);} // prefill bottom
   if (perimpos > c4) { graphics_fill_rect(ctx, GRect(x, y, sw, h-sw), 0, GCornerNone);} // fill left

   // Draw from the last filled side to our current position
   if (perimpos >= c4) {
	  // TOP LEFT to TOP MIDDLE
	  graphics_fill_rect(ctx, GRect(sw+x, y, perimpos-c4, sw), 0, GCornerNone);
   }
  else if (perimpos <= c1) {
	  // TOP MIDDLE to TOP RIGHT
	  graphics_fill_rect(ctx, GRect((w/2)+x, y, perimpos, sw), 0, GCornerNone);
  }
  else if (perimpos <= c2) {
	  // TOP RIGHT to BOTTOM RIGHT
	  graphics_fill_rect(ctx, GRect(w-sw+x, sw+y, sw, perimpos-c1), 0, GCornerNone);
  }
  else if (perimpos <= c3) {
	  // BOTTOM RIGHT to BOTTOM LEFT
	  graphics_fill_rect(ctx, GRect(w-sw-(perimpos-c2)+x, h-sw+y, perimpos-c2, sw), 0, GCornerNone);
  }
  else if (perimpos < c4) {
	  // BOTTOM LEFT to TOP LEFT
	  graphics_fill_rect(ctx, GRect(x, h-sw-(perimpos-c3)+y, sw, perimpos-c3), 0, GCornerNone);
  }
  
}
  
  
// Handle representation of seconds
void draw_seconds(GContext *ctx, uint8_t seconds, Layer *layer) {
  uint8_t padding = 0;
  uint8_t stroke_width = 14;
  GRect bounds = layer_get_bounds(layer);
  uint8_t width = bounds.size.w;
  uint8_t height = bounds.size.h;
  
  //draw_border(ctx, calculate_border(layer, padding, SCALED_TIME(seconds)), padding, stroke_width);
  draw_border(ctx, padding, padding, width, height, stroke_width, seconds+1, 60);
}

// Handle representation of minutes
void draw_minutes(GContext *ctx, uint8_t minutes, Layer *layer) {
  uint8_t padding = 20;
  uint8_t stroke_width = 14;
  GRect bounds = layer_get_bounds(layer);
  uint8_t width = bounds.size.w;
  uint8_t height = bounds.size.h;
  
  //draw_border(ctx, calculate_border(layer, padding, SCALED_TIME(minutes)), padding, stroke_width);
  draw_border(ctx, padding, padding, width-(padding*2), height-(padding*2), stroke_width, minutes+1, 60);
}

// Handle representation of hours
void draw_hours(GContext *ctx, uint8_t hours, Layer *layer) {
  uint8_t padding = 40;
  uint8_t stroke_width = 14;
  GRect bounds = layer_get_bounds(layer);
  uint8_t width = bounds.size.w;
  uint8_t height = bounds.size.h;
  
  //draw_border(ctx, calculate_border(layer, padding, SCALED_HOUR(hours)), padding, stroke_width);
  
  if (hours > 12) { hours -= 12;}
  if (hours == 0) { hours = 12;}
  
  draw_border(ctx, padding, padding, width-(padding*2), height-(padding*2), stroke_width, hours, 12);
}



// Set the color for drawing routines 
static void set_color(GContext *ctx, GColor color) {
  graphics_context_set_fill_color(ctx, color);
}

// Update the watchface display
static void update_display(Layer *layer, GContext *ctx) {
  set_color(ctx, s_palette->seconds);
  draw_seconds(ctx, s_second, layer);

  set_color(ctx, s_palette->minutes);
  draw_minutes(ctx, s_minute, layer);

  set_color(ctx, s_palette->hours);
  draw_hours(ctx, s_hour % 12, layer);
}

// Update the current time values for the watchface
static void update_time(struct tm *tick_time) {
  s_hour = tick_time->tm_hour;
  s_minute = tick_time->tm_min;
  s_second = tick_time->tm_sec;
  layer_mark_dirty(s_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void window_load(Window *window) {
  s_palette = malloc(sizeof(Palette));
  *s_palette = (Palette) {
      COLOR_FALLBACK(GColorRichBrilliantLavender,GColorWhite),
      COLOR_FALLBACK(GColorVividViolet,GColorWhite),
      COLOR_FALLBACK(GColorBlueMoon,GColorWhite)
  };
  
  s_layer = layer_create(layer_get_bounds(window_get_root_layer(s_window)));
  layer_add_child(window_get_root_layer(s_window), s_layer);
  layer_set_update_proc(s_layer, update_display);
}

static void window_unload(Window *window) {
  free(s_palette);
  layer_destroy(s_layer);
}

static void init(void) {
  s_window = window_create();
  
#ifdef PBL_SDK_2
  window_set_fullscreen(s_window, true);
#endif
  
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_set_background_color(s_window, GColorBlack);
  window_stack_push(s_window, true);

  time_t start = time(NULL);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  update_time(localtime(&start));
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}