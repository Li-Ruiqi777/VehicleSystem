#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "asm-generic/gpio.h"
#include "asm/gpio.h"
#include <linux/of_gpio.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

#define DRV_NAME        "gpio_key_platform"
#define KEY_DEBOUNCE_MS 10 // 去抖动时间

struct key_device
{
    struct input_dev *input;
    int gpio_index;
    int irq;
    struct timer_list debounce_timer;
};

static irqreturn_t key_interrupt(int irq, void *dev_id)
{
    struct key_device *dev = dev_id;

    // 启动去抖动定时器
    mod_timer(&dev->debounce_timer, jiffies + msecs_to_jiffies(KEY_DEBOUNCE_MS));

    return IRQ_HANDLED;
}

static void key_debounce_handler(unsigned long data)
{
    struct key_device *dev = (struct key_device *)data;
    int state = gpio_get_value(dev->gpio_index);

    input_report_key(dev->input, KEY_0, !state);
    input_sync(dev->input);
    printk(KERN_INFO "KEY_0: %d\n\r", state);
}

static int key_probe(struct platform_device *pdev)
{
    struct key_device *dev;
    struct input_dev *input;
    int ret;

    /* 1. 分配设备结构体 */
    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    /* 2. 获取GPIO */
    dev->gpio_index = of_get_named_gpio(pdev->dev.of_node, "key-gpio", 0);
    if (dev->gpio_index < 0)
    {
        dev_err(&pdev->dev, "Failed to get GPIO\n");
        return dev->gpio_index;
    }

    /* 3. 申请中断 */
    dev->irq = gpio_to_irq(dev->gpio_index);
    ret = devm_request_irq(&pdev->dev, dev->irq, key_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                           DRV_NAME, dev);
    if (ret)
    {
        dev_err(&pdev->dev, "Failed to request IRQ %d\n", dev->irq);
        goto err_irq;
    }

    /* 4. 初始化输入设备 */
    input = input_allocate_device();
    if (!input)
    {
        ret = -ENOMEM;
        goto err_input;
    }

    input->name = pdev->name;
    input->phys = "gpio-keys/input0";
    input->id.bustype = BUS_HOST;

    __set_bit(EV_KEY, input->evbit);
    __set_bit(KEY_0, input->keybit);

    dev->input = input;
    platform_set_drvdata(pdev, dev);

    /* 5. 初始化去抖动定时器 */
    setup_timer(&dev->debounce_timer, key_debounce_handler, (unsigned long)dev);

    /* 6. 注册输入设备 */
    ret = input_register_device(dev->input);
    if (ret)
    {
        dev_err(&pdev->dev, "Failed to register input device\n");
        goto err_reg;
    }

    return 0;

err_reg:
    input_free_device(dev->input);
err_input:
    gpio_free(dev->gpio_index);
err_irq:
    return ret;
}

static int key_remove(struct platform_device *pdev)
{
    struct key_device *dev = platform_get_drvdata(pdev);

    del_timer_sync(&dev->debounce_timer);
    input_unregister_device(dev->input);
    gpio_free(dev->gpio_index);

    return 0;
}

/* 设备树匹配表 */
static const struct of_device_id key_of_match[] = {
    {.compatible = "key"},
    {},
};
MODULE_DEVICE_TABLE(of, key_of_match);

/* Platform驱动定义 */
static struct platform_driver key_driver = {
    .probe = key_probe,
    .remove = key_remove,
    .driver =
        {
            .name = DRV_NAME,
            .of_match_table = key_of_match,
        },
};
module_platform_driver(key_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Platform-based GPIO Key Driver");
