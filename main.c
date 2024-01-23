#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/sync.h"
/* !!! PART 2 & 3 !!! */

/* !!! MIGHT WANT TO CHANGE THIS !!! */
#define BUTTON_DEBOUNCE_DELAY 50
#define EVENT_QUEUE_LENGTH 10

const static uint led0 = 0;
const static uint led1 = 1;
const static uint led2 = 2;
const static uint led3 = 3;

const static uint B1 = 20;
const static uint B2 = 21;

queue_t Evt_Queue;

/* Function pointer primitive */
typedef void (*state_func_t)(void);

typedef struct _state_t
{
    uint8_t id;
    state_func_t Enter;
    state_func_t Do;
    state_func_t Exit;
    uint32_t delay_ms;
} state_t;

typedef enum _event_t
{
    b1_evt = 0,
    b2_evt = 1,
    /*b3_evt = 2*/
    no_evt = 2
} event_t;

/* !!! PART 2 & 3 !!! */
/* Define event queue */

unsigned long last_time = 0;

void button_isr(uint gpio, uint32_t events)
{
    /* !!! PART 2 !!! */

    if ((to_ms_since_boot(get_absolute_time()) - last_time) > BUTTON_DEBOUNCE_DELAY)
    {
        last_time = to_ms_since_boot(get_absolute_time());

        event_t evt;
        switch (gpio)
        {
        case B1:
            evt = b1_evt;
            queue_try_add(&Evt_Queue, &evt);
            break;

        case B2:
            evt = b2_evt;
            queue_try_add(&Evt_Queue, &evt);
            break;
        }
    }
}

void private_init()
{
    /* !!! PART 2 !!! */
    /* Event queue setup */
    queue_init(&Evt_Queue, sizeof(event_t), 10);

    /* !!! PART 2 !!! */
    /* Button setup */
    gpio_set_irq_enabled_with_callback(B1, GPIO_IRQ_EDGE_FALL, true, &button_isr);
    gpio_set_irq_enabled(B2, GPIO_IRQ_EDGE_FALL, true);

    /* !!! PART 1 !!! */
    /* LED setup */

    gpio_init(led0);
    gpio_init(led1);
    gpio_init(led2);
    gpio_init(led3);
    gpio_set_dir(led0, GPIO_OUT);
    gpio_set_dir(led1, GPIO_OUT);
    gpio_set_dir(led2, GPIO_OUT);
    gpio_set_dir(led3, GPIO_OUT);
}

/* The next three methods are for convenience, you might want to use them. */
event_t get_event(void)
{
    /* !!!! PART 2 !!!! */
    event_t evt;
    if (queue_try_remove(&Evt_Queue, &evt))
    {
        return evt;
    }
    return no_evt;
}

void leds_off()
{
    /* !!! PART 1 !!! */
    gpio_put(led0, 0);
    gpio_put(led1, 0);
    gpio_put(led2, 0);
    gpio_put(led3, 0);
}

void leds_on()
{
    /* !!! PART 2 !!! */
    gpio_put(led0, 1);
    gpio_put(led1, 1);
    gpio_put(led2, 1);
    gpio_put(led3, 1);
}

void do_state_0(void)
{
    /* !!! PART 1 !!! */
    static uint led_last = 0;
    gpio_put(led_last, 0);
    led_last = (led_last + 1) % 4;
    gpio_put(led_last, 1);
}

void do_state_1(void)
{
    leds_on();
}

void do_state_2(void)
{
    static uint led_last = 3;
    gpio_put(led_last, 0);
    led_last = led_last - 1;
    if (led_last == -1)
    {
        led_last = 3;
    }
    gpio_put(led_last, 1);
}

/* !!! PART 1 !!! */
const state_t state0 = {
    0,
    leds_off,
    do_state_0,
    leds_off,
    1000};

const state_t state1 = {
    1,
    leds_off,
    do_state_1,
    leds_off,
    100};

const state_t state2 = {
    2,
    leds_off,
    do_state_2,
    leds_off,
    100};

/* !!! PART 2 !!! */
const state_t state_table[3][3] = {
    // current-state  B1        B2      No-event
    {/*s0*/ state2, state1, state0},
    {/*s1*/ state0, state2, state1},
    {/*s2*/ state1, state0, state2}};

/* !!! ALL PARTS !!! */
int main()
{
    private_init();

    state_t current_state = state0;
    event_t evt = no_evt;
    // current_state.Enter();

    for (;;)
    {
        current_state.Enter();
        // evt = get_event();
        evt = no_evt;
        while (current_state.id == state_table[current_state.id][evt].id)
        {
            current_state.Do();
            sleep_ms(current_state.delay_ms);
            evt = get_event();
        }
        // current_state.Exit();
        current_state = state_table[current_state.id][evt];
    }
}
