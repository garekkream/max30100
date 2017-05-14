#include <linux/sysfs.h>
#include <linux/i2c.h>

#include "max30100.h"

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
    uint32_t irq;

    if(kstrtouint(buf, 10, &irq) || !irq) {
        dev_err(dev, "Invalid IRQ number\n");
        return -EINVAL;
    }

    i2c_smbus_write_byte_data(max->client, MAX30100_REG_INT_ENABLE, irq);

    return 0;
}
DEVICE_ATTR_RW(interrupts);

ssize_t interrupts_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->interrupts = i2c_smbus_read_byte_data(max->client, MAX30100_REG_INT_STATUS);

    return snprintf(buf, PAGE_SIZE, "Power ready flag:      \t %d\n\
SpO2 Data ready:       \t %d\n \
Heart Rate Data ready: \t %d\n \
Temperature Ready flag:\t %d\n \
FIFO almost full:      \t %d\n",
        max->interrupts & (1 << 0),
        max->interrupts & (1 << 4),
        max->interrupts & (1 << 5),
        max->interrupts & (1 << 6),
        max->interrupts & (1 << 7));
}
DEVICE_ATTR_RO(interrupts_dump);

ssize_t mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->mode = i2c_smbus_read_byte_data(max->client, MAX30100_REG_MODE_CONFIG);

    return snprintf(buf, PAGE_SIZE, "%d", max->mode);
}

ssize_t mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    uint32_t mode;

    if(kstrtouint(buf, 10, &mode) || !mode) {
        dev_err(dev, "Invalid Mode value\n");
        return -EINVAL;
    }

    i2c_smbus_write_byte_data(max->client, MAX30100_REG_MODE_CONFIG, mode);

    return 0;
}
DEVICE_ATTR_RW(mode);

ssize_t mode_dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->mode = i2c_smbus_read_byte_data(max->client, MAX30100_REG_MODE_CONFIG);

    return snprintf(buf, PAGE_SIZE, "Mode control:      \t %d\n \
Temperature control:       \t %d\n \
Reset control: \t %d\n \
Shutdown control:\t %d\n",
        ((max->mode & (1 << 2)) | (max->mode & (1 << 1)) | (max->mode & (1 << 0))),
        max->mode & (1 << 3),
        max->mode & (1 << 6),
        max->mode & (1 << 7));
}
DEVICE_ATTR_RO(mode_dump);

ssize_t spo2_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    max->spo2 = i2c_smbus_read_byte_data(max->client, MAX30100_REG_SPO2_CONFIG);

    return snprintf(buf, PAGE_SIZE, "%d", max->spo2);
}

ssize_t spo2_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    uint32_t spo2;

    if(kstrtouint(buf, 10, &spo2) || !spo2) {
        dev_err(dev, "Invalid Mode value\n");
        return -EINVAL;
    }

    i2c_smbus_write_byte_data(max->client, MAX30100_REG_SPO2_CONFIG, spo2);

    return 0;
}
DEVICE_ATTR_RW(spo2);

static struct attribute *attrs_sensor[] = {
    &dev_attr_interrupts.attr,
    &dev_attr_interrupts_dump.attr,
    &dev_attr_mode.attr,
    &dev_attr_mode_dump.attr,
    &dev_attr_spo2.attr,
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

int max30100_sysfs_init(struct max30100 *maxdrv)
{
    int ret;

    max = maxdrv;

    ret = sysfs_create_groups(&max->client->dev.kobj, max30100_groups);
    if(ret < 0) {
        dev_err(&max->client->dev, "Failed to create sysfs entires! (errno = -%d)\n", ret);
        return -ret;
    }

    return ret;
}

void max30100_sysfs_exit(struct max30100 *maxdrv)
{
    sysfs_remove_groups(&maxdrv->client->dev.kobj, max30100_groups);
}
