#ifndef __IIO_DRIVER_ICM20X__
#define __IIO_DRIVER_ICM20X__

struct icm20x_reg_ops {
    int (*read_reg)(const struct i2c_client *client, u8 *buf, u32 size, const u8 reg);
    int (*write_reg)(const struct i2c_client *client, u8 *buf, u32 size);
    int (*clear_bit_reg)(const struct i2c_client *client, const u8 reg_bank, const u8 reg, const u8 bit);
};

/**************************************************
* MACROS DEFINITION
**************************************************/

#define ICM20X_REG_BANK_SEL     0x7F

#define ICM20X_MASK_SEL_UB_0    0x00
#define ICM20X_MASK_SEL_UB_1    (0x01 << 4)
#define ICM20X_MASK_SEL_UB_2    (0x02 << 4)
#define ICM20X_MASK_SEL_UB_3    (0x03 << 4)

/**************************************************
* USER BANK 0
*     REGISTERS
**************************************************/

#define ICM20X_REG_USER_CTRL    0x03
#define ICM20X_REG_PWR_MGMT_1   0x06
#define ICM20X_REG_PWR_MGMT_2   0x07
#define ICM20X_REG_INT_ENABLE_2 0x12
#define ICM20X_REG_FIFO_EN_2    0x67
#define ICM20X_REG_FIFO_RST     0x68
#define ICM20X_REG_FIFO_MODE    0x69
#define ICM20X_REG_FIFO_COUNTH  0x70
#define ICM20X_REG_FIFO_COUNTL  0x71
#define ICM20X_REG_R_W          0x72
#define ICM20X_REG_ACCEL_XOUT_H 0x2D
#define ICM20X_REG_ACCEL_XOUT_L 0x2E

/**************************************************
* USER BANK 2
*     REGISTERS
**************************************************/
#define ICM20X_REG_ODR_ALIGN_EN       0x09u
#define ICM20X_REG_ACCEL_SMPLRT_DIV_1 0x10u
#define ICM20X_REG_ACCEL_SMPLRT_DIV_2 0x11u
#define ICM20X_REG_ACCEL_CONFIG       0x14u


/**************************************************
* PWR_MGMT_1 REGISTER
*     MASKS
**************************************************/
#define ICM20X_MASK_PWR_MGMT_RESET     (0x01 << 7)
#define ICM20X_MASK_PWR_MGMT_SLEEP     (0x01 << 6)
#define ICM20X_MASK_PWR_MGMT_TEMP_DIS  (0x01 << 3)
#define ICM20X_MASK_PWR_MGMT_CLKSEL    0x01u

/**************************************************
* PWR_MGMT_2 REGISTER
*     MASKS
**************************************************/
#define ICM20X_MASK_PWR_MGMT_2_DISABLE_GYRO     0x07u

/**************************************************
* USER_CTRL REGISTER
*     MASKS
**************************************************/
#define ICM20X_MASK_USER_CTRL_FIFO_EN (0x01 << 6)

#define ICM20X_MASK_INT_ENABLE_2_FIFO_OVERFLOW_EN 0x01

#define ICM20X_MASK_FIFO_EN_2_ACCEL_FIFO_EN (0x01 << 4)

#define ICM20X_MASK_FIFO_MODE_STREAM   0x00
#define ICM20X_MASK_FIFO_MODE_SNAPSHOT 0x01

#define ICM20X_MASK_ODR_ALIGN_EN_ON 0x01

#define ICM20X_MASK_ACCEL_CONFIG_ACCEL_FS_SEL_2G  0x00
#define ICM20X_MASK_ACCEL_CONFIG_ACCEL_FCHOICE_ON 0x01

#define ICM20X_MASK_FIFO_RST_ASSERT   0x01u
#define ICM20X_MASK_FIFO_RST_DEASSERT 0x00u

/**************************************************
* UTILITIES
*
**************************************************/
#define SCAN_CHAN_0 0x01
#define SCAN_CHAN_1 (0x01 << 1)
#define SCAN_CHAN_2 (0x01 << 2)

#define ICM20X_WORD_SIZE        2
#define ICM20X_FIFO_SET_SIZE    3

#endif /* __IIO_DRIVER_ICM20X__ */