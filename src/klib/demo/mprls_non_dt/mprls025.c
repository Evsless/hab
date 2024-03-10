#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>

/* MODULE INFO */
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("mprls025 sensor device driver");

typedef unsigned char uint8;
static char buffer[255];


/* ------- DEVICE DRIVER REGISTRATION ------- */
#define MINOR_NUM 0


/* ------- I2C RELATED DATA ------- */
#define DRIVER_NAME  "mprls025"
#define DRIVER_CLASS "mprls025_cl"

/* DEFINES FOR I2C SPECIFICATION */
#define I2C_BUS_0         0
#define SLAVE_DEVICE_NAME "MPRLS_0_25"
#define SLAVE_DEVICE_ID   0
#define SLAVE_ADDRESS     0x18

/* I2C GLOBAL VARIABLES */
static struct i2c_adapter *mprls_i2c_adapter = NULL;
static struct i2c_client *mprls_i2c_client = NULL;

/* PROVIDE THE BOARD INFO */
static struct i2c_board_info mprls_i2c_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SLAVE_ADDRESS)
};

/* CREATE A DEVICE ID */
static const struct i2c_device_id mprls_device_id[] = {
    {SLAVE_DEVICE_NAME, SLAVE_DEVICE_ID},
    { }
};
MODULE_DEVICE_TABLE(i2c, mprls_device_id);

/* CREATE A DEVICE DRIVER STRUCT */
static struct i2c_driver mprls_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
};

/* -------- HW RELATED FUNCTIONS -------- */
static u8 mprls_busy(void) {
    int err = 0;

    err = i2c_master_recv(mprls_i2c_client, buffer, 1);

    return buffer[0] & 0x20;
}

static u32 mprls_read_humid(void) {
    int err = 0;
    u32 retval = 0;
    char write_cmd[3] = {0xAA, 0x00, 0x00};

    if (!i2c_trylock_bus(mprls_i2c_adapter, I2C_LOCK_SEGMENT))
        i2c_lock_bus(mprls_i2c_adapter, I2C_LOCK_SEGMENT);
    i2c_unlock_bus(mprls_i2c_adapter, I2C_LOCK_SEGMENT);

    err = i2c_master_send(mprls_i2c_client, write_cmd, 3);

    while (mprls_busy())
        asm("nop");

    err = i2c_master_recv(mprls_i2c_client, buffer, 3);

    retval = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];

    return retval;
}

/* -------- DEVICE REGISTRATION RELATED DATA -------- */
static dev_t  devno;
static struct class *devclass;
static struct cdev device;


/* ------------- FILE OPERATIONS ------------- */
static int driver_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "mprls: device file opened.\n");
    return 0;
}

static int driver_close(struct inode *, struct file *filp) {
    printk(KERN_INFO "mprls: device file closed.\n");
    return 0;
}

static ssize_t driver_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
    int to_copy = 0, not_copied = 0, delta = 0;
    u32 humidity = 0;
    char out_str[24];

    to_copy = min(count, sizeof(out_str));
    
    humidity = mprls_read_humid();
    snprintf(out_str, sizeof(out_str), "%d\n", humidity);

    not_copied = copy_to_user(buff, out_str, to_copy);
    delta = to_copy - not_copied;

    return delta;
}


/* Map the file operations */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read
};

/* -------- INIT AND EXIT FUNTIONS -------- */
static int __init mprls_init(void) {
    int err = 0, retval = 0;

    err = alloc_chrdev_region(&devno, MINOR_NUM, 1, DRIVER_NAME);
    if (err < 0) {
        printk(KERN_ERR "mprls: could not register a device number.\n");
    }
    printk(KERN_INFO "mprls: registered device %d, %d.\n", MAJOR(devno), MINOR(devno));

    devclass = class_create(THIS_MODULE, DRIVER_CLASS);
    if (IS_ERR(devclass)) {
        printk(KERN_ERR "mprls: could not create a class for device.\n");
        goto class_err;
    }

    if (IS_ERR(device_create(devclass, NULL, devno, NULL, DRIVER_NAME))) {
        printk(KERN_ERR "mprls: could not add a device.\n");
        goto device_err;
    }

    cdev_init(&device, &fops);
    device.owner = THIS_MODULE;

    err = cdev_add(&device, devno, 1);
    if (err < 0) {
        printk(KERN_ERR "mprls: could not add a device %s, error %d.\n", DRIVER_NAME, err);
        goto ker_err;
    }

    mprls_i2c_adapter = i2c_get_adapter(I2C_BUS_0);
    mprls_i2c_client  = i2c_new_client_device(mprls_i2c_adapter, &mprls_i2c_board_info);

    if (mprls_i2c_client != NULL) {
        err = i2c_add_driver(&mprls_driver);
        if (err < 0) {
            printk(KERN_ERR "mprls: could not add an i2c driver.\n");
            retval = -1;
        }
    }
    i2c_put_adapter(mprls_i2c_adapter);

    printk(KERN_INFO "mprls: i2c driver added.\n");

    // buffer[0] = 0xAA;
    // buffer[1] = 0x00;
    // buffer[2] = 0x00;
    // err = i2c_master_send(mprls_i2c_client, buffer, 3);
    // for (int i = 0; i < 10000000; ++i)
    //     asm("nop");

    // if (err < 0) {
    //     printk(KERN_ERR "mprls: error when sending i2c data.\n");
    // } else {
    //     err = i2c_master_recv(mprls_i2c_client, buffer, 3);
    // }

    // if (err < 0) {
    //     printk(KERN_ERR "mprls: error when receiving i2c data.\n");
    // } else {
    //     // printk(KERN_INFO "mprls: STATUS BIT: %d.\n", buffer[0]);
    //     printk(KERN_INFO "mprls: RESULT: %d, %d, %d", buffer[1], buffer[2], buffer[3]);
    // }

    return 0;

ker_err:
    device_destroy(devclass, devno);
device_err:
    class_destroy(devclass);
class_err:
    unregister_chrdev_region(devno, 1);
    return -1;
}

static void __exit mprls_exit(void) {
    cdev_del(&device);

    i2c_unregister_device(mprls_i2c_client);
	i2c_del_driver(&mprls_driver);

    device_destroy(devclass, devno);
    class_destroy(devclass);

    unregister_chrdev_region(devno, 1);

    printk(KERN_INFO "mprls: driver removed. \n");
}

module_init(mprls_init);
module_exit(mprls_exit);
