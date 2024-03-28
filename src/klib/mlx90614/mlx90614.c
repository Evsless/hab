#include <linux/i2c.h>
#include <linux/crc8.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>

#include <linux/gpio/consumer.h>

#include <linux/iio/iio.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

/***********************************************************************************************************************
* PREPROCESSOR DEFINITION                                                                                               
***********************************************************************************************************************/

#define MLX90614_OP_RAM       0x00
#define MLX90614_OP_EEPROM    (0x01 << 5)
#define MLX90614_OP_SLEEP     0xFF
#define MLX90614_OP_SLEEP_PEC 0xE8

#define MLX90614_CFG_IIR_SHIFT  0x00
#define MLX90614_CFG_DUAL_DHIFT 0x06
#define MLX90614_CFG_FIR_SHIT   0x08

#define MLX90614_RAM_AMBIENT    0x06
#define MLX90614_RAM_OBJ1       0x07

#define MLX90614_CONST_SCALE_MICRO       20000
#define MLX90614_CONST_OFFSET_MICRO      273150000
#define MLX90614_CONST_PM_SLEEP_DELAY_MS 2000
#define MLX90614_CONST_SDA_LOW_TIME_MS   35
#define MLX90614_CONST_WAKEUP_TIME_MS    250

/***********************************************************************************************************************
* STRUCT PROTOTYPES                                                                                               
***********************************************************************************************************************/

struct mlx90614_fields {
    u16 t_ambient;
    u16 t_obj;
    s64 ts;
};

struct mlx90614_data {
    struct i2c_client *client;
    struct mutex lock;
    struct gpio_desc *wakeup_gpio;

    struct mlx90614_fields vals;
};

enum mlx90614_ps {
    MLX90614_PS_SLEEP = 0,
    MLX90614_PS_ACTIVE,
};

/***********************************************************************************************************************
* INTERNAL FUNCTION DEFINITIONS                                                                                               
***********************************************************************************************************************/
static int mlx90614_probe(struct i2c_client *client);
static void mlx90614_remove(struct i2c_client *client);
static int mlx90614_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, 
                                int *val, int *val2, long mask);

static int mlx90614_rtm_suspend(struct device *dev);
static int mlx90614_rtm_resume(struct device *dev);

static irqreturn_t mlx90614_trigger_handler(int irq, void *p);

static int mlx90614_read_temp(struct mlx90614_data *data);
static int mlx90614_set_ps(struct mlx90614_data *data, enum mlx90614_ps);
static int mlx90614_sleep(struct mlx90614_data *data);
static void mlx90614_wakeup(struct mlx90614_data *data);

/***********************************************************************************************************************
* GLOBAL DATA DECLARATION                                                                                               
***********************************************************************************************************************/
static const struct iio_chan_spec mlx90614_channels[] = {
    {
        .type = IIO_TEMP,
        .modified = 1,
        .channel2 = IIO_MOD_TEMP_AMBIENT,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_OFFSET) | BIT(IIO_CHAN_INFO_SCALE),
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
        .modified = 1,
        .channel2 = IIO_MOD_TEMP_OBJECT,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW), /* Possibly emissivity + lpf cutoff */
        .info_mask_shared_by_type = BIT(IIO_CHAN_INFO_OFFSET) | BIT(IIO_CHAN_INFO_SCALE),
        .scan_index = 1,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_CPU,
        },
    },
    IIO_CHAN_SOFT_TIMESTAMP(2),
};

static const struct  dev_pm_ops mlx90614_pm_ops = {
    SET_RUNTIME_PM_OPS(mlx90614_rtm_suspend, mlx90614_rtm_resume, NULL)
};

static const struct i2c_device_id mlx90614_ids[] = {
    { "mlx90614" },
    { }
};
MODULE_DEVICE_TABLE(i2c, mlx90614_ids);

static const struct of_device_id mlx90614_matches[] = {
    { .compatible = "melexis,mlx90614" },
    { }
};
MODULE_DEVICE_TABLE(of, mlx90614_matches);

