#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/units.h>
#include <linux/regmap.h>
#include <linux/module.h>
#include <linux/math64.h>

#include <linux/iio/iio.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

/***********************************************************************************************************************
* PREPROCESSOR DEFINITIONS                                                                                               
***********************************************************************************************************************/

/*
 * 1. Address Point Register list
 */
#define ADS1115_APR_CONV_REG    0x00
#define ADS1115_APR_CONFIG_REG  0x01
#define ADS1115_APR_LO_THR_REG  0x02
#define ADS1115_APR_HI_THR_REG  0x03

/*
 * 2. Config Register field descriptions
 */

#define ADS1115_CONF_OS         (0x01 << 15)
#define ADS1115_CONF_MUX        (0x01 << 12)
#define ADS1115_CONF_PGA        (0x01 << 9)
#define ADS1115_CONF_MODE       (0x01 << 8)
#define ADS1115_CONF_DR         (0x01 << 5)
#define ADS1115_CONF_COMP_MODE  (0x01 << 4)
#define ADS1115_CONF_COMP_POL   (0x01 << 3)
#define ADS1115_CONF_COMP_LAT   (0x01 << 2)
#define ADS1115_CONF_COMP_QUE   0x00

/*
 * 3. Config register masks
 */

/* 3.1) CONF_OS masks */
#define ADS1115_MASK_SCON_START 0x01

/* 3.2) CONF_MODE masks */
#define ADS1115_MASK_CCM 0x00
#define ADS1115_MASK_SSM 0x01

/* 3.3) COMP_QUE masks */
#define ADS1115_MASK_COMP_DISABLE 0x03

/*
 * 4. Register offsets
 */
#define ADS1115_OS_OFFSET  	1
#define ADS1115_MUX_OFFSET 	12
#define ADS1115_PGA_OFFSET 	9
#define ADS1115_MODE_OFFSET 8
#define ADS1115_DR_OFFSET   5

/*
 * 4. Helper macros
 */

 #define FIT_CHECK(_testbits, _fitbits)					\
	(								                    \
		(_fitbits) *						            \
		!!sizeof(struct {					            \
			static_assert((_testbits) <= (_fitbits));	\
			int pad;					                \
		})							                    \
	)

#define ADS1115_V_CHAN(_chan, _addr, _realbits, _shift) 		\
{ 																\
	.type = IIO_VOLTAGE,					                    \
	.indexed = 1,						                        \
	.address = _addr,					                        \
	.channel = _chan,					                        \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |		        \
				BIT(IIO_CHAN_INFO_SCALE) |	                    \
				BIT(IIO_CHAN_INFO_SAMP_FREQ),	                \
	.scan_index = _addr,					                    \
	.scan_type = {						                        \
		.sign = 's',					                        \
		.realbits = (_realbits),			                    \
		.storagebits = FIT_CHECK((_realbits) + (_shift), 16),	\
		.shift = (_shift),				                        \
		.endianness = IIO_CPU,				                    \
	},							                                \
	.datasheet_name = "AIN"#_chan,				                \
}

#define ADS1115_V_DIFF_CHAN(_chan, _chan2, _addr, _realbits, _shift) \
{ 																\
	.type = IIO_VOLTAGE,					                    \
	.differential = 1,					                        \
	.indexed = 1,						                        \
	.address = _addr,					                        \
	.channel = _chan,					                        \
	.channel2 = _chan2,					                        \
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |		        \
				BIT(IIO_CHAN_INFO_SCALE) |	                    \
				BIT(IIO_CHAN_INFO_SAMP_FREQ),	                \
	.scan_index = _addr,					                    \
	.scan_type = {						                        \
		.sign = 's',					                        \
		.realbits = (_realbits),			                    \
		.storagebits = FIT_CHECK((_realbits) + (_shift), 16),	\
		.shift = (_shift),				                        \
		.endianness = IIO_CPU,				                    \
	},							                                \
	.datasheet_name = "AIN"#_chan"-AIN"#_chan2,		            \
}

