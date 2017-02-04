#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kthread.h>

/* Polling frequency in hz. Max 1000000 */
#define POLLING_FREQ_HZ 5000

#define MODULE_NAME "Polling GPIO keyboard driver"
#define MAX_SLEEP (1000000 / (POLLING_FREQ_HZ - POLLING_FREQ_HZ * 0.1))
#define MIN_SLEEP (1000000 / (POLLING_FREQ_HZ + POLLING_FREQ_HZ * 0.1))

static struct input_dev *device = NULL;
static struct task_struct *thread = NULL;

static struct gpio_map {
    const char * const name;
    const int gpio;
    const int key;
    char state;
    char registered;
} gpio_map[] = { { "A", 5, KEY_A, 0, 0 },
                 { "B", 6, KEY_B, 0, 0 },
                 { "C", 13, KEY_C, 0, 0 },
                 { "D", 19, KEY_D, 0, 0 }
               };

#define GPIO_MAP_SIZE (sizeof(gpio_map) / sizeof(gpio_map))

static int thread_fn(void *data)
{
    while (kthread_should_stop() == 0) {
        int i;

        for (i = 0; i < GPIO_MAP_SIZE; i++) {
            const int state = gpio_get_value(gpio_map[i].gpio);

            if (state != gpio_map[i].state) {
                gpio_map[i].state = state;

                input_report_key(device, gpio_map[i].key, !gpio_map[i].state);
                input_sync(device);
            }
        }

        usleep_range(MIN_SLEEP, MAX_SLEEP);
    }

    return 0;
}

int init_module(void)
{
    int i;

    device = input_allocate_device();

    if (device == NULL) {
        printk(KERN_INFO "input_allocate_device() failed.\n");
        return -ENOMEM;
    }

    /* We will only handle keys */
    set_bit(EV_KEY, device->evbit);

    /* Enable auto repeat */
    set_bit(EV_REP, device->evbit);

    /* Set some keys even if we do not use them. This is necessary
     * for udev to detect a keyboard. */
    set_bit(KEY_ESC, device->keybit);
    set_bit(KEY_1, device->keybit);
    set_bit(KEY_2, device->keybit);
    set_bit(KEY_3, device->keybit);
    set_bit(KEY_4, device->keybit);
    set_bit(KEY_5, device->keybit);
    set_bit(KEY_6, device->keybit);
    set_bit(KEY_7, device->keybit);
    set_bit(KEY_8, device->keybit);
    set_bit(KEY_9, device->keybit);
    set_bit(KEY_0, device->keybit);
    set_bit(KEY_MINUS, device->keybit);
    set_bit(KEY_EQUAL, device->keybit);
    set_bit(KEY_BACKSPACE, device->keybit);
    set_bit(KEY_TAB, device->keybit);
    set_bit(KEY_Q, device->keybit);
    set_bit(KEY_W, device->keybit);
    set_bit(KEY_E, device->keybit);
    set_bit(KEY_R, device->keybit);
    set_bit(KEY_T, device->keybit);
    set_bit(KEY_Y, device->keybit);
    set_bit(KEY_U, device->keybit);
    set_bit(KEY_I, device->keybit);
    set_bit(KEY_O, device->keybit);
    set_bit(KEY_P, device->keybit);
    set_bit(KEY_LEFTBRACE, device->keybit);
    set_bit(KEY_RIGHTBRACE, device->keybit);
    set_bit(KEY_ENTER, device->keybit);
    set_bit(KEY_LEFTCTRL, device->keybit);
    set_bit(KEY_A, device->keybit);
    set_bit(KEY_S, device->keybit);
    set_bit(KEY_D, device->keybit);

    for (i = 0; i < GPIO_MAP_SIZE; i++) {
        if (gpio_request(gpio_map[i].gpio, gpio_map[i].name) != 0) {
            printk(KERN_INFO "gpio_request(%d, \"%s\") failed.\n", gpio_map[i].gpio, gpio_map[i].name);
            input_free_device(device);
            return -ENODEV;
        }

        if (gpio_direction_input(gpio_map[i].gpio) != 0) {
            printk(KERN_INFO "gpio_direction_input(%d) failed.\n", gpio_map[i].gpio);
            input_free_device(device);
            return -ENODEV;
        }

        set_bit(gpio_map[i].key, device->keybit);

        gpio_map[i].state = 1;
        gpio_map[i].registered = 1;
    }

    device->name = MODULE_NAME;
    device->phys = "pgpiokb/input0";
    device->id.bustype = BUS_VIRTUAL;

    if ((i = input_register_device(device)) < 0) {
        printk(KERN_INFO "input_register_device() failed.\n");
        cleanup_module();
        return i;
    }

    thread = kthread_create(thread_fn, &gpio_map, MODULE_NAME "_0");

    if (IS_ERR_OR_NULL(thread)) {
        printk(KERN_INFO "kthread_create() failed.\n");
        cleanup_module();
        return -PTR_ERR(thread);
    }

    wake_up_process(thread);

    printk(KERN_INFO MODULE_NAME " initialized.\n");

    return 0;
}

void cleanup_module(void)
{
    int i;

    if (!IS_ERR_OR_NULL(thread)) {
        kthread_stop(thread);
    }

    if (device != NULL) {
        input_unregister_device(device);
        input_free_device(device);
    }

    for (i = 0; i < GPIO_MAP_SIZE; i++) {
        if (gpio_map[i].registered) {
            gpio_free(gpio_map[i].gpio);
            gpio_map[i].registered = 0;
        }
    }

    printk(KERN_INFO MODULE_NAME " exiting.\n");
}

MODULE_AUTHOR("Steffen Schröter <steffen@vexar.de>");
MODULE_DESCRIPTION(MODULE_NAME);
MODULE_LICENSE("GPL v2");
