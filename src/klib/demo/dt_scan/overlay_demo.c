#include <linux/module.h>
#include <linux/init.h>

#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/of_device.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("Parses the DT and extracts data from it.");

/* Declare probe init & remove functions */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "hab,mydev",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
	.probe = dt_probe,
	.remove = dt_remove,
	.driver = {
		.name = "my_device_driver",
		.of_match_table = my_driver_ids,
	},
};


static int dt_probe(struct platform_device *pdev) {
	struct device *dev = &pdev->dev;
	const char *label;
	int my_value, ret;

	printk("overlay_demo: entered probe function.\n");

	/* Check for device properties */
	if(!device_property_present(dev, "label")) {
		printk("overlay_demo: property 'label' is not present.\n");
		return -1;
	}
	if(!device_property_present(dev, "my_value")) {
		printk("overlay_demo: property 'my_value' is not present.\n");
		return -1;
	}

	/* Read device properties */
	ret = device_property_read_string(dev, "label", &label);
	if(ret) {
		printk("overlay_demo: property 'label' could to be read.\n");
		return -1;
	}
	printk("overlay_demo: 'label' property value %s\n", label);

	ret = device_property_read_u32(dev, "my_value", &my_value);
	if(ret) {
		printk("overlay_demo: property 'my_value' could to be read.\n");
		return -1;
	}
	printk("overlay_demo: 'my_value' property value %d.\n", my_value);

	return 0;
}

static int dt_remove(struct platform_device *pdev) {
	printk("overlay_demo: entered dt_remove.\n");
	return 0;
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("overlay_demo: loading the driver.\n");
	if(platform_driver_register(&my_driver)) {
		printk("overlay_demo: could not register the driver.\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("overlay_demo: unloading the driver.\n");
	platform_driver_unregister(&my_driver);
}

module_init(my_init);
module_exit(my_exit);