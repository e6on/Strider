#include "data.h"

typedef enum {
  AppKeyCurrentAverage = 0,
  AppKeyDailyAverage,
  AppKeyCurrentSteps
} AppKey;

typedef enum {
  AverageTypeCurrent = 0,
  AverageTypeDaily
} AverageType;

static GBitmap *s_blue_shoe, *s_red_shoe, *s_green_shoe, *sbmpBleN,*sbmpBle,*sbmpBatteryCharging,*sbmpBattery;
static GFont s_font_small, s_font_big, s_font_med;

static int s_current_steps, s_daily_average, s_current_average;
static char s_current_steps_buffer[8];

static void update_average(AverageType type) {
  // Start time is midnight
  const time_t start = time_start_of_today();

  time_t end = start;
  int steps = 0;
  switch(type) {
    case AverageTypeDaily:
      // One whole day
      end = start + SECONDS_PER_DAY;
      break;
    case AverageTypeCurrent:
      // Time from midnight to now
      end = start + (time(NULL) - time_start_of_today());
      break;
    default:
      if(DEBUG) APP_LOG(APP_LOG_LEVEL_ERROR, "Unknown average type!");
      break;
  } 

  // Check the average data is available
  HealthServiceAccessibilityMask mask = health_service_metric_averaged_accessible(
                                HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available, read it
    steps = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, 
                                                                    HealthServiceTimeScopeDaily);
  } else {
    if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "No data available for daily average");
  }

  // Store the calculated value
  switch(type) {
    case AverageTypeDaily:
      s_daily_average = steps;
      persist_write_int(AppKeyDailyAverage, s_daily_average);

      if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Daily average: %d", s_daily_average);
      break;
    case AverageTypeCurrent:
      s_current_average = steps;
      persist_write_int(AppKeyCurrentAverage, s_current_average);

      if(DEBUG) APP_LOG(APP_LOG_LEVEL_DEBUG, "Current average: %d", s_current_average);
      break;
    default: break;  // Handled by previous switch
  }
}

void data_update_steps_buffer() {
  int thousands = s_current_steps / 1000;
  int hundreds = s_current_steps % 1000;
  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%d%03d", thousands, hundreds);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%d", hundreds);
  }

  main_window_redraw();
}

static void load_health_data_handler(void *context) {
  const struct tm *time_now = util_get_tm();

  s_current_steps = health_service_sum_today(HealthMetricStepCount);
  persist_write_int(AppKeyCurrentSteps, s_current_steps);

  update_average(AverageTypeDaily);
  update_average(AverageTypeCurrent);

  data_update_steps_buffer();
}

void data_reload_averages() {
  app_timer_register(LOAD_DATA_DELAY, load_health_data_handler, NULL);
}

void data_init() {
  // Load resources
  s_green_shoe = gbitmap_create_with_resource(RESOURCE_ID_GREEN_SHOE_LOGO);
  s_blue_shoe = gbitmap_create_with_resource(RESOURCE_ID_BLUE_SHOE_LOGO);
  s_red_shoe = gbitmap_create_with_resource(RESOURCE_ID_RED_SHOE_LOGO);
  sbmpBattery= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  sbmpBatteryCharging= gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY_CHARGER);
  sbmpBle=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLE);
  sbmpBleN=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLEN);

  s_font_small = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS); // steps text
//  s_font_med = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS); // date text
//  s_font_big = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS); // time text
//  s_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MET_BoldCaps_26)); // steps text
  s_font_med = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MET_BoldCaps_26)); // date text
  s_font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MET_BoldCaps_46)); // time text

  // First time persist
  if(!persist_exists(AppKeyCurrentSteps)) {
    s_current_steps = 0;
    s_current_average = 0;
    s_daily_average = 0;
  } else {
    s_current_average = persist_read_int(AppKeyCurrentAverage);
    s_daily_average = persist_read_int(AppKeyDailyAverage);
    s_current_steps = persist_read_int(AppKeyCurrentSteps);
  }
  data_update_steps_buffer();

  // Avoid half-second delay loading the app by delaying API read
  data_reload_averages();
}

void data_deinit() {
  gbitmap_destroy(s_green_shoe);
  gbitmap_destroy(s_blue_shoe);
  gbitmap_destroy(s_red_shoe);
}

int data_get_current_steps() {
  return s_current_steps;
}

int data_get_current_average() {
  return s_current_average;
}

int data_get_daily_average() {
  return s_daily_average;
}

void data_set_current_steps(int value) {
  s_current_steps = value;
}

void data_set_current_average(int value) {
  s_current_average = value;
}

void data_set_daily_average(int value) {
  s_daily_average = value;
}

GFont data_get_font(FontSize size) {
  switch(size) {
    case FontSizeSmall:  return s_font_small;
    case FontSizeMedium: return s_font_med;
    case FontSizeLarge:  return s_font_big;
    default: return s_font_small;
  }
}

GBitmap* data_get_blue_shoe() {
  return s_blue_shoe;
}

GBitmap* data_get_green_shoe() {
  return s_green_shoe;
}

GBitmap* data_get_red_shoe() {
  return s_red_shoe;
}

char* data_get_current_steps_buffer() {
  return s_current_steps_buffer;
}

/// Added to support battery and BLE icons
GBitmap* data_get_BLE(bool isConnected) {
    if (isConnected) return sbmpBle;
    else return sbmpBleN;
}

GBitmap* data_get_Battery(bool isCharging) {
    if (isCharging) return sbmpBatteryCharging;
    else return sbmpBattery;
}