#define ADS1115_CHANNELS 8
#define ADS1115_DEFAULT_PGA 2
#define ADS1115_DEFAULT_DR  4

/***********************************************************************************************************************
* INTERNAL STRUCTURES DEFINITION                                                                                               
***********************************************************************************************************************/

struct ads1115_chan_data {
	bool    enabled;
    u8 		pga;
    u8 		dr;

    u16 	val;
};

struct ads1115_data {
	struct regmap *regmap;
    struct mutex lock;

    struct ads1115_chan_data channels[ADS1115_CHANNELS];
};

enum ads1115_meas_channels {
	ADS1115_AIN0_AIN1 = 0,
	ADS1115_AIN0_AIN3,
	ADS1115_AIN1_AIN3,
	ADS1115_AIN2_AIN3,
	ADS1115_AIN0,
	ADS1115_AIN1,
	ADS1115_AIN2,
	ADS1115_AIN3,
	ADS1115_TIMESTAMP
};

static const int ads1115_dr_lut[] = {
	8, 16, 32, 64, 128, 250, 475, 860
};

static const int ads1115_fsr_lut[] = {
	6144, 4096, 2048, 1024, 512, 256, 256, 256
};

/**
 * Delay time in us is calculated as follows
 *   td = 1 / dr
 * Since the resolution in 2bytes, then
 *   td = 2 / dr
 */
static const u32 ads1115_delay_lut[] = {
	250000,
	125000,
	62500,
	31250,
	15625,
	8000,
	4210,
	2325
};

static const struct regmap_range ads1115_wr_ranges[] = {
	regmap_reg_range(ADS1115_APR_CONFIG_REG, ADS1115_APR_HI_THR_REG),
};

static const struct regmap_access_table ads1115_wr_table = {
	.yes_ranges = ads1115_wr_ranges,
	.n_yes_ranges = ARRAY_SIZE(ads1115_wr_ranges),
};

static const struct regmap_config ads1115_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,
	.max_register = ADS1115_APR_HI_THR_REG,
	.wr_table = &ads1115_wr_table,
};

/***********************************************************************************************************************
* INTERNAL FUNCTION DEFINITIONS                                                                                               
***********************************************************************************************************************/
static int ads1115_probe(struct i2c_client *client);
static int ads1115_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, 
                                int *val, int *val2, long mask);
static int ads1115_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan,
								int val, int val2, long mask);

static int ads1115_read_channel(struct ads1115_data *data, int channel);


static int ads1115_set_fsr(struct ads1115_data *data, struct iio_chan_spec const *chan,
								int scale, int uscale);

static int ads1115_set_dr(struct ads1115_data *data, struct iio_chan_spec const *chan,
							int rate);

static irqreturn_t ads1115_trigger_handler(int irq, void *p);

/***********************************************************************************************************************
* DRIVER DATA DEFINITION                                                                                               
***********************************************************************************************************************/
static const struct iio_chan_spec ads1115_channels[] = {
    ADS1115_V_DIFF_CHAN(0, 1, ADS1115_AIN0_AIN1, 16, 0),
    ADS1115_V_DIFF_CHAN(0, 3, ADS1115_AIN0_AIN3, 16, 0),
    ADS1115_V_DIFF_CHAN(1, 3, ADS1115_AIN1_AIN3, 16, 0),
    ADS1115_V_DIFF_CHAN(2, 3, ADS1115_AIN2_AIN3, 16, 0),
	ADS1115_V_CHAN(0, ADS1115_AIN0, 16, 0),
	ADS1115_V_CHAN(1, ADS1115_AIN1, 16, 0),
	ADS1115_V_CHAN(2, ADS1115_AIN2, 16, 0),
	ADS1115_V_CHAN(3, ADS1115_AIN3, 16, 0),
	IIO_CHAN_SOFT_TIMESTAMP(ADS1115_TIMESTAMP),
};


static const struct of_device_id ads1115_matches[] = {
    {.compatible = "ti,ads1115"},
    { }
};
MODULE_DEVICE_TABLE(of, ads1115_matches);

