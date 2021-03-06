#include "main_window.h"

static Window *s_window;
static Layer *s_canvas_layer, *s_text_layer;

static char s_current_time_buffer[8];
static char s_current_date_buffer[11];

static const char *const s_day_names[7] =
{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static const char *const s_month_names[12] =
{"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static bool bleConnected;
static bool batteryCharging;
static int batteryLevel;

static void progress_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const int fill_thickness = PBL_IF_RECT_ELSE(12, (180 - grect_inset(bounds, GEdgeInsets(12)).size.h) / 2);
  int current_steps = data_get_current_steps();
  int daily_average = data_get_daily_average();
  int current_average = data_get_current_average();

  // Set new exceeded daily average
//  if(current_steps > daily_average) {
//    daily_average = current_steps;
//    data_set_daily_average(daily_average);
//  }

  // Decide color scheme based on progress to/past goal
  GColor scheme_color;
  GBitmap *bitmap;
  
  // Decide color scheme based on progress to/past goal
  // Perform drawing
  graphics_draw_outer_dots(ctx, bounds);
  /// Above daily average go JaegerGreen
   if(current_steps >= daily_average) {
        scheme_color = GColorJaegerGreen;
        bitmap = data_get_green_shoe();
        graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, scheme_color);
        /// 10x above daily average go BrightGreen
        if(current_steps > 10*daily_average) {
          scheme_color = GColorBrightGreen;
          bitmap = data_get_brgreen_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorJaegerGreen);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*10), fill_thickness, bounds, scheme_color);
        }
        /// 9x above daily average go JaegerGreen
        else if(current_steps > 9*daily_average) {
          scheme_color = GColorJaegerGreen;
          bitmap = data_get_green_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorPictonBlue);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*9), fill_thickness, bounds, scheme_color);
        }
        /// 8x above daily average go PictonBlue
        else if(current_steps > 8*daily_average) {
          scheme_color = GColorPictonBlue;
          bitmap = data_get_blue_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorBrightGreen);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*8), fill_thickness, bounds, scheme_color);
        }
        /// 7x above daily average go BrightGreen
        else if(current_steps > 7*daily_average) {
          scheme_color = GColorBrightGreen;
          bitmap = data_get_brgreen_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorJaegerGreen);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*7), fill_thickness, bounds, scheme_color);
        }
        /// 6x above daily average go JaegerGreen
        else if(current_steps > 6*daily_average) {
          scheme_color = GColorJaegerGreen;
          bitmap = data_get_green_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorPictonBlue);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*6), fill_thickness, bounds, scheme_color);
        }
        /// 5x above daily average go PictonBlue
        else if(current_steps > 5*daily_average) {
          scheme_color = GColorPictonBlue;
          bitmap = data_get_blue_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorBrightGreen);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*5), fill_thickness, bounds, scheme_color);
        }
        /// 4x above daily average go BrightGreen
        else if(current_steps > 4*daily_average) {
          scheme_color = GColorBrightGreen;
          bitmap = data_get_brgreen_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorJaegerGreen);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*4), fill_thickness, bounds, scheme_color);
        }
        /// 3x above daily average go JaegerGreen
        else if(current_steps > 3*daily_average) {
          scheme_color = GColorJaegerGreen;
          bitmap = data_get_green_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorPictonBlue);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*3), fill_thickness, bounds, scheme_color);
        }
        /// 2x above daily average go PictonBlue
        else if(current_steps > 2*daily_average) {
          scheme_color = GColorPictonBlue;
          bitmap = data_get_blue_shoe();
          graphics_fill_outer_ring(ctx, daily_average, fill_thickness, bounds, GColorBrightGreen);
          graphics_fill_outer_ring(ctx, (current_steps-daily_average*2), fill_thickness, bounds, scheme_color);
        }
        /// 1x above daily average go BrightGreen
        else if(current_steps > daily_average) {
          scheme_color = GColorBrightGreen;
          bitmap = data_get_brgreen_shoe();
          graphics_fill_outer_ring(ctx, (current_steps-daily_average), fill_thickness, bounds, scheme_color);
        }
   }
   /// above average go blue
   else if(current_steps >= current_average) {
     scheme_color = GColorPictonBlue;
     bitmap = data_get_blue_shoe();
     graphics_fill_outer_ring(ctx, current_steps, fill_thickness, bounds, scheme_color);
   }
   /// below average draw in red
   else {
     scheme_color = GColorRed;
     bitmap = data_get_red_shoe();
     graphics_fill_outer_ring(ctx, current_steps, fill_thickness, bounds, scheme_color);
   }
  
  // Perform drawing
