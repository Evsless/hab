#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/units.h>
#include <linux/math64.h>
#include <linux/module.h>

#include <linux/iio/iio.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

#include "icm20x.h"

/***********************************************************************************************************************
* STRUCT PROTOTYPES                                                                                               
***********************************************************************************************************************/
struct icm20x_fields {
    u16 acc_x;
    u16 acc_y;
    u16 acc_z;
};

struct icm20x_data {
    struct  mutex lock;
    struct  i2c_client *client;

    struct  icm20x_fields sshot_data;
    u16     fifo[2048];
};

/***********************************************************************************************************************
* INTERNAL FUNCTION DEFINITIONS                                                                                               
***********************************************************************************************************************/
static int icm20x_write_reg(const struct i2c_client *client, u8 *buf, u32 size);
static int icm20x_read_reg(const struct i2c_client *client, u8 *buf, u32 size, const u8 reg);

static int icm20x_reset(struct icm20x_data *data);
static int icm20x_fifo_reset(const struct icm20x_data *data);
static int icm20x_init(struct icm20x_data *data);
static int icm20x_read_accel_sshot(struct icm20x_data *data);


static int icm20x_probe(struct i2c_client *client);
static int icm20x_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, 
                                int *val, int *val2, long mask);

static int icm20x_buffer_postenable(struct iio_dev *indio_dev);

static void icm20x_arrange_axis_data(u16 *sample, const unsigned long *scan_channels);
/***********************************************************************************************************************
* GLOBAL DATA DECLARATION                                                                                               
***********************************************************************************************************************/

static struct icm20x_reg_ops reg_ops = {
    .read_reg = icm20x_read_reg,
    .write_reg = icm20x_write_reg,
};

static const struct iio_chan_spec icm20x_channels[] = {
    {
        .type = IIO_ACCEL,
        .modified = 1,
        .channel2 = IIO_MOD_X,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .scan_index = 0,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_CPU,
        },
    },
    {
        .type = IIO_ACCEL,
        .modified = 1,
        .channel2 = IIO_MOD_Y,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .scan_index = 1,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_CPU,
        },
    },
    {
        .type = IIO_ACCEL,
        .modified = 1,
        .channel2 = IIO_MOD_Z,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .scan_index = 2,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_CPU,
        },
    },
};

static const struct i2c_device_id icm20x_ids[] = {
    { "icm20x" },
    { }
};
MODULE_DEVICE_TABLE(i2c, icm20x_ids);

static const struct of_device_id icm20x_matches[] = {
    { .compatible = "invensense,icm20x" },
    { }
};
MODULE_DEVICE_TABLE(of, icm20x_matches);

static const struct iio_info icm20x_info = {
    .read_raw = icm20x_read_raw,
};

static const struct iio_buffer_setup_ops icm20x_buffer_ops = {
    .postenable = icm20x_buffer_postenable,
};

static struct i2c_driver icm20x_driver = {
    .probe = icm20x_probe,
    .id_table = icm20x_ids,
    .driver = {
        .name = "icm20x",
        .of_match_table = icm20x_matches,
    },
};

module_i2c_driver(icm20x_driver);

/***********************************************************************************************************************
* INTERNAL FUNCTION DECLARATIONS                                                                                               
***********************************************************************************************************************/
static void icm20x_arrange_axis_data(u16 *sample, const unsigned long *scan_channels) {
    int i = 0;
    u16 axis_sample[ICM20X_FIFO_SET_SIZE];

    memset(axis_sample, 0, ICM20X_FIFO_SET_SIZE * sizeof(u16));

    if (*scan_channels & SCAN_CHAN_0)
        axis_sample[i++] = sample[0];
    if (*scan_channels & SCAN_CHAN_1)
        axis_sample[i++] = sample[1];
    if (*scan_channels & SCAN_CHAN_2)
        axis_sample[i++] = sample[2];

    memcpy(sample, axis_sample, ICM20X_FIFO_SET_SIZE * sizeof(u16));
}