static const struct i2c_device_id ads1115_ids[] = {
    { "ads1115" },
    { }
};
MODULE_DEVICE_TABLE(i2c, ads1115_ids);

static const struct iio_info ads1115_info = {
    .read_raw = ads1115_read_raw,
	.write_raw = ads1115_write_raw,
};

static struct i2c_driver ads1115_driver = {
    .probe = ads1115_probe,
    .id_table = ads1115_ids,
    .driver = {
        .name = "ads1115",
        .of_match_table = ads1115_matches,
    },
};

module_i2c_driver(ads1115_driver);

/***********************************************************************************************************************
* INTERNAL FUNCTION DECLARATIONS                                                                                               
***********************************************************************************************************************/

static int ads1115_probe(struct i2c_client *client) {
	int ret = 0;
	u32 pval, channel, pga, dr;

    struct ads1115_data *data;
    struct iio_dev *indio_dev;
    struct device *dev = &client->dev;

	struct fwnode_handle *node;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
        dev_err_probe(dev, -EOPNOTSUPP, "i2c is not supported.\n");

    indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
    if (!indio_dev)
        dev_err_probe(dev, -ENOMEM, "could not create iio device.\n");

    data = iio_priv(indio_dev);
	mutex_init(&data->lock);

	indio_dev->name 		= "ads1115";
	indio_dev->info 		= &ads1115_info;
	indio_dev->channels 	= ads1115_channels;
	indio_dev->num_channels = ARRAY_SIZE(ads1115_channels);
	indio_dev->modes 		= INDIO_DIRECT_MODE;
	data->regmap 			= devm_regmap_init_i2c(client, &ads1115_regmap_config);

	device_for_each_child_node(dev, node) {
		pga = ADS1115_DEFAULT_PGA;
		dr  = ADS1115_DEFAULT_DR;

		if (fwnode_property_read_u32(node, "reg", &pval)) {
			dev_err(dev, "Invalid reg on %pfw\n", node);
			continue;
		}
		channel = pval;


		if (!fwnode_property_read_u32(node, "ti,gain", &pval)) {
			pga = pval;
		}

		if (!fwnode_property_read_u32(node, "ti,datarate", &pval)) {
			dr = pval;
		}

		data->channels[channel].enabled = true;
		data->channels[channel].pga = pga;
		data->channels[channel].dr = dr;
	}

	ret = devm_iio_triggered_buffer_setup(dev, indio_dev, NULL, ads1115_trigger_handler, NULL);
	if (ret)
		return dev_err_probe(dev, ret, "iio triggered buffer setup failed\n");

	ret = devm_iio_device_register(dev, indio_dev);
	if (ret)
		return dev_err_probe(dev, ret, "unable to register iio device.\n");

    return 0;
}

static irqreturn_t ads1115_trigger_handler(int irq, void *p) {
	int ret;
	u8 cnt = 0;

	/**
	 * Timestamp size is 64 bit, then:
	 *     64 / 16 = 4.
	 * If every channel is measured,
	 * then 16 * 4 bits must be added.
	 */
	u16 buffer[ADS1115_CHANNELS + 4];

	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct ads1115_data *data = iio_priv(indio_dev);

	memset(buffer, 0, sizeof(buffer));

	mutex_lock(&data->lock);
	for (u8 i = 0; i < ADS1115_CHANNELS; i++) {
		if (data->channels[i].enabled) {
			ret = ads1115_read_channel(data, i);
			if (ret)
				goto release;

			buffer[cnt++] = data->channels[i].val;
		}
	}

	iio_push_to_buffers_with_timestamp(indio_dev, buffer, iio_get_time_ns(indio_dev));

release:
	mutex_unlock(&data->lock);
	iio_trigger_notify_done(indio_dev->trig);

	return IRQ_HANDLED;
}