//  graphics_draw_outer_dots(ctx, bounds);
//  graphics_fill_outer_ring(ctx, current_steps, fill_thickness, bounds, scheme_color);
  graphics_fill_goal_line(ctx, daily_average, fill_thickness, 4, bounds, GColorYellow);
  graphics_draw_steps_value(ctx, bounds, scheme_color, bitmap);
  GPoint pt;
  pt.x=PBL_IF_RECT_ELSE(20, 50);
  pt.y=PBL_IF_RECT_ELSE(25, 30);
  graphics_draw_status_icons(ctx, pt,data_get_Battery(batteryCharging),data_get_BLE(connection_service_peek_pebble_app_connection()),batteryLevel);
  pt.x=PBL_IF_RECT_ELSE(20, 50);
  pt.y=PBL_IF_RECT_ELSE(25, 30);
}

static void text_update_proc(Layer *layer, GContext *ctx) {
  const GRect layer_bounds = layer_get_bounds(layer);

  const GFont font_med = data_get_font(FontSizeMedium);
  const GFont font_large = data_get_font(FontSizeLarge);

  // Get total width
  int total_width = 0;
  GSize time_size = graphics_text_layout_get_content_size(
    s_current_time_buffer, font_large, layer_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft);
  total_width += time_size.w;
  if(!clock_is_24h_style()) {
    total_width += graphics_text_layout_get_content_size(
      "AM", font_med, layer_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft).w;
  }

  const int x_margin = 0; //(layer_bounds.size.w - total_width) / 2;
  const int xx_margin = (layer_bounds.size.w - total_width) / 2;
  const int y_margin = PBL_IF_RECT_ELSE(8, 2);
  const GRect time_rect = grect_inset(layer_bounds, GEdgeInsets(-y_margin, 0, 0, x_margin));
   /// Added to support date - kuup asukoht
  const GRect date_rect = grect_inset(layer_bounds, GEdgeInsets(y_margin+PBL_IF_RECT_ELSE(35, 45), 0, 0, x_margin+3));
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_text(ctx, s_current_time_buffer, font_large, time_rect, 
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
   /// Added to support date
  graphics_context_set_text_color(ctx, GColorYellow);
  graphics_draw_text(ctx, s_current_date_buffer, font_med, date_rect,
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  if(!clock_is_24h_style()) {
    // 12 hour mode
    const struct tm *time_now = util_get_tm();
    const bool am = time_now->tm_hour < 12;
    const int spacing = 15;

    const GRect period_rect = grect_inset(layer_bounds, 
      GEdgeInsets(PBL_IF_RECT_ELSE(-2, 4), 0, 0, time_size.w + xx_margin + spacing));
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, am ? "AM" : "PM", font_med, period_rect, 
                       GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  }
}

/*********************************** Window ***********************************/

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, progress_update_proc);
  layer_add_child(window_layer, s_canvas_layer);
// siin defineeritakse kella asukoht ekraanil
  GEdgeInsets time_insets = GEdgeInsets(70, 0, 0, 0);
  s_text_layer = layer_create(grect_inset(window_bounds, time_insets));
  layer_set_update_proc(s_text_layer, text_update_proc);
  layer_add_child(window_layer, s_text_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
  layer_destroy(s_text_layer);

  window_destroy(s_window);
}

void main_window_push() {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  graphics_set_window(s_window);
}

void main_window_update_time(struct tm* tick_time) {
  strftime(s_current_time_buffer, sizeof(s_current_time_buffer),
    clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
/// Added to support date 
 snprintf(s_current_date_buffer, sizeof(s_current_date_buffer), "%s %d %s",
    s_day_names[tick_time->tm_wday], tick_time->tm_mday,s_month_names[ tick_time->tm_mon ]);
  layer_mark_dirty(s_text_layer);
}

void main_window_redraw() {
  if(s_canvas_layer && s_text_layer) {
    layer_mark_dirty(s_canvas_layer);
    layer_mark_dirty(s_text_layer);
  }
}

void main_window_update_ble(bool isConnected){
    bleConnected=isConnected;
    layer_mark_dirty(s_canvas_layer);
}

void main_window_update_battery(bool isCharging,int charge){
    batteryCharging=isCharging;
    batteryLevel=charge;
    layer_mark_dirty(s_canvas_layer);
}
