#include "asm/uaccess.h"

#include "linux/init.h"
#include "linux/kern_levels.h"
#include "linux/module.h"

#include "linux/cdev.h"
#include "linux/fs.h"
#include "linux/platform_device.h"

#include "linux/gpio.h"
#include "linux/of_gpio.h"

#define LEDOFF 0
#define LEDON  1

struct LED_dev
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

static struct LED_dev led_dev;

void led_gpio_init(struct platform_device *pdev);
static int led_open(struct inode *inode, struct file *file);
static int led_release(struct inode *inode, struct file *file);
static ssize_t led_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
static int led_probe(struct platform_device *dev);
static int led_remove(struct platform_device *dev);
static int led_module_init(void);
static void led_module_exit(void);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write,
};

static int led_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "LED opened\n");
    file->private_data = &led_dev;
    return 0;
}

static int led_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "LED closed\n");
    return 0;
}

static ssize_t led_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (buf == NULL)
        return -EINVAL;

    int ret = 0;
    uint8_t databuf[1];
    ret = __copy_from_user(databuf, buf, count);
    struct LED_dev *dev = file->private_data;

    if (ret < 0)
    {
        printk(KERN_ERR "write LED status failed! \n");
        return EFAULT;
    }

    if (databuf[0] == LEDON)
        gpio_set_value(dev->gpio_index, 0);

    else
        gpio_set_value(dev->gpio_index, 1);
}

void led_gpio_init(struct platform_device *pdev)
{
    // 获取设备树
    led_dev.device_tree_node = pdev->dev.of_node;
    if (led_dev.device_tree_node == NULL)
    {
        printk("gpioled node nost find!\r\n");
        return;
    }

    // 从设备树中获取GPIO编号
    led_dev.gpio_index = of_get_named_gpio(led_dev.device_tree_node, "led-gpio", 0);
    if (led_dev.gpio_index < 0)
    {
        printk("can't get led-gpio! \r\n");
        return;
    }
    printk("LED's GPIO Index = %d\r\n", led_dev.gpio_index);

    // 申请并配置GPIO
    gpio_request(led_dev.gpio_index, "led0");
    gpio_direction_output(led_dev.gpio_index, 1);
}

int led_probe(struct platform_device *pdev)
{
    led_gpio_init(pdev);

    // 动态分配设备号
    if (led_dev.major)
    {
        led_dev.devid = MKDEV(led_dev.major, 0);
        register_chrdev_region(led_dev.devid, 1, "gpio_led");
    }
    else
    {                                                          /* 没有定义设备号 */
        alloc_chrdev_region(&led_dev.devid, 0, 1, "gpio_led"); /* 申请设备号 */
        led_dev.major = MAJOR(led_dev.devid);                  /* 获取分配号的主设备号 */
        led_dev.minor = MINOR(led_dev.devid);                  /* 获取分配号的次设备号 */
    }
    printk("gpioled major=%d,minor=%d\r\n", led_dev.major, led_dev.minor);

    // 注册字符设备
    led_dev.cdev.owner = THIS_MODULE;
    cdev_init(&led_dev.cdev, &fops);
    int ret = cdev_add(&led_dev.cdev, led_dev.devid, 1);
    if (ret)
    {
        printk(KERN_NOTICE "Error %d adding gpio_led", ret);
        unregister_chrdev_region(led_dev.devid, 1);
        return ret;
    }

    // 创建设备类
    led_dev.class = class_create(THIS_MODULE, "gpio_led");
    if (IS_ERR(led_dev.class))
    {
        return PTR_ERR(led_dev.class);
    }

    // 创建设备节点
    led_dev.device = device_create(led_dev.class, NULL, led_dev.devid, NULL, "led");
    if (IS_ERR(led_dev.device))
    {
        return PTR_ERR(led_dev.device);
    }

    printk(KERN_INFO "gpio_led module loaded \n");

    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    device_destroy(led_dev.class, led_dev.devid);
    class_destroy(led_dev.class);
    cdev_del(&led_dev.cdev);
    devm_gpio_free(&pdev->dev, led_dev.gpio_index);
    unregister_chrdev_region(led_dev.devid, 1);

    printk(KERN_INFO "gpio_led module unloaded \n");
}

static const struct of_device_id led_of_match[] = {{.compatible = "gpio-led"}, {}};

static struct platform_driver led_driver = {
    .driver =
        {
            .name = "imx6ul-led",
            .of_match_table = led_of_match,
        },
    .probe = led_probe,
    .remove = led_remove,
};

static int led_module_init(void)
{
    return platform_driver_register(&led_driver);
}

static void led_module_exit(void)
{
    platform_driver_unregister(&led_driver);
}

module_init(led_module_init);
module_exit(led_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LRQ");
MODULE_DESCRIPTION("LED Driver");
MODULE_VERSION("1.0");
