#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

struct dht11_dev
{
    int gpio;
    int major;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int us_array[40];
    int time_array[40];
    int us_index;
};

static struct dht11_dev *dht11_dev_data;

void dht11_reset(void)
{
    gpio_direction_output(dht11_dev_data->gpio, 1);
}

void dht11_start(void)
{
    mdelay(30);
    gpio_set_value(dht11_dev_data->gpio, 0);
    mdelay(20);
    gpio_set_value(dht11_dev_data->gpio, 1);
    udelay(40);
    gpio_direction_input(dht11_dev_data->gpio);
    udelay(2);
}

static int dht11_wait_for_ready(void)
{
    int timeout = 200;

    while (gpio_get_value(dht11_dev_data->gpio) && --timeout)
        udelay(1);
    if (!timeout)
        return -1;

    while (!gpio_get_value(dht11_dev_data->gpio) && --timeout)
        udelay(1);
    if (!timeout)
        return -1;

    while (gpio_get_value(dht11_dev_data->gpio) && --timeout)
        udelay(1);
    if (!timeout)
        return -1;

    return 0;
}

static int dht11_read_byte(unsigned char *buf)
{
    int i;
    unsigned char data = 0;
    int timeout_us = 200;
    u64 pre, last;
    int ns;

    for (i = 0; i < 8; i++)
    {
        timeout_us = 400;
        while (!gpio_get_value(dht11_dev_data->gpio) && --timeout_us)
            udelay(1);
        if (!timeout_us)
            return -1;

        timeout_us = 20000000;

        pre = ktime_get_boot_ns();
        while (gpio_get_value(dht11_dev_data->gpio) && --timeout_us)
            ;

        last = ktime_get_boot_ns();
        ns = last - pre;
        if (!timeout_us)
            return -1;

        dht11_dev_data->us_array[dht11_dev_data->us_index] = ns;
        dht11_dev_data->time_array[dht11_dev_data->us_index++] = 20000000 - timeout_us;
        data = (data << 1) | (ns > 40000 ? 1 : 0);
    }

    *buf = data;
    return 0;
}

static int dht11_open(struct inode *inode, struct file *file)
{
    file->private_data = dht11_dev_data;
    return 0;
}

static ssize_t dht11_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
    unsigned long flags;
    int i, err;
    unsigned char data[5];
    struct dht11_dev *dev = (struct dht11_dev *)file->private_data;

    dev->us_index = 0;

    if (size != 4)
        return -EINVAL;

    local_irq_save(flags);

    dht11_reset();
    dht11_start();

    if (dht11_wait_for_ready())
    {
        local_irq_restore(flags);
        dev_err(dev->device, "device not ready\n");
        return -EAGAIN;
    }

    for (i = 0; i < 5; i++)
    {
        if (dht11_read_byte(&data[i]))
        {
            local_irq_restore(flags);
            dev_err(dev->device, "read byte timeout\n");
            return -EAGAIN;
        }
    }

    dht11_reset();
    local_irq_restore(flags);

    if (data[4] != (data[0] + data[1] + data[2] + data[3]))
    {
        dev_err(dev->device, "checksum error\n");
        return -EIO;
    }

    err = copy_to_user(buf, data, 4);
    return 4;
}

static struct file_operations gpio_key_drv = {
    .owner = THIS_MODULE,
    .open  = dht11_open,
    .read  = dht11_read,
};

static int gpio_drv_probe(struct platform_device *pdev)
{
    int err = 0;
    struct device_node *np = pdev->dev.of_node;
    struct dht11_dev *dev;

    dev_info(&pdev->dev, "probe\n");

    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dht11_dev_data = dev;

    if (np)
    {
        dev->gpio = of_get_gpio(np, 0);
        if (dev->gpio < 0)
        {
            dev_err(&pdev->dev, "get gpio from dts failed\n");
            kfree(dev);
            return dev->gpio;
        }
    }
    else
    {
        struct resource *res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
        if (!res)
        {
            dev_err(&pdev->dev, "get resource failed\n");
            kfree(dev);
            return -EINVAL;
        }
        dev->gpio = res->start;
    }

    err = gpio_request(dev->gpio, "dht11");
    if (err)
    {
        dev_err(&pdev->dev, "gpio_request failed %d\n", dev->gpio);
        kfree(dev);
        return err;
    }
    gpio_direction_output(dev->gpio, 1);

    dev->major = register_chrdev(0, "dht11", &gpio_key_drv);
    if (dev->major < 0)
    {
        dev_err(&pdev->dev, "register_chrdev failed %d\n", dev->major);
        gpio_free(dev->gpio);
        kfree(dev);
        return dev->major;
    }

    dev->class = class_create(THIS_MODULE, "dht11_class");
    if (IS_ERR(dev->class))
    {
        dev_err(&pdev->dev, "class_create failed\n");
        gpio_free(dev->gpio);
        unregister_chrdev(dev->major, "dht11");
        kfree(dev);
        return PTR_ERR(dev->class);
    }

    dev->device = device_create(dev->class, NULL, MKDEV(dev->major, 0), NULL, "dht11");
    if (!dev->device)
    {
        dev_err(&pdev->dev, "device_create failed\n");
        class_destroy(dev->class);
        unregister_chrdev(dev->major, "dht11");
        gpio_free(dev->gpio);
        kfree(dev);
        return -EINVAL;
    }

    platform_set_drvdata(pdev, dev);
    dev_info(&pdev->dev, "dht11 device registered\n");

    return 0;
}

static int gpio_drv_remove(struct platform_device *pdev)
{
    struct dht11_dev *dev = platform_get_drvdata(pdev);

    dev_info(&pdev->dev, "remove\n");

    device_destroy(dev->class, MKDEV(dev->major, 0));
    class_destroy(dev->class);
    unregister_chrdev(dev->major, "dht11");
    gpio_free(dev->gpio);
    kfree(dev);

    return 0;
}

static const struct of_device_id gpio_dt_ids[] = {
    {
        .compatible = "alientek,dht11",
    },
};

static struct platform_driver gpio_platform_driver = {
    .driver =
        {
            .name = "dht11",
            .of_match_table = gpio_dt_ids,
        },
    .probe = gpio_drv_probe,
    .remove = gpio_drv_remove,
};

static int __init dht11_drv_init(void)
{
    return platform_driver_register(&gpio_platform_driver);
}

static void __exit dht11_drv_exit(void)
{
    platform_driver_unregister(&gpio_platform_driver);
}

module_init(dht11_drv_init);
module_exit(dht11_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LRQ");
MODULE_DESCRIPTION("DHT11 Driver");
MODULE_VERSION("1.0");