static irqreturn_t icm20x_trigger_handler(int irq, void *p) {
    int ret;
    u16 fifo_len;
    u8 buffer[2];
    struct iio_poll_func *pf = p; 
    struct iio_dev *indio_dev = pf->indio_dev;
    struct icm20x_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);

    /* 1. Set the reg bank 0 */
    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_0;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        goto release;

    /* 2. Read FIFO length */
    ret = reg_ops.read_reg(data->client, buffer, sizeof(buffer), ICM20X_REG_FIFO_COUNTH);
    if (ret)
        goto release;
    fifo_len = ((buffer[0] << 8) | (buffer[1] & 0xFF)) >> 1;

    /* 3. Read FIFO */
    ret = reg_ops.read_reg(data->client, (u8 *)data->fifo, sizeof(data->fifo), ICM20X_REG_R_W);
    if (ret)
        goto release;

    /* 4. Push collected data to FIFO */
    for (u16 i = 0; i < fifo_len; i += ICM20X_FIFO_SET_SIZE) {
        icm20x_arrange_axis_data(&data->fifo[i], indio_dev->active_scan_mask);
        iio_push_to_buffers(indio_dev, &data->fifo[i]);
    }

    /* 4. Reset the FIFO */
    ret = icm20x_fifo_reset(data);

release:

    mutex_unlock(&data->lock);
    iio_trigger_notify_done(indio_dev->trig);

    return IRQ_HANDLED;
}

static int icm20x_write_reg(const struct i2c_client *client, u8 *buf, u32 size) {
    int ret;

    ret = i2c_master_send(client, buf, size);
    if (ret < 0) {
        dev_err(&client->dev, "regops_w: error when sending data, code: %d.\n", ret);
    } else if (size != ret) {
        dev_err(&client->dev, "regops_w: message wasn't send completely. Send: %d, expected: %u.\n", 
                    ret, size);
        ret = -EIO;
    } else {
        ret = 0;
    }

    return ret;
}

static int icm20x_read_reg(const struct i2c_client *client, u8 *buf, u32 size, const u8 reg) {
    int ret = 0;
    u8 target_reg = reg;

    ret = icm20x_write_reg(client, &target_reg, sizeof(target_reg));
    if (ret)
        return ret;

    ret = i2c_master_recv(client, buf, size);
    if (ret < 0) {
        dev_err(&client->dev, "regops_r: error when reading data, code: %d.\n", ret);
    } else if (size != ret) {
        dev_err(&client->dev, "regops_r: incoplete message read. Read: %d, expected: %u.\n",
                ret, size);
        ret = -EIO;
    } else {
        ret = 0;
    }

    return ret;
}

static int icm20x_reset(struct icm20x_data *data) {
    int ret;
    u8 buffer[2];

    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_0;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_PWR_MGMT_1;
    buffer[1] = ICM20X_MASK_PWR_MGMT_RESET;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    usleep_range(20000, 30000);

    printk(KERN_INFO "Reset completed\n");
    return 0;
}

static int icm20x_fifo_reset(const struct icm20x_data *data) {
    int ret;
    u8 buffer[2];

    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_0;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_FIFO_RST;
    buffer[1] = ICM20X_MASK_FIFO_RST_ASSERT;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[1] = ICM20X_MASK_FIFO_RST_DEASSERT;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    return 0;
}

static int icm20x_init(struct icm20x_data *data) {
    int ret = 0;
    u8 buffer[2];

    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_0;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_PWR_MGMT_1;
    buffer[1] = ICM20X_MASK_PWR_MGMT_TEMP_DIS | ICM20X_MASK_PWR_MGMT_CLKSEL;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_PWR_MGMT_2;
    buffer[1] = ICM20X_MASK_PWR_MGMT_2_DISABLE_GYRO;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    /* Setting up REG BANK 2 registers */
    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_2;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;
    
    buffer[0] = ICM20X_REG_ACCEL_SMPLRT_DIV_2;
    buffer[1] = 0x0A;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_ODR_ALIGN_EN;
    buffer[1] = ICM20X_MASK_ODR_ALIGN_EN_ON;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_ACCEL_CONFIG;
    buffer[1] = ICM20X_MASK_ACCEL_CONFIG_ACCEL_FS_SEL_2G |
                    ICM20X_MASK_ACCEL_CONFIG_ACCEL_FCHOICE_ON;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));

    return 0;
}

