#include <linux/i2c.h>
#include <linux/units.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/math64.h>
#include <linux/iio/iio.h>
#include <linux/property.h>
#include <linux/completion.h>
#include <linux/mod_devicetable.h>
#include <linux/regulator/consumer.h>

#include <asm-generic/unaligned.h>

#define MPRLS_I2C_BUSY BIT(5)

enum mprls_tf_id {
    MPRLS_FUNCTION_A,
    MPRLS_FUNCTION_B,
    MPRLS_FUNCTION_C,
};

struct mprls_tf_spec {
    u32 output_min;
    u32 output_max;
};

static const struct mprls_tf_spec mprls_tf_specs[] = {
    [MPRLS_FUNCTION_A] = {
        .output_min = 1677722,
        .output_max = 15099494
    },
    [MPRLS_FUNCTION_B] = {
        .output_min =  419430,
        .output_max =  3774874
    },
    [MPRLS_FUNCTION_C] = {
        .output_min = 3355443,
        .output_max = 13421773
    }
};

struct mprls_chan {
    s32 pres;
    s64 ts;
};

struct mprls_data {
    struct i2c_client *client;
    /* struct mutex lock; */
    u32 pmin;
    u32 pmax;
    
    enum mprls_tf_id function;
    u32 outmin;
    u32 outmax;

    int scale;
    int scale2;

    int offset;
    int offset2;
    
    struct completion completion;
    struct mprls_chan chan;
};

static const struct iio_chan_spec mprls_channels[] = {
    {    
        .type = IIO_PRESSURE,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | 
                                BIT(IIO_CHAN_INFO_SCALE) |
                                BIT(IIO_CHAN_INFO_OFFSET),
        .scan_index = 0,
        .scan_type = {
            .sign = 's',
            .realbits = 32,
            .storagebits = 32,
            .endianness = IIO_CPU,
        },
    },
    IIO_CHAN_SOFT_TIMESTAMP(1)
};

static const struct of_device_id mprls_matches[] = {
    {.compatible = "honeywell,mprls0025pa"},
    { }
};
MODULE_DEVICE_TABLE(of, mprls_matches);

static const struct i2c_device_id mprls_ids[] = {
    { "mprls0025pa" },
    { }
};
MODULE_DEVICE_TABLE(i2c, mprls_ids);

static int mprls_read_pressure(struct mprls_data *data, s32 *pressure) {
    struct device *dev = &data->client->dev;
    int ret = 0;
    u8 w_buff[] = {0xAA, 0x00, 0x00};
    u8 r_buff[4];

    s32 status;
    u8 nloops = 40, it = 0;

    ret = i2c_master_send(data->client, w_buff, sizeof(w_buff));
    if (ret < 0) {
        dev_err(dev, "Error when sending the data. Code: %d\n", ret);
        return ret;
    }

    if (ret != sizeof(w_buff)) {
        dev_err(dev, "The message wasn't send completely. Send: %d, expected: %u\n", ret, (u32)sizeof(w_buff));
        return -EIO;
    }

    for (it = 0; it < nloops; it++) {
        usleep_range(5000, 10000);
        status = i2c_smbus_read_byte(data->client);
        if (status < 0) {
            dev_err(dev, "error while reading the status. Status: %d\n", status);
            return status;
        }

        if (!(status & MPRLS_I2C_BUSY))
            break;
    }

    if (nloops == 10) {
        dev_err(dev, "error while reading. Timeout\n");
        return -ETIMEDOUT;
    }

    ret = i2c_master_recv(data->client, r_buff, sizeof(r_buff));
    if (ret < 0) {
        dev_err(dev, "error in i2c_master_recv. Code: %d\n", ret);
        return ret;
    }

    if (sizeof(r_buff) != ret) {
        dev_err(dev, "The message wasn't send completely. Send: %d, expected: %u\n", 
                    ret, (u32)sizeof(r_buff));
        return -EIO;
    }

    /* EXTRA CHECKOUT */
    if (r_buff[0] & MPRLS_I2C_BUSY) {
        dev_err(dev, "error while reading. Data still not ready. Status reg: %08x\n", r_buff[0]);
        return -ETIMEDOUT;
    }

    *pressure = get_unaligned_be24(&r_buff[1]);

    dev_dbg(dev, "received %*ph cnt: %d\n", ret, r_buff, *pressure);

    return 0;
}

