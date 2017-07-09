#include <pebble.h>

typedef struct {
  int pm10Value;
  int pm25Value;
  int pm10Grade;
  char *dataTime;
} PmValuesStructType;

static void prv_update_app_glance(AppGlanceReloadSession *session, size_t limit, void *context);
static void prv_get_pm();
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void prv_exit_delay();
static void prv_exit_application(void *data);
static void prv_init_app_message();
static void prv_init(void);
static void prv_deinit(void);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
const char * prv_convPMGrade2str(int pmGrade);
