#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("Registers a device nr. and implement some callback functions");

/* Buffers for data */
static char buffer[255];
static int buffer_ptr = 0;

/* Variables for device and device class */
static dev_t mydev_nr;
static struct cdev mycdev;
static struct class *myclass;

#define MINOR_NUM 0
#define DRIVER_NAME "dev_nr"
#define CLASS_NAME "dev_nr_class"

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	printk("dev_nr - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is closed
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk("dev_nr - close was called!\n");
	return 0;
}

/**
 * @brief Read data out of the buffer
*/
static ssize_t driver_read(struct file *filp, char __user *ubuff, size_t count, loff_t *offp) {
	int to_copy, not_copied, delta;

	/* Calculate amout of data to be copied */
	to_copy = min(count, (size_t)buffer_ptr);

	/* Copy data to user */
	not_copied = copy_to_user(ubuff, buffer, to_copy);

	/* Calculate the remaining data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief Write the data to the buffer
*/
static ssize_t driver_write(struct file *filp, const char __user *ubuff, size_t count, loff_t *offp) {
	int to_copy, not_copied, delta;

	/* Get amount of data to be copied */
	to_copy = min(count, sizeof(buffer));

	/* Copy the data from user buffer to kernel */
	not_copied = copy_from_user(buffer, ubuff, to_copy);
	buffer_ptr = to_copy;

	/* Calculate the rest of the data */
	delta = to_copy - not_copied;

	return delta;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write
};


/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
    int err = 0;

	/* register device nr. */
    err = alloc_chrdev_region(&mydev_nr, MINOR_NUM, 1, DRIVER_NAME);

	if (err < 0) {
		printk(KERN_INFO "Could not register device number!\n");
		return -1;
	} else {
        printk(KERN_INFO "dev_nr - registered Device number Major: %d, Minor: %d\n", MAJOR(mydev_nr), MINOR(mydev_nr));
    }

	/* Create a device class */
	myclass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(myclass)) {
		printk(KERN_INFO "ERROR: Imposible to create a device class.\n");
		goto class_error;
	}

	/* Create a device file */
	if (IS_ERR(device_create(myclass, NULL, mydev_nr, NULL, DRIVER_NAME))) {
		printk(KERN_INFO "ERROR: Impossible to add a device.\n");
		goto device_error;
	}

    cdev_init(&mycdev, &fops);
    mycdev.owner = THIS_MODULE;

    err = cdev_add(&mycdev, mydev_nr, 1);
    if (err < 0) {
        pr_debug("Error(%d): Adding %s error\n", err, DRIVER_NAME);
		goto cdev_add_error;
    }

	return 0;

cdev_add_error:
	device_destroy(myclass, mydev_nr);
device_error:
	class_destroy(myclass);
class_error:
	unregister_chrdev_region(mydev_nr, 1);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
    cdev_del(&mycdev);
	device_destroy(myclass, mydev_nr);
	class_destroy(myclass);
	unregister_chrdev_region(mydev_nr, 1);

	printk("dev_nr: exiting the device driver.\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);