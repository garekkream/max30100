#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/sysfs.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include "max30100.h"

static struct max30100 *max;

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

    ret = max30100_sysfs_init(max);
    if(ret < 0)
        goto cleanup_free_memory;

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
    max30100_sysfs_exit(max);

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
