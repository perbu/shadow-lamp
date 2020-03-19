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
    if (token->type == JSON_TYPE_NUMBER) // disregard anything that isn't a number.
    {
        int newstate = 0;
        // this might accept some whitespace here and there. Thanks Unix.
        int ret = sscanf(path, ".lamp.%n", &newstate);
        if (ret == 1 && (newstate == 0 || newstate == 1))
        {
            set_lamp_state(newstate);
            LOG(LL_INFO, ("Setting lamp state to %d", newstate));
        }
        else
        {
            LOG(LL_ERROR, ("Invalid state given %d", newstate));
        }
    }
    (void)event_type;
}

static void delta_cb(int ev, void *ev_data, void *userdata)
{
    int event_type = 0;
    struct mg_str *delta = (struct mg_str *)ev_data;
    LOG(LL_INFO, ("Delta callback: len: %d delta: '%s'", (int)delta->len, delta->p));
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
    LOG(LL_INFO, ("Init done"));
    return MGOS_APP_INIT_SUCCESS;
}
