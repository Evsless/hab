#include <linux/i2c.h>
#include <linux/crc8.h>
#include <linux/delay.h>
#include <linux/units.h>
#include <linux/math64.h>
#include <linux/module.h>

#include <linux/iio/iio.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

#define SHT4X_RES_H_THR ((1 << 16) - 1)
#define SHT4X_CRC8_POLY  0x31
#define SHT4X_CRC8_INIT  0xFF
#define SHT4X_WORD_LEN   2

struct sht4x_chan {
    s16 humid;
    s16 temp;
    s64 ts;
};

struct scaling_data {
    int scale;
    int scale2;

    int offset;
    int offset2;
};

struct sht4x_data {
    struct mutex lock;
    struct i2c_client *client;

    struct scaling_data t_sdata;
    struct scaling_data h_sdata;

    struct sht4x_chan chan;

    u8 crc_table[CRC8_TABLE_SIZE];
};

static const struct iio_chan_spec sht4x_channels[] = {
    {
        .type = IIO_HUMIDITYRELATIVE,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
                                BIT(IIO_CHAN_INFO_SCALE) |
                                BIT(IIO_CHAN_INFO_OFFSET),
        .scan_index = 0,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_CPU,
        },
    },
    {
        .type = IIO_TEMP,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
                                BIT(IIO_CHAN_INFO_SCALE) |
                                BIT(IIO_CHAN_INFO_OFFSET),
        .scan_index = 1,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_CPU,
        },
    },
    IIO_CHAN_SOFT_TIMESTAMP(2)
};

static const struct i2c_device_id sht4x_ids[] = {
    { "sht4x" },
    { }
};
MODULE_DEVICE_TABLE(i2c, sht4x_ids);

static const struct of_device_id sht4x_matches[] = {
    { .compatible = "sensirion,sht4x" },
    { }
};
MODULE_DEVICE_TABLE(of, sht4x_matches);

static int sht4x_read_all(struct sht4x_data *data) {
    struct device *dev = &data->client->dev;
    int ret = 0;
    u8 crc = 0;
    u8 w_buff[] = {0xFD};
    u8 r_buff[6];

    ret = i2c_master_send(data->client, w_buff, sizeof(w_buff));
    if (ret < 0) {
        dev_err(dev, "error when sending data, code: %d.\n", ret);
        return ret;
    }
    if (sizeof(w_buff) != ret) {
        dev_err(dev, "message wasn't send completely, send: %d, expected: %u.\n", 
                    ret, (u32)sizeof(w_buff));
        return -EIO;
    }

    usleep_range(6900, 8300);
    ret = i2c_master_recv(data->client, r_buff, sizeof(r_buff));
    if (ret < 0) {
        dev_err(dev, "error when receiving data, code: %d.\n", ret);
        return ret;
    }
    if (sizeof(r_buff) != ret) {
        dev_err(dev, "received data not complete, send: %d, expected: %u.\n",
                    ret, (u32)sizeof(r_buff));
        return -EIO;
    }

    data->chan.temp  = (r_buff[0] << 8) | r_buff[1];
    data->chan.humid = (r_buff[3] << 8) | r_buff[4];

    crc = crc8(data->crc_table, &r_buff[0], SHT4X_WORD_LEN, SHT4X_CRC8_INIT);
    if (crc != r_buff[2]) {
        dev_err(dev, "data integrity check failure.\n");
        return -EIO;
    }

    crc = crc8(data->crc_table, &r_buff[3], SHT4X_WORD_LEN, SHT4X_CRC8_INIT);
    if (crc != r_buff[5]) {
        dev_err(dev, "data integrity check failure.\n");
        return -EIO;
    }

    /**
     * TBD
     * 1) dev debug message - nice to have.
    */

    return 0;
}

