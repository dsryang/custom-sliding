#include <pebble.h>

static Window *main_window;
static TextLayer *time_layer, *date_layer;
static Layer *light_layer, *dark_layer;
static GFont time_font;
static PropertyAnimation *property_animation_light, *property_animation_dark;
static Animation *animation_light, *animation_dark, *layer_slide;
static int day_of_week;
static int light_color_hex;
static int dark_color_hex;

#define KEY_LIGHT_COLOR 0
#define KEY_DARK_COLOR 1
#define KEY_TIME_COLOR 2
#define KEY_DATE_COLOR 3

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(time_layer, buffer);
}

static void update_date() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *date = localtime(&temp);
  
  static char full_date[16];
  day_of_week = date->tm_wday;
  int month = date->tm_mon;
  char day[] = "31";
  
  full_date[0] = '\0';
  
  // Get current date
  strftime(day, sizeof("31"), "%d", date);
  
  switch (day_of_week) {
    case 0:
      strcat(full_date, "SUN, ");
      break;
    case 1:
      strcat(full_date, "MON, ");
      break;
    case 2:
      strcat(full_date, "TUES, ");
      break;
    case 3:
      strcat(full_date, "WED, ");
      break;
    case 4:
      strcat(full_date, "THURS, ");
      break;
    case 5:
      strcat(full_date, "FRI, ");
      break;
    case 6:
      strcat(full_date, "SAT, ");
      break;
  }
  
  switch (month) {
    case 0:
      strcat(full_date, "JAN ");
      break;
    case 1:
      strcat(full_date, "FEB ");
      break;
    case 2:
      strcat(full_date, "MAR ");
      break;
    case 3:
      strcat(full_date, "APR ");
      break;
    case 4:
      strcat(full_date, "MAY ");
      break;
    case 5:
      strcat(full_date, "JUNE ");
      break;
    case 6:
      strcat(full_date, "JULY ");
      break;
    case 7:
      strcat(full_date, "AUG ");
      break;
    case 8:
      strcat(full_date, "SEPT ");
      break;
    case 9:
      strcat(full_date, "OCT ");
      break;
    case 10:
      strcat(full_date, "NOV ");
      break;
    case 11:
      strcat(full_date, "DEC ");
      break;
  }
  
  if (day[0] == '0') {
    strcat(full_date, &day[1]);
  }
  else {
    strcat(full_date, day);
  }

  // Display this date on the TextLayer
  text_layer_set_text(date_layer, full_date);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if (tick_time->tm_wday != day_of_week) {
    update_date();
  }
}

