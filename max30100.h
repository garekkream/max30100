#ifndef MAX30100_H
#define MAX30100_H

#define DRV_NAME    "max30100"

#define MAX30100_REG_INT_STATUS             0x00
#define MAX30100_REG_INT_ENABLE             0x01
#define MAX30100_REG_FIFO_WRITE_PTR         0x02
#define MAX30100_REG_OVERFLOW_CNT           0x03
#define MAX30100_REG_FIFO_READ_PTR          0x04
#define MAX30100_REG_FIFO_DATA              0x05
#define MAX30100_REG_MODE_CONFIG            0x06
#define MAX30100_REG_SPO2_CONFIG            0x07
#define MAX30100_REG_LED_CONFIG             0x09
#define MAX30100_REG_TEMP_INTEGER           0x16
#define MAX30100_REG_TEMP_FRACTION          0x17
#define MAX30100_REG_ID_REVISION            0xFE
#define MAX30100_REG_ID_PART                0xFF

struct max30100 {
    struct i2c_client *client;
    uint8_t interrupts;
    uint8_t mode;
    uint8_t spo2;
    uint8_t rev;
    uint8_t part;
};

int max30100_sysfs_init(struct max30100 *);
void max30100_sysfs_exit(struct max30100 *);

#endif //MAX30100_H