static int icm20x_buffer_postenable(struct iio_dev *indio_dev) {
    int ret = 0;
    u8 buffer[2];
    struct icm20x_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);

    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_0;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    /**
     * FIFO buffer setup.
     * 1) Enable FIFO;
     * 2) Write accel to FIFO;
     * 3) Setup FIFO mode;
     *
    */
    buffer[0] = ICM20X_REG_USER_CTRL;
    buffer[1] = ICM20X_MASK_USER_CTRL_FIFO_EN;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_FIFO_EN_2;
    buffer[1] = ICM20X_MASK_FIFO_EN_2_ACCEL_FIFO_EN;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    buffer[0] = ICM20X_REG_FIFO_MODE;
    buffer[1] = ICM20X_MASK_FIFO_MODE_SNAPSHOT;
    ret = reg_ops.write_reg(data->client, buffer, sizeof(buffer));
    if (ret)
        return ret;

    ret = icm20x_fifo_reset(data);
    if (ret)
        return ret;

    mutex_unlock(&data->lock);
    
    return 0;
}

static int icm20x_read_accel_sshot(struct icm20x_data *data) {
    int ret = 0;
    u8 buffer[6];

    buffer[0] = ICM20X_REG_BANK_SEL;
    buffer[1] = ICM20X_MASK_SEL_UB_0;
    ret = reg_ops.write_reg(data->client, buffer, ICM20X_WORD_SIZE);
    if (ret)
        return ret;

    ret = reg_ops.read_reg(data->client, buffer, sizeof(buffer), ICM20X_REG_ACCEL_XOUT_H);
    if (ret)
        return ret;

    data->sshot_data.acc_x = (buffer[0] << 8) | buffer[1];
    data->sshot_data.acc_y = (buffer[2] << 8) | buffer[3];
    data->sshot_data.acc_z = (buffer[4] << 8) | buffer[5];

    printk(KERN_INFO "--RAW READOUT--\n");
    for (u8 i = 0; i < 6; i++)
        printk(KERN_INFO "%u) %u\n", i, buffer[i]);
    return 0;
}

static int icm20x_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, 
                                int *val, int *val2, long mask) {
    int ret;
    u16 raw_val;
    struct icm20x_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);
    ret = icm20x_read_accel_sshot(data);
    mutex_unlock(&data->lock);

    switch (chan->channel2) {
        case IIO_MOD_X:
            raw_val = data->sshot_data.acc_x;
            break;
        case IIO_MOD_Y:
            raw_val = data->sshot_data.acc_y;
            break;
        case IIO_MOD_Z:
            raw_val = data->sshot_data.acc_z;
            break;
        default:
            break;
    }

    switch (mask) {
        case IIO_CHAN_INFO_RAW:
            *val = raw_val;
            return IIO_VAL_INT;
        default:
            return -EINVAL;
    }

    return 0;
}

static int icm20x_probe(struct i2c_client *client) {
    int ret = 0;
    struct icm20x_data *data;
    struct iio_dev *indio_dev;
    struct device *dev = &client->dev;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
        dev_err_probe(dev, -EOPNOTSUPP, "i2c is not supported.\n");

    indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
    if (!indio_dev)
        dev_err_probe(dev, -ENOMEM, "could not create iio device.\n");

    data = iio_priv(indio_dev);
    
    data->client = client;
    mutex_init(&data->lock);
    
    indio_dev->name = "icm20x";
    indio_dev->info = &icm20x_info;
    indio_dev->channels = icm20x_channels;
    indio_dev->num_channels = ARRAY_SIZE(icm20x_channels);
    
    /* WARNING: That field might be modified for kfifo. */
    indio_dev->modes = INDIO_DIRECT_MODE | INDIO_BUFFER_SOFTWARE;

    mutex_lock(&data->lock);
    ret = icm20x_reset(data);
    mutex_unlock(&data->lock);

    mutex_lock(&data->lock);
    ret = icm20x_init(data);
    mutex_unlock(&data->lock);

    if (ret)
        return ret;
    
    ret = devm_iio_triggered_buffer_setup_ext(dev, 
                                              indio_dev, NULL, 
                                              icm20x_trigger_handler,
                                              IIO_BUFFER_DIRECTION_IN,
                                              &icm20x_buffer_ops,
                                              NULL);
    if (ret)
        return dev_err_probe(dev, ret, "iio triggered buffer setup failed.\n");

    ret = devm_iio_device_register(dev, indio_dev);
    if (ret)
        return dev_err_probe(dev, ret, "unable to register iio device.\n");
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("icm20x iio driver");