static int mprls_read_raw(struct iio_dev *indio_dev, 
        struct iio_chan_spec const *chan, int *val, int *val2, long mask) {
    int ret;
    s32 pressure;
    struct mprls_data *data = iio_priv(indio_dev);

    if (chan->type != IIO_PRESSURE)
        return -EINVAL;

    switch (mask) {
        case IIO_CHAN_INFO_RAW:
            ret = mprls_read_pressure(data, &pressure);
            if (ret < 0)
                return ret;
            *val = pressure;
            return IIO_VAL_INT;
        default:
            return -EINVAL;
    }
    return 0;
}

static const struct iio_info mprls_info = {
    .read_raw = mprls_read_raw,
};

static int mprls_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    int ret;
    struct mprls_data *data;
    struct iio_dev *indio_dev;
    struct device *dev = &client->dev;
    s64 scale, offset;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
        return dev_err_probe(dev, -EOPNOTSUPP, "I2C functionality is not supported.\n");

    indio_dev = devm_iio_device_alloc(dev, sizeof(*data));

    if (!indio_dev)
        return dev_err_probe(dev, -ENOMEM, "IIO device could not be created.\n");

    data = iio_priv(indio_dev); /* Memory allocation for a device */
    data->client = client;

    /**
     * TBD
     * 1) Initialize a mutex;
     * 2) Initialize a completion structure.
     */
    indio_dev->name = "mprls0025";
    indio_dev->info = &mprls_info;
    indio_dev->channels = mprls_channels;
    indio_dev->num_channels = ARRAY_SIZE(mprls_channels);
    indio_dev->modes = INDIO_DIRECT_MODE;

    ret = devm_regulator_get_enable(dev, "vdd");

    if (ret)
        return dev_err_probe(dev, ret, "Could not get and enable vdd supply.\n");

    if (dev_fwnode(dev)) {
        ret = device_property_read_u32(dev, "honeywell,pmin-pascal", &data->pmin);
        if (ret)
            return dev_err_probe(dev, ret, "honeywell,pmin-pascal property couldn't be read.\n");

        ret = device_property_read_u32(dev, "honeywell,pmax-pascal", &data->pmax);
        if (ret)
            return dev_err_probe(dev, ret, "honeywell,pmax-pascal property couldn't be read.\n");
        
        ret = device_property_read_u32(dev, "honeywell,transfer-function", &data->function);
        if (ret)
            return dev_err_probe(dev, ret, "honeywell,transfer-function could not be read.\n");

        if (data->function > MPRLS_FUNCTION_C)
            return dev_err_probe(dev, -EINVAL, "honeywell,transfer-function has invalid value: %d.\n", data->function);
        
    } else {
        /* No device firmware found. Using defaults */
        dev_notice(dev, "Device firmware wasn't found. Using defaults.\n");
        data->pmin = 0;
        data->pmax = 172369;
        data->function = MPRLS_FUNCTION_A;
    }

    data->outmin = mprls_tf_specs[data->function].output_min;
    data->outmax = mprls_tf_specs[data->function].output_max;

    scale = div_s64(((s64)(data->pmax - data->pmin) * NANO), data->outmax - data->outmin);
    data->scale = div_s64_rem(scale, NANO, &data->scale2);

    /* POSSIBLY AN ERROR WITH MATH */
    offset = ((-1LL) * data->outmin) * NANO - div_s64(div_s64((s64)data->pmin * NANO, scale), NANO);
    data->offset = div_s64_rem(offset, NANO, &data->offset2);

    /*
     * TBD:
     * 1) IRQ handling;
     * 2) GPIO reset pin handling;
     * 3) Buffer setup.
     * 
     */
     ret = devm_iio_device_register(dev, indio_dev);
     if (ret)
        return dev_err_probe(dev, ret, "Unable to register a device.\n");

    return 0;
}

static struct i2c_driver mprls_driver = {
    .probe = mprls_probe,
    .id_table = mprls_ids,
    .driver = {
        .name = "mprls0025pa",
        .of_match_table = mprls_matches,
    },
};

module_i2c_driver(mprls_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("mprl0025 iio driver");