static const struct iio_info mlx90614_info = {
    .read_raw = mlx90614_read_raw,
};

static struct i2c_driver mlx90614_driver = {
    .probe = mlx90614_probe,
    .remove = mlx90614_remove,
    .id_table = mlx90614_ids,
    .driver = {
        .name           = "mlx90614",
        .of_match_table = mlx90614_matches,
        .pm             = pm_ptr(&mlx90614_pm_ops),
    },
};

module_i2c_driver(mlx90614_driver);

/***********************************************************************************************************************
* INTERNAL FUNCTION DECLARATIONS                                                                                               
***********************************************************************************************************************/
static int mlx90614_probe(struct i2c_client *client) {
    int ret = 0;
    struct mlx90614_data *data;
    struct iio_dev       *indio_dev;
    struct device        *dev = &client->dev;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_READ_BYTE))
        return dev_err_probe(dev, -EOPNOTSUPP, "i2c is not supported.\n");

    indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
    if (!indio_dev)
        return dev_err_probe(dev, -ENOMEM, "could not create iio device.\n");
    
    data = iio_priv(indio_dev);
    i2c_set_clientdata(client, indio_dev);

    data->client = client;
    mutex_init(&data->lock);
    
    /* TBD: Simplify error checking here. */
    data->wakeup_gpio = devm_gpiod_get_optional(&client->dev, "wakeup", GPIOD_IN | GPIOD_FLAGS_BIT_NONEXCLUSIVE);
    if (IS_ERR(data->wakeup_gpio))
        return dev_err_probe(&client->dev, PTR_ERR(data->wakeup_gpio), "error acquiring given gpio");
    if (!data->wakeup_gpio)
        return dev_err_probe(&client->dev, -EOPNOTSUPP, "given gpio was not found");

    indio_dev->name         = "mlx90614";
    indio_dev->info         = &mlx90614_info;
    indio_dev->channels     = mlx90614_channels;
    indio_dev->num_channels = ARRAY_SIZE(mlx90614_channels);
    indio_dev->modes        = INDIO_DIRECT_MODE;

    mlx90614_wakeup(data);

    ret = devm_iio_triggered_buffer_setup(dev, indio_dev, NULL, mlx90614_trigger_handler, NULL);
    if (ret)
        return dev_err_probe(dev, ret, "iio triggered buffer setup failed\n");
    
    ret = pm_runtime_set_active(&client->dev);
    if (ret)
        return ret;
    pm_runtime_set_autosuspend_delay(&client->dev, MLX90614_CONST_PM_SLEEP_DELAY_MS);
	pm_runtime_use_autosuspend(&client->dev);
	pm_runtime_enable(&client->dev);
    
    ret = iio_device_register(indio_dev);
    if (ret)
        return dev_err_probe(dev, ret, "unable to register iio device.\n");
    
    return 0;
}

static void mlx90614_remove(struct i2c_client *client) {
    struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct mlx90614_data *data = iio_priv(indio_dev);

    iio_device_unregister(indio_dev);

    if (data->wakeup_gpio) {
        pm_runtime_disable(&client->dev);
        if (!pm_runtime_status_suspended(&client->dev))
			mlx90614_sleep(data);
		pm_runtime_set_suspended(&client->dev);
        gpiod_put(data->wakeup_gpio);
    }
}

static int mlx90614_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, 
                                int *val, int *val2, long mask)
{
    int ret = 0;
    struct mlx90614_data *data = iio_priv(indio_dev);

    switch (mask) {
        case IIO_CHAN_INFO_RAW:
            mutex_lock(&data->lock);
            ret = mlx90614_read_temp(data);
            mutex_unlock(&data->lock);

            if (ret)
                return ret;
        
            if (IIO_MOD_TEMP_AMBIENT == chan->channel2)
                *val = data->vals.t_ambient;
            else
                *val = data->vals.t_obj;
            return IIO_VAL_INT;

        case IIO_CHAN_INFO_SCALE:
            *val = 0;
            *val2 = MLX90614_CONST_SCALE_MICRO;
            return IIO_VAL_INT_PLUS_MICRO;

        case IIO_CHAN_INFO_OFFSET:
            *val  = (-1LL) * MLX90614_CONST_OFFSET_MICRO;
            *val2 = MLX90614_CONST_SCALE_MICRO;
            return IIO_VAL_FRACTIONAL;

        default:
            return -EINVAL;
    }
}

