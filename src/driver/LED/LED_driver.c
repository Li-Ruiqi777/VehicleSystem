#include "linux/printk.h"
#include <asm/uaccess.h>

#include <linux/init.h>
#include <linux/kern_levels.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
//内核空间必须用这个ioctl.h
#include <linux/ioctl.h> 
#include <linux/of_gpio.h>

#define LED_MAGIC 'L'
#define LED_ON    _IO(LED_MAGIC, 0)
#define LED_OFF   _IO(LED_MAGIC, 1)

// 设备驱动的信息
struct LED_device
{
    dev_t devid;                          // 设备号
    int major;                            // 主设备号
    int minor;                            // 次设备号
    struct cdev cdev;                     // 字符设备
    struct class *class;                  // 设备类
    struct device *device;                // 设备实例
    struct device_node *device_tree_node; // 设备数中的节点
    int gpio_index;                       // led对应gpio的序号
};

static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct LED_device *led_device = container_of(file->f_inode->i_cdev, struct LED_device, cdev);
    switch (cmd)
    {
    case LED_ON:
        pr_info("LED ON\n");
        gpio_set_value(led_device->gpio_index, 0);
        break;
    case LED_OFF:
        pr_info("LED OFF\n");
        gpio_set_value(led_device->gpio_index, 1);
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = led_ioctl,
};

void led_gpio_init(struct platform_device *pdev)
{
    struct LED_device *led_device = platform_get_drvdata(pdev);

    // 获取设备树
    led_device->device_tree_node = pdev->dev.of_node;
    if (led_device->device_tree_node == NULL)
    {
        printk("gpioled node nost find!\r\n");
        return;
    }

    // 从设备树中获取GPIO编号
    led_device->gpio_index = of_get_named_gpio(led_device->device_tree_node, "led-gpio", 0);
    if (led_device->gpio_index < 0)
    {
        printk("can't get led-gpio! \r\n");
        return;
    }
    printk("LED's GPIO Index = %d\r\n", led_device->gpio_index);

    // 申请并配置GPIO
    gpio_request(led_device->gpio_index, "led0");
    gpio_direction_output(led_device->gpio_index, 1);
}

int led_probe(struct platform_device *pdev)
{
    struct LED_device *led_device = devm_kzalloc(&pdev->dev, sizeof(struct LED_device), GFP_KERNEL);
    // 将设备信息保存到platform_device的上下文中, 以便在其他地方能够访问到led_device
    platform_set_drvdata(pdev, led_device);

    led_gpio_init(pdev);

    // 动态分配设备号
    if (led_device->major) // 已经定义了设备号
    {
        led_device->devid = MKDEV(led_device->major, 0);
        register_chrdev_region(led_device->devid, 1, "gpio_led");
    }
    else // 未定义设备号
    {
        alloc_chrdev_region(&led_device->devid, 0, 1, "gpio_led"); // 申请设备号
        led_device->major = MAJOR(led_device->devid);              // 获取分配号的主设备号
        led_device->minor = MINOR(led_device->devid);              // 获取分配号的次设备号
    }
    printk("gpioled major = %d, minor = %d\r\n", led_device->major, led_device->minor);

    // 注册字符设备
    led_device->cdev.owner = THIS_MODULE;
    cdev_init(&led_device->cdev, &fops);
    int ret = cdev_add(&led_device->cdev, led_device->devid, 1); // 将字符设备和设备号关联, 并插入内核
    if (ret)
    {
        printk(KERN_NOTICE "Error %d adding gpio_led", ret);
        unregister_chrdev_region(led_device->devid, 1);
        return ret;
    }

    // 在/sys/class下创建设备类
    led_device->class = class_create(THIS_MODULE, "gpio_led");
    if (IS_ERR(led_device->class))
    {
        return PTR_ERR(led_device->class);
    }

    // 在/dev下创建设备节点,并关联到设备类
    led_device->device = device_create(led_device->class, NULL, led_device->devid, NULL, "led");
    if (IS_ERR(led_device->device))
    {
        return PTR_ERR(led_device->device);
    }

    printk(KERN_INFO "gpio_led module loaded \n");

    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    struct LED_device *led_device = platform_get_drvdata(pdev);

    device_destroy(led_device->class, led_device->devid);
    class_destroy(led_device->class);
    cdev_del(&led_device->cdev);
    unregister_chrdev_region(led_device->devid, 1);

    printk(KERN_INFO "gpio_led module unloaded \n");
}

static const struct of_device_id led_of_match[] = {{.compatible = "gpio-led"}, {}};

static struct platform_driver led_driver = {
    .driver =
        {
            .name = "imx6ull-led",
            .of_match_table = led_of_match,
        },
    .probe = led_probe,
    .remove = led_remove,
};

static int __init led_module_init(void)
{
    return platform_driver_register(&led_driver);
}

static void __exit led_module_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(led_module_init);
module_exit(led_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LRQ");
MODULE_DESCRIPTION("LED Driver");
MODULE_VERSION("1.0");
