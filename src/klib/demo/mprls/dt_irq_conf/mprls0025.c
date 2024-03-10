#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

/* MODULE INFO */
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mprls025 sensor device driver instantiated from DT.");

static int irq_num;

/* Global variables  */
static struct i2c_client *mprls_i2c_client = NULL;
static struct proc_dir_entry *procfil;

/* Global function prototypes */
static int mprls_probe(struct i2c_client *client, const struct i2c_device_id *id);
static void mprls_remove(struct i2c_client *client);
static ssize_t mprls_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);

/* CREATE A DEVICE ID */
static const struct of_device_id mprls_matches[] = {
    { .compatible = "honeywell,mprls0025pa" },
    { }
};
MODULE_DEVICE_TABLE(of, mprls_matches);

static const struct i2c_device_id mprls_ids[] = {
    {"mprls0025"},
    { }
};
MODULE_DEVICE_TABLE(i2c, mprls_ids);

/* CREATE A DEVICE DRIVER STRUCT */
static struct i2c_driver mprls_driver = {
    .probe = mprls_probe,
    .remove = mprls_remove,
    .id_table = mprls_ids,
    .driver = {
        .name = "mprls0025",
        .of_match_table = mprls_matches
    }
};

static struct proc_ops fops = {
    .proc_read = mprls_read
    // .proc_write = mprls_write
};

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs) {
	printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
	return (irq_handler_t) IRQ_HANDLED; 
}

/* ------------- MPRLS UTILITIES ------------- */
static u8 mprls_busy(void) {
    int err = 0;
    char buff[3];

    err = i2c_master_recv(mprls_i2c_client, buff, 1);

    return buff[0] & 0x20;
}

static u32 mprls_read_raw(void) {
    int err = 0;
    u32 retval = 0;
    char buff[3] = {0xAA, 0x00, 0x00};

    err = i2c_master_send(mprls_i2c_client, buff, 3);

    while (mprls_busy())
        asm("nop");

    err = i2c_master_recv(mprls_i2c_client, buff, 3);
    retval = (buff[1] << 16) | (buff[2] << 8) | buff[3];

    return retval;
}


/* ------------- R/W FUNCTIONS ------------- */
static ssize_t mprls_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
    u32 mprls_data = 0;
    int to_copy, not_copied = 0;
    char cp_buff[11];

    to_copy = min(count, sizeof(cp_buff));
    mprls_data = mprls_read_raw();
    snprintf(cp_buff, sizeof(cp_buff), "%d\n", mprls_data);

    not_copied = copy_to_user(buff, cp_buff, to_copy);
    return to_copy - not_copied;
}

// static ssize_t mprls_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp) {
//     return 0;a
// }



/* -------- INIT AND EXIT FUNTIONS -------- */
static int mprls_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    irq_num = client->irq;
    if(request_irq(irq_num, (irq_handler_t) gpio_irq_handler, IRQF_TRIGGER_RISING, "my_gpio_irq", NULL) != 0){
		printk("Error!\nCan not request interrupt nr.: %d\n", irq_num);
		return -1;
	}

    if(client->addr != 0x18) {
        printk(KERN_ERR "mprls: wrong device address.\n");
        return -1;
    }
    mprls_i2c_client = client;

    procfil = proc_create("mprls0025", 0666, NULL, &fops);
    if(NULL == procfil) {
        printk(KERN_ERR "mprls: error while creating a procfs entry.\n");
        return -ENOMEM;
    }
    
    printk(KERN_INFO "mprls: process loaded.\n");
    return 0;
}

static void mprls_remove(struct i2c_client *client) {
    free_irq(irq_num, NULL);
    proc_remove(procfil);
    printk(KERN_INFO "mprls: process removed.\n");
}

module_i2c_driver(mprls_driver);