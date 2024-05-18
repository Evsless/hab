#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/uaccess.h>


#define HAB_LED_DRIVER_NAME "hab_led"
#define HAB_LED_CLASS_NAME  "hab_led_class"
#define HAB_LED_NO_GPIO16   587
#define HAB_LED_ON_TIME     200

#define FIRST_TOGGLE_DELAY  10u
#define TOGGLE_PRESCALER    2u

#define HAB_INIT_OK         4u
#define HAB_INIT_NOT_OK     6u

static int __init hab_led_init(void);
static void __exit hab_led_exit(void);

static int hab_led_open(struct inode *inode, struct file *file);
static int hab_led_release(struct inode *inode, struct file *file);
static ssize_t hab_led_read(struct file *filp, char __user *buff, size_t len, loff_t *off);
static ssize_t hab_led_write(struct file *filp, const char *buff, size_t len, loff_t *off);

static u8 led_state;
static u8 toggle_counter;

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = hab_led_open,
    .release = hab_led_release,
    .read = hab_led_read,
    .write = hab_led_write,
};

dev_t dev = 0;
static struct class *dev_class;
static struct cdev hab_led_cdev;

static struct timer_list hab_led_tim;

static void hab_led_tim_cb(struct timer_list *data) {
    led_state = !led_state;
    gpio_set_value(HAB_LED_NO_GPIO16, led_state);

    toggle_counter--;
    if (toggle_counter > 0)
        mod_timer(&hab_led_tim, jiffies + msecs_to_jiffies(HAB_LED_ON_TIME));
}

static int __init hab_led_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, HAB_LED_DRIVER_NAME) < 0) {
        pr_err("ERROR: Error allocating major number for hab_led module.\n");
        goto dev_unreg;
    }
    pr_info("INFO: Allocated hab_led module - %d %d.\n", MAJOR(dev), MINOR(dev));

    cdev_init(&hab_led_cdev, &fops);
    if (cdev_add(&hab_led_cdev, dev, 1) < 0) {
        pr_err("ERROR: Error adding character device to the system.\n");
        goto dev_del;
    }

    if ((dev_class = class_create(HAB_LED_CLASS_NAME)) == NULL) {
        pr_err("ERROR: Error creading class for device.\n");
        goto dev_del_class;
    }

    if (device_create(dev_class, NULL, dev, NULL, HAB_LED_DRIVER_NAME) == NULL) {
        pr_err("ERROR: Error when creating the hab_led device.\n");
        goto dev_remove;
    }

    if (gpio_request(HAB_LED_NO_GPIO16, "GPIO16") < 0) {
        pr_err("ERROR: Error allocating gpio pin.\n");
        goto dev_remove_gpio;
    }
    gpio_direction_output(HAB_LED_NO_GPIO16, 0);

    timer_setup(&hab_led_tim, hab_led_tim_cb, 0);

    return 0;

dev_remove_gpio:
    gpio_free(HAB_LED_NO_GPIO16);

dev_remove:
    device_destroy(dev_class, dev);

dev_del_class:
    class_destroy(dev_class);

dev_del:
    cdev_del(&hab_led_cdev);

dev_unreg:
    unregister_chrdev_region(dev, 1);

    return 0;
}


static void __exit hab_led_exit(void) {
    gpio_set_value(HAB_LED_NO_GPIO16, 0);
    gpio_free(HAB_LED_NO_GPIO16);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&hab_led_cdev);
    unregister_chrdev_region(dev, 1);
}


static ssize_t hab_led_read(struct file *filp, char __user *buff, size_t len, loff_t *off) {
    int to_copy, not_copied;
    char value[3] = " \n";

    to_copy = min(len, sizeof(value));

    value[0] = gpio_get_value(HAB_LED_NO_GPIO16) + '0';

    not_copied = copy_to_user(buff, &value, to_copy);
    if (not_copied > 0)
        pr_err("ERROR: Error copying data to user space. Bytes not copied: %d.\n", not_copied);

    return (to_copy - not_copied);
}

static ssize_t hab_led_write(struct file *filp, const char *buff, size_t len, loff_t *off) {
    int to_copy, not_copied;
    u8 rec_buff[8] = {0};

    to_copy = min(len, sizeof(rec_buff));

    not_copied = copy_from_user(rec_buff, buff, len);
    if (not_copied > 0)
        pr_err("ERROR: Error copying data from user space. Bytes not copied: %d.\n", not_copied);
    
    if ('0' == rec_buff[0]) {
        toggle_counter = HAB_INIT_OK * TOGGLE_PRESCALER;
        mod_timer(&hab_led_tim, jiffies + msecs_to_jiffies(FIRST_TOGGLE_DELAY));
    } else if ('1' == rec_buff[0]) {
        toggle_counter = HAB_INIT_NOT_OK * TOGGLE_PRESCALER;
        mod_timer(&hab_led_tim, jiffies + msecs_to_jiffies(FIRST_TOGGLE_DELAY));
        
    } else {
        pr_err("ERROR: Unknown hab_led command detected.\n");
    }

    return (to_copy - not_copied);
}

static int hab_led_open(struct inode *inode, struct file *file) {
    return 0;
}

static int hab_led_release(struct inode *inode, struct file *file) {
    return 0;
}


module_init(hab_led_init);
module_exit(hab_led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("HAB LED driver for signalizing device state.");
