#include <pebble.h>

#include "modules/data.h"
#include "modules/health.h"
#include "modules/util.h"

#include "windows/main_window.h"

/// Vibration pattern connection lost
static const uint32_t const patternLost[] = { 200, 200, 200,200,200 };

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  main_window_update_time(tick_time);
  data_reload_averages();
  main_window_redraw();
}

void hdlBle(bool state){
  APP_LOG(APP_LOG_LEVEL_DEBUG, "BLE status change (%d)", state);
    main_window_update_ble(state);
    if(!state){
       VibePattern pat = {
           .durations = patternLost,
           .num_segments = ARRAY_LENGTH(patternLost),
       };
       vibes_enqueue_custom_pattern(pat);
    }
}

void hdlBattery(BatteryChargeState battery){
    main_window_update_battery(battery.is_charging, battery.charge_percent);
}

void init() {
  data_init();
  health_init();
  main_window_push();
  connection_service_subscribe((ConnectionHandlers) {
  .pebble_app_connection_handler = hdlBle,
  .pebblekit_connection_handler = hdlBle
});
  battery_state_service_subscribe (&hdlBattery);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  hdlBattery(battery_state_service_peek());
  main_window_update_time(util_get_tm());
}

void deinit() { }

int main() {
  init();
  app_event_loop();
  deinit();
}