static int ads1115_read_channel(struct ads1115_data *data, int channel) {
	int ret;
	unsigned int request;

	u8 pga_idx = data->channels[channel].pga;
	u8 dr_idx  = data->channels[channel].dr;

	/* 1. Initiate conversion bit */
	request = ADS1115_CONF_OS;

	/* 2. Setup channel to be read */
	request |= channel << ADS1115_MUX_OFFSET;
	 
	/* 3. Setup FSR */
	request |= pga_idx << ADS1115_PGA_OFFSET;

	/* 4. Setup operating mode (hab uses SSM by default) */
	request |= ADS1115_MASK_SSM << ADS1115_MODE_OFFSET;

	/* 5. Setup data rate */
	request |= dr_idx << ADS1115_DR_OFFSET;

	/* 6. Disable the comparator by default */
	request |= ADS1115_MASK_COMP_DISABLE;

	ret = regmap_write(data->regmap, ADS1115_APR_CONFIG_REG, request);
	if (ret)
		return ret;
	
	for (u8 i = 0; i < 10; i++) {
		usleep_range(ads1115_delay_lut[dr_idx], ads1115_delay_lut[dr_idx] + 2000);

		ret = regmap_read(data->regmap, ADS1115_APR_CONFIG_REG, &request);
		if (ret)
			return ret;
		
		if (request & ADS1115_CONF_OS)
			break;
	}

	ret = regmap_read(data->regmap, ADS1115_APR_CONV_REG, &request);
	if (ret)
		return ret;

	data->channels[channel].val = request;

	return 0;
}

static int ads1115_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, 
                                int *val, int *val2, long mask)
{
	int ret;
	u8 idx = 0;
	struct ads1115_data *data = iio_priv(indio_dev);

	mutex_lock(&data->lock);
	/* Consider checking the return status from funtion */
	if (data->channels[chan->address].enabled)
		ret = ads1115_read_channel(data, chan->address);
	mutex_unlock(&data->lock);

	switch(mask) {
		case IIO_CHAN_INFO_RAW:
			*val = data->channels[chan->address].val;
			return IIO_VAL_INT;
		case IIO_CHAN_INFO_SCALE:
			idx = data->channels[chan->address].pga;
			*val   = ads1115_fsr_lut[idx];
			*val2  = chan->scan_type.realbits - 1;
			return IIO_VAL_FRACTIONAL_LOG2;
		case IIO_CHAN_INFO_SAMP_FREQ:
			idx = data->channels[chan->address].dr;
			*val = ads1115_dr_lut[idx];
			return IIO_VAL_INT;
		default:
			return -EINVAL;
	}
}

static int ads1115_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan,
								int val, int val2, long mask)
{
	int ret = 0;
	struct ads1115_data *data = iio_priv(indio_dev);

	mutex_lock(&data->lock);
	switch(mask) {
		case IIO_CHAN_INFO_SCALE:
			ads1115_set_fsr(data, chan, val, val2);
			break;
		case IIO_CHAN_INFO_SAMP_FREQ:
			ads1115_set_dr(data, chan, val);
			break;

		default:
			ret = -EINVAL;
			break;
	}
	mutex_unlock(&data->lock);
	
	return ret;
}

static int ads1115_set_fsr(struct ads1115_data *data, struct iio_chan_spec const *chan,
								int scale, int uscale)
{
	int fsr = div_s64((scale * MICRO + uscale), MILLI);

	for (u8 i = 0; i < ARRAY_SIZE(ads1115_fsr_lut); i++) {
		if (ads1115_fsr_lut[i] == fsr) {
			data->channels[chan->address].pga = i;
			printk(KERN_INFO "FSR: %d\n", fsr);
			return 0;
		}
	}
	return -EINVAL;
}

static int ads1115_set_dr(struct ads1115_data *data, struct iio_chan_spec const *chan,
							int rate)
{
	for (u8 i = 0; i < ARRAY_SIZE(ads1115_dr_lut); i++) {
		if (ads1115_dr_lut[i] == rate) {
			data->channels[chan->address].dr = i;
			printk(KERN_INFO "DR: %d\n", rate);
			return 0;
		}
	}
	return -EINVAL;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("Simplified ads1115 driver for hab mission");