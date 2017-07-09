#include <pebble.h>
#include "app_pm.h"

#define API_SERVICE_KEY "OP7ODnqejGwpT1mU%2BvxP9xYTt%2BspoqiYJBpEdO3CG6DgGnpVnBPOt%2F5PdlS7Rtu1ry9tFDWFxK3aPzsJ04FKEQ%3D%3D"

static Window *s_window;
static PmValuesStructType s_pm_values;
static TextLayer *s_txt_layer;

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

static void prv_init(void) {
  prv_init_app_message();

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, false);
}

static void prv_init_app_message() {
  // Initialize AppMessage
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //Create a TextLayer to show the result
  s_txt_layer = text_layer_create(GRect(0, (bounds.size.h/2)-50, bounds.size.w, 100));
  text_layer_set_background_color(s_txt_layer, GColorClear);
  text_layer_set_text_color(s_txt_layer, GColorBlack);
  text_layer_set_font(s_txt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_txt_layer, GTextAlignmentCenter);
  //text_layer_set_text(s_txt_layer, "Initializing");
  layer_add_child(window_layer, text_layer_get_layer(s_txt_layer));
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_APP_READY);
  if (ready_tuple) {
    if(launch_reason() == APP_LAUNCH_USER || launch_reason() == APP_LAUNCH_QUICK_LAUNCH) {
      // get pm values
      prv_get_pm();
    } else {
      // Application was just installed, or configured
      text_layer_set_text(s_txt_layer, "App Installed");
    }
    return;
  }

  Tuple *pm10Value_tuple = dict_find(iter, MESSAGE_KEY_PM10_VALUE);
  Tuple *pm25Value_tuple = dict_find(iter, MESSAGE_KEY_PM25_VALUE);
  Tuple *pm10Grade_tuple = dict_find(iter, MESSAGE_KEY_PM10_GRADE);
  Tuple *dataTime_tuple = dict_find(iter, MESSAGE_KEY_DATA_TIME);
  if (pm10Value_tuple) {
    s_pm_values.pm10Value = (int)pm10Value_tuple->value->int32;
    s_pm_values.pm25Value = (int)pm25Value_tuple->value->int32;
    s_pm_values.pm10Grade = (int)pm10Grade_tuple->value->int32;
    s_pm_values.dataTime  = (char *)dataTime_tuple->value;
    APP_LOG(APP_LOG_LEVEL_INFO, "pm10Value: %d", s_pm_values.pm10Value);
    APP_LOG(APP_LOG_LEVEL_INFO, "pm10Grade: %d", s_pm_values.pm10Grade);
    // Display the current lock state
    static char str[100];
    snprintf(str, sizeof(str), "미세먼지: %d(%s)\n초미세먼지: %d\n서초구\n%s", 
             (int)pm10Value_tuple->value->int32, 
             prv_convPMGrade2str((int)pm10Grade_tuple->value->int32),
             (int)pm25Value_tuple->value->int32,
             (char *)dataTime_tuple->value);
    text_layer_set_text(s_txt_layer, str);
  }
}

static void prv_window_unload(Window *window) {
  window_destroy(s_window);
}

static void prv_deinit(void) {
  // Before the application terminates, setup the AppGlance
  app_glance_reload(prv_update_app_glance, &s_pm_values);
}

// Create the AppGlance displayed in the system launcher
static void prv_update_app_glance(AppGlanceReloadSession *session, size_t limit, void *context) {
  // Check we haven't exceeded system limit of AppGlance's
  if (limit < 1) return;

  // Retrieve the current state from context
  PmValuesStructType *pm_values = context;

  char str[50];
  snprintf(str, sizeof(str), "%s pm10:%d pm25:%d", 
           prv_convPMGrade2str((int)pm_values->pm10Grade),
           (int)pm_values->pm10Value, (int)pm_values->pm25Value);

  // Create the AppGlanceSlice (no icon, no expiry)
  AppGlanceSlice entry = (AppGlanceSlice) {
    .layout = {
      //.icon = PUBLISHED_ID_PM,
      .subtitle_template_string = str
    },
    .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
  };

  // Add the slice, and check the result
  const AppGlanceResult result = app_glance_add_slice(session, entry);
  if (result != APP_GLANCE_RESULT_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
  }
}

static void prv_get_pm() {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if (result != APP_MSG_OK) {
    text_layer_set_text(s_txt_layer, "Outbox Failed");
  }

  dict_write_cstring(out, MESSAGE_KEY_API_SERVICE_KEY, API_SERVICE_KEY);

  result = app_message_outbox_send();
  if (result != APP_MSG_OK) {
    text_layer_set_text(s_txt_layer, "Message Failed");
  }
}

const char *prv_convPMGrade2str(int pmGrade) {
  const char *PMGradeStr[5] = {
    "", "좋음", "보통", "나쁨", "매우나쁨"
  };
  if(pmGrade > 4)
    pmGrade = 0;
  if(pmGrade < 0)
    pmGrade = 0;
  return PMGradeStr[pmGrade];
}
