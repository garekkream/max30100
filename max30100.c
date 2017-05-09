#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/sysfs.h>
#include <linux/errno.h>
#include <linux/slab.h>

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
    uint8_t rev;
    uint8_t part;
};

static struct max30100 *max;

ssize_t rev_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->rev = i2c_smbus_read_byte_data(max->client, MAX30100_REG_ID_REVISION);

    return sprintf(buf, "0x%2x", max->rev);
}
DEVICE_ATTR_RO(rev_id);

ssize_t part_id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->part = i2c_smbus_read_byte_data(max->client, MAX30100_REG_ID_PART);

    return sprintf(buf, "0x%2x", max->part);
}
DEVICE_ATTR_RO(part_id);

static struct attribute *attrs_info[] = {
    &dev_attr_rev_id.attr,
    &dev_attr_part_id.attr,
    NULL,
};

ssize_t interrupts_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->interrupts = i2c_smbus_read_byte_data(max->client, MAX30100_REG_INT_STATUS);

    return sprintf(buf, "0x%x", max->interrupts);
}

ssize_t interrupts_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return 0;
}
DEVICE_ATTR_RW(interrupts);

ssize_t interrupts_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->interrupts = i2c_smbus_read_byte_data(max->client, MAX30100_REG_INT_STATUS);

    return sprintf(buf, "Power ready flag:      \t %d\n\
SpO2 Data ready:       \t %d\n\
Heart Rate Data ready: \t %d\n\
Temperature Ready flag:\t %d\n\
FIFO almost full:      \t %d\n",
        max->interrupts & (1 << 0),
        max->interrupts & (1 << 4),
        max->interrupts & (1 << 5),
        max->interrupts & (1 << 6),
        max->interrupts & (1 << 7));
}
DEVICE_ATTR_RO(interrupts_dump);

static struct attribute *attrs_sensor[] = {
    &dev_attr_interrupts.attr,
    &dev_attr_interrupts_dump.attr,
    NULL,
};

static struct attribute_group sensor_group = {
    .name = "sensor",
    .attrs = attrs_sensor,
};

static struct attribute_group info_group = {
    .name = "info",
    .attrs = attrs_info,
};

static const struct attribute_group *max30100_groups[] = {
    &info_group,
    &sensor_group,
    NULL,
};

static int max30100_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;

    max = (struct max30100 *)kmalloc(sizeof(struct max30100), GFP_KERNEL);
    if (IS_ERR(max)) {
        ret = PTR_ERR(max);
        dev_err(&client->dev, "Failed to initialize memory! (errno = %d)", -ret);
        return -ret;
    }

    max->client = client;

    ret = sysfs_create_groups(&client->dev.kobj, max30100_groups);
    if(ret < 0) {
        dev_err(&client->dev, "Failed to create sysfs entires! (errno = -%d)\n", ret);
        goto cleanup_free_memory;
    }

    dev_info(&client->dev, "Probed sucessful!\n");
    return ret;

cleanup_free_memory:
    if(max) {
        kfree(max);
        max = NULL;
    }
    return -ret;
}

static int max30100_remove(struct i2c_client *client)
{
    sysfs_remove_groups(&client->dev.kobj, max30100_groups);

    if(max) {
        kfree(max);
        max = NULL;
    }

    dev_info(&client->dev, "Removed!\n");
    return 0;
}

static const struct i2c_device_id max30100_ids[] = {
        {DRV_NAME, 0},
        { }
};
MODULE_DEVICE_TABLE(i2c, max30100_ids);

static struct i2c_driver max30100_drv = {
    .probe = max30100_probe,
    .remove = max30100_remove,
    .driver = {
        .name = DRV_NAME,
        .owner = THIS_MODULE,
    },
    .id_table = max30100_ids,
};

module_i2c_driver(max30100_drv);

MODULE_AUTHOR("Krzysztof Garczynski <krzysztof.garczynski@gmail.com>");
MODULE_DESCRIPTION("Simple driver for MAX30100 Pulse Oximeter");
MODULE_LICENSE("GPL");