static void draw_light_layer(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GColor light_color = GColorFromHEX(light_color_hex);

  // Draw a light blue filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, light_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void draw_dark_layer(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GColor dark_color = GColorFromHEX(dark_color_hex);

  // Draw a dark blue filled rectangle with sharp corners
  graphics_context_set_fill_color(ctx, dark_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void set_time_color(int color) {
  GColor time_color = GColorFromHEX(color);
  text_layer_set_text_color(time_layer, time_color);
}

static void set_date_color(int color) {
  GColor date_color = GColorFromHEX(color);
  text_layer_set_text_color(date_layer, date_color);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *light_color_t = dict_find(iter, KEY_LIGHT_COLOR);
  Tuple *dark_color_t = dict_find(iter, KEY_DARK_COLOR);
  Tuple *time_color_t = dict_find(iter, KEY_TIME_COLOR);
  Tuple *date_color_t = dict_find(iter, KEY_DATE_COLOR);

  if (light_color_t) {
    light_color_hex = light_color_t->value->int32;

    persist_write_int(KEY_LIGHT_COLOR, light_color_hex);

    layer_set_update_proc(light_layer, draw_light_layer);
  }

  if (dark_color_t) {
    dark_color_hex = dark_color_t->value->int32;

    persist_write_int(KEY_DARK_COLOR, dark_color_hex);

    layer_set_update_proc(dark_layer, draw_dark_layer);
  }

  if (time_color_t) {
    int time_color = time_color_t->value->int32;

    persist_write_int(KEY_TIME_COLOR, time_color);

    set_time_color(time_color);
  }

  if (date_color_t) {
    int date_color = date_color_t->value->int32;

    persist_write_int(KEY_DATE_COLOR, date_color);

    set_time_color(date_color);
  }
}

static void animations(int16_t window_width, int16_t window_height) {
  // Slide from left animation for light background layer
  GRect light_start = GRect(-window_width, 0, 0, window_height);
  GRect light_end = GRect(0, 0, window_width, window_height);
  property_animation_light = property_animation_create_layer_frame(light_layer, &light_start, &light_end);
  animation_light = property_animation_get_animation(property_animation_light);
  animation_set_duration(animation_light, 800);
  
  // Slide from right animation for dark background layer
  int16_t dark_height = 82;
  int16_t dark_y = (window_height / 2) - (dark_height / 2);
  GRect dark_start = GRect(window_width, dark_y, window_width, dark_height);
  GRect dark_end = GRect(0, dark_y, window_width, dark_height);
  property_animation_dark = property_animation_create_layer_frame(dark_layer, &dark_start, &dark_end);
  animation_dark = property_animation_get_animation(property_animation_dark);
  animation_set_duration(animation_dark, 800);
  
  // Create spawn animation for both layers to slide at the same time
  layer_slide = animation_spawn_create(animation_light, animation_dark, NULL);
  animation_set_play_count(layer_slide, 1);
  animation_schedule(layer_slide);
}

static void main_window_load(Window *window) {
  // Get the root layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int16_t window_width = bounds.size.w;
  int16_t window_height = bounds.size.h;
  int16_t text_offset = 7;
  int16_t dark_height = 82;
  int16_t time_text_height = 46;
  int16_t date_text_height = 26;
  int16_t time_text_y = (window_height / 2) - (dark_height / 2) +
                        ((dark_height - time_text_height - date_text_height) / 2) - text_offset;
  int16_t date_text_y = time_text_y + time_text_height;
  
  // Create light layer
  light_layer = layer_create(GRectZero);
  if (persist_read_int(KEY_LIGHT_COLOR)) {
    light_color_hex = persist_read_int(KEY_LIGHT_COLOR);
  }
  else {
    light_color_hex = 5614335;
  }
  layer_set_update_proc(light_layer, draw_light_layer);
  layer_add_child(window_layer, light_layer);
  
  // Create dark layer
  dark_layer = layer_create(GRectZero);
  if (persist_read_int(KEY_DARK_COLOR)) {
    dark_color_hex = persist_read_int(KEY_DARK_COLOR);
  }
  else {
    dark_color_hex = 21930;
  }
  layer_set_update_proc(dark_layer, draw_dark_layer);
  layer_add_child(window_layer, dark_layer);
  
  // Create time TextLayer
  time_layer = text_layer_create(GRect(0, time_text_y, window_width, time_text_height));
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);

  // Apply to time TextLayer
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LATO_BOLD_46));
  text_layer_set_font(time_layer, time_font);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  // Create date TextLayer
  date_layer = text_layer_create(GRect(0, date_text_y, window_width, date_text_height));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorWhite);

  // Apply to date TextLayer
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  // Play animations
  animations(window_width, window_height);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  
  // Destroy colored layers
  layer_destroy(light_layer);
  layer_destroy(dark_layer);
  
  // Destroy animations
  property_animation_destroy(property_animation_dark);
  property_animation_destroy(property_animation_light);
  animation_destroy(animation_light);
  animation_destroy(animation_dark);
  animation_destroy(layer_slide);
}

static void init() {
  // Create main Window element and assign to pointer
  main_window = window_create();
  window_set_background_color(main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Make sure the date is displayed from the start
  update_date();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}