static int sht4x_read_raw(struct iio_dev *indio_dev, 
        struct iio_chan_spec const *chan, int *val, int *val2, long mask) {

    int ret;
    s32 raw_val;
    struct scaling_data sd;
    struct sht4x_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);
    ret = sht4x_read_all(data);
    mutex_unlock(&data->lock);

    if (ret < 0)
        return ret;

    if (IIO_TEMP == chan->type) {
        raw_val = data->chan.temp;
        sd = data->t_sdata;
    } else {
        raw_val = data->chan.humid;
        sd = data->h_sdata;
    }
    
    switch (mask) {
        case IIO_CHAN_INFO_RAW:
            *val = raw_val;
            return IIO_VAL_INT;
        case IIO_CHAN_INFO_SCALE:
            *val = sd.scale;
            *val2 = sd.scale2;
            return IIO_VAL_INT_PLUS_NANO;
        case IIO_CHAN_INFO_OFFSET:
            *val = sd.offset;
            *val2 = sd.offset2;
            return IIO_VAL_INT_PLUS_NANO;
        default:
            return -EINVAL;
    }
}

static irqreturn_t sht4x_trigger_handler(int irq, void *p) {
    int ret = 0;
    struct iio_poll_func *pf = p;
    struct iio_dev *indio_dev = pf->indio_dev;
    struct sht4x_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);
    ret = sht4x_read_all(data);
    if (ret < 0)
        goto release;

    iio_push_to_buffers_with_timestamp(indio_dev, &data->chan, iio_get_time_ns(indio_dev));

release:
    mutex_unlock(&data->lock);
    iio_trigger_notify_done(indio_dev->trig);

    return IRQ_HANDLED;
}

static const struct iio_info sht4x_info = {
    .read_raw = sht4x_read_raw,
};

static int sht4x_probe(struct i2c_client *client) {
    int ret;
    s64 scale, offset;
    struct sht4x_data *data;
    struct iio_dev *indio_dev;
    struct device *dev = &client->dev;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
        dev_err_probe(dev, -EOPNOTSUPP, "i2c not supported.\n");
    
    indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
    if (!indio_dev)
        dev_err_probe(dev, -ENOMEM, "couldn't create iio device.\n");
    
    data = iio_priv(indio_dev);

    data->client = client;
    mutex_init(&data->lock);
    crc8_populate_msb(data->crc_table, SHT4X_CRC8_POLY);

    indio_dev->name = "sht4x";
    indio_dev->info = &sht4x_info;
    indio_dev->channels = sht4x_channels;
    indio_dev->num_channels = ARRAY_SIZE(sht4x_channels);
    indio_dev->modes = INDIO_DIRECT_MODE;

    /* Setup humidity scaling */
    scale = div_s64((s64)125 * NANO, (s64)SHT4X_RES_H_THR);
    data->h_sdata.scale = div_s64_rem(scale, NANO, &data->h_sdata.scale2);

    offset = div_s64(((-1LL) * (s64)6) * NANO, scale) * NANO;
    data->h_sdata.offset = div_s64_rem(offset, NANO, &data->h_sdata.offset2);

    /* Setup temperature scaling */
    scale = div_s64((s64)175 * NANO, (s64)SHT4X_RES_H_THR);
    data->t_sdata.scale = div_s64_rem(scale, NANO, &data->t_sdata.scale2);

    offset = div_s64(((-1LL) * (s64)45) * NANO, scale) * NANO;
    data->t_sdata.offset = div_s64_rem(offset, NANO, &data->t_sdata.offset2);

    /**
     * TBD
     * 1) Setup triggered buffer;
     * 2) Run a reset before registering a device;
    */

    ret = devm_iio_triggered_buffer_setup(dev, indio_dev, NULL, sht4x_trigger_handler, NULL);
    if (ret)
        dev_err_probe(dev, ret, "iio triggered buffer setup failed.\n");

    ret = devm_iio_device_register(dev, indio_dev);
    if (ret)
        return dev_err_probe(dev, ret, "unable to register iio device.\n");
    return 0;
}

static struct i2c_driver sht4x_driver = {
    .probe = sht4x_probe,
    .id_table = sht4x_ids,
    .driver = {
        .name = "sht4x",
        .of_match_table = sht4x_matches,
    },
};

module_i2c_driver(sht4x_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("sht4x iio driver");