static int mlx90614_rtm_resume(struct device *dev) {
    struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
    struct mlx90614_data *data = iio_priv(indio_dev);

    mlx90614_wakeup(data);
    
    return 0;
}

static int mlx90614_rtm_suspend(struct device *dev) {
    int ret;
    struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
    struct mlx90614_data *data = iio_priv(indio_dev);

    ret = mlx90614_sleep(data);

    return ret;
}

static int mlx90614_read_temp(struct mlx90614_data *data) {
    int ret;
    ret = mlx90614_set_ps(data, MLX90614_PS_ACTIVE);
    if (ret)
        return ret;

    ret = i2c_smbus_read_word_data(data->client, MLX90614_OP_RAM | MLX90614_RAM_AMBIENT);
    if (ret < 0)
        return ret;
    data->vals.t_ambient = ret;

    ret = i2c_smbus_read_word_data(data->client, MLX90614_OP_RAM | MLX90614_RAM_OBJ1);
    if (ret < 0)
        return ret;
    data->vals.t_obj = ret;

    ret = mlx90614_set_ps(data, MLX90614_PS_SLEEP);
    if (ret)
        return ret;

    return 0;
}

static int mlx90614_set_ps(struct mlx90614_data *data, enum mlx90614_ps ps) {
    int ret = 0;
    struct device *dev = &data->client->dev;

    if (MLX90614_PS_ACTIVE == ps) {
        ret = pm_runtime_resume_and_get(dev);
    } else {
        pm_runtime_mark_last_busy(dev);
		ret = pm_runtime_put_autosuspend(dev);
    }

    return ret < 0 ? ret : 0;
}

static irqreturn_t mlx90614_trigger_handler(int irq, void *p) {
    int ret = 0;
    struct iio_poll_func *pf = p;
    struct iio_dev *indio_dev = pf->indio_dev;
    struct mlx90614_data *data = iio_priv(indio_dev);

    mutex_lock(&data->lock);
    ret = mlx90614_read_temp(data);
    if (ret)
        goto err;

    iio_push_to_buffers_with_timestamp(indio_dev, &data->vals, iio_get_time_ns(indio_dev));

err:
    mutex_unlock(&data->lock);
    iio_trigger_notify_done(indio_dev->trig);
    return IRQ_HANDLED;
}

static int mlx90614_sleep(struct mlx90614_data *data) {
    int ret;

    mutex_lock(&data->lock);
    ret = i2c_smbus_xfer(data->client->adapter, data->client->addr,
			     data->client->flags | I2C_CLIENT_PEC,
			     I2C_SMBUS_WRITE, MLX90614_OP_SLEEP,
			     I2C_SMBUS_BYTE, NULL);
    mutex_unlock(&data->lock);

    return ret;
}

static void mlx90614_wakeup(struct mlx90614_data *data) {

    i2c_lock_bus(data->client->adapter, I2C_LOCK_ROOT_ADAPTER);
    gpiod_direction_output(data->wakeup_gpio, 0);

    msleep(MLX90614_CONST_SDA_LOW_TIME_MS);

    gpiod_direction_input(data->wakeup_gpio);
    i2c_unlock_bus(data->client->adapter, I2C_LOCK_ROOT_ADAPTER);

    msleep(MLX90614_CONST_WAKEUP_TIME_MS);

    /* Quirk */
    i2c_smbus_read_word_data(data->client, MLX90614_OP_RAM | MLX90614_RAM_AMBIENT);
}

/***********************************************************************************************************************
* MODULE SPECIFIC INFO                               
***********************************************************************************************************************/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yahor Yauseyenka");
MODULE_DESCRIPTION("mlx90614 iio driver");