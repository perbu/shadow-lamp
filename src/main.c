#include "mgos.h"
#include "mgos_ro_vars.h"
#include "mgos_app.h"
#include "mgos_system.h"
#include "mgos_timers.h"
#include "mgos_shadow.h"

// JSON parser.
#include <frozen.h>

int state = 0;

static void set_lamp_state(int newstate)
{
    // Set GPIO pin.
    mgos_gpio_write(mgos_sys_config_get_pins_lampled(), newstate);
}

static void js_token_cb(void *callback_data,
                        const char *name, size_t name_len,
                        const char *path, const struct json_token *token)
{
    int event_type = *(int *)callback_data;

    if (token->type == JSON_TYPE_NUMBER) // disregard anything that isn't a number. Change this!
    {
        LOG(LL_INFO, ("Path: '%s'", path));
        int newstate = 666;
        // If you need more complex parsing you might wanna sscanf here and figure out where you are.
        // From ukukhanya: int ret = sscanf(path, ".leds.%i.%c", &pixelid, &color);

        if (strcmp(path, ".lamp") == 0)
        {
            newstate = atoi(token->ptr);
            LOG(LL_INFO, ("Setting lamp state to %d", newstate));
            set_lamp_state(newstate);
        }
    }
    (void)event_type;
}

static void delta_cb(int ev, void *ev_data, void *userdata)
{
    int event_type = 0;
    struct mg_str *delta = (struct mg_str *)ev_data;
    char log_bfr[255];

    // Make a proper c string out of the pointer so we can log it.
    strncpy(log_bfr, delta->p, (int)delta->len > 255 ? 255 : (int)delta->len);

    LOG(LL_INFO, ("Delta callback: len: %d delta: '%s'", (int)delta->len, log_bfr));
    int ret = json_walk(delta->p, (int)delta->len, js_token_cb, &event_type);
    LOG(LL_INFO, ("Scanned shadow delta and found %d entries", ret));

    (void)event_type;
    (void)ev;
    (void)userdata;
}

static void setup_shadow()
{
    mgos_event_add_handler(MGOS_SHADOW_UPDATE_DELTA, delta_cb, NULL);
}

enum mgos_app_init_result mgos_app_init(void)
{
    setup_shadow();
    LOG(LL_INFO, ("Lamp on GPIO pin %d",
                  mgos_sys_config_get_pins_lampled()));
    mgos_gpio_set_mode(mgos_sys_config_get_pins_lampled(),
                       MGOS_GPIO_MODE_OUTPUT);

    return MGOS_APP_INIT_SUCCESS;
}
