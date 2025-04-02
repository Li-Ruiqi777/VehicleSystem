#include <asm/uaccess.h>

#include <linux/init.h>
#include <linux/kern_levels.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/i2c.h>

#include "ap3216c.h"
#include "linux/printk.h"

// 设备驱动的信息
typedef struct
{
    dev_t devid;               // 设备号
    int major;                 // 主设备号
    int minor;                 // 次设备号
    struct cdev cdev;          // 字符设备
    struct class *class;       // 设备类
    struct device *device;     // 设备实例
    struct i2c_client *client; // i2c设备

    uint16_t ps_data;  // 接近传感器数据
    uint16_t als_data; // 环境光数据
    uint16_t ir_data;  // 红外光数据
} ap3216c_dev_t;

/*
 * @description : 从ap3216c读取多个寄存器数据
 * @param - dev:  ap3216c设备
 * @param - reg:  要读取的寄存器首地址
 * @param - val:  读取到的数据
 * @param - len:  要读取的数据长度
 * @return   : 操作结果
 */
static int ap3216c_read_regs(ap3216c_dev_t *ap3216c_device, u8 reg, void *val, int len)
{
    int ret;
    struct i2c_msg msg[2];
    struct i2c_client *client = ap3216c_device->client;

    // msg[0]为发送要读取的首地址
    msg[0].addr = client->addr; // ap3216c地址
    msg[0].flags = 0;           // 标记为发送数据
    msg[0].buf = &reg;          // 读取的首地址
    msg[0].len = 1;             // reg长度

    // msg[1]读取数据
    msg[1].addr = client->addr; // ap3216c地址
    msg[1].flags = I2C_M_RD;    // 标记为读取数据
    msg[1].buf = val;           // 读取数据缓冲区
    msg[1].len = len;           // 要读取的数据长度

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret == 2)
    {
        ret = 0;
    }
    else
    {
        printk("i2c rd failed=%d reg=%06x len=%d\n", ret, reg, len);
        ret = -EREMOTEIO;
    }
    return ret;
}

/*
 * @description : 向ap3216c多个寄存器写入数据
 * @param - dev:  ap3216c设备
 * @param - reg:  要写入的寄存器首地址
 * @param - val:  要写入的数据缓冲区
 * @param - len:  要写入的数据长度
 * @return    :   操作结果
 */
static int ap3216c_write_regs(ap3216c_dev_t *ap3216c_device, u8 reg, u8 *buf, u8 len)
{
    u8 b[256];
    struct i2c_msg msg;
    struct i2c_client *client = ap3216c_device->client;
    if (client == NULL)
    {
        printk("client is null");
        return 0;
    }

    b[0] = reg;              // 寄存器首地址
    memcpy(&b[1], buf, len); // 将要写入的数据拷贝到数组b里面

    msg.addr = client->addr; // ap3216c地址
    msg.flags = 0;           // 标记为写数据

    msg.buf = b;       // 要写入的数据缓冲区
    msg.len = len + 1; // 要写入的数据长度

    return i2c_transfer(client->adapter, &msg, 1);
}

/*
 * @description : 读取ap3216c指定寄存器值，读取一个寄存器
 * @param - dev:  ap3216c设备
 * @param - reg:  要读取的寄存器
 * @return    :   读取到的寄存器值
 */
static uint8_t ap3216c_read_reg(ap3216c_dev_t *ap3216c_device, u8 reg)
{
    u8 data = 0;

    ap3216c_read_regs(ap3216c_device, reg, &data, 1);
    return data;
}

/*
 * @description : 向ap3216c指定寄存器写入指定的值，写一个寄存器
 * @param - dev:  ap3216c设备
 * @param - reg:  要写的寄存器
 * @param - data: 要写入的值
 * @return   :    无
 */
static void ap3216c_write_reg(ap3216c_dev_t *ap3216c_device, u8 reg, u8 data)
{
    u8 buf = 0;
    buf = data;
    ap3216c_write_regs(ap3216c_device, reg, &buf, 1);
}

/*
 * @description : 读取AP3216C的数据，读取原始数据，包括ALS,PS和IR, 注意！
 *    : 如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
 * @param - ir : ir数据
 * @param - ps  : ps数据
 * @param - ps  : als数据
 * @return   : 无。
 */
void ap3216c_read_data(ap3216c_dev_t *ap3216c_device)
{
    unsigned char i = 0;
    unsigned char buf[6];

    /* 循环读取所有传感器数据 */
    for (i = 0; i < 6; i++)
    {
        buf[i] = ap3216c_read_reg(ap3216c_device, AP3216C_IRDATALOW + i);
    }
    /* IR_OF位为1,则数据无效 */
    if (buf[0] & 0X80)
    {
        ap3216c_device->ir_data = 65535;
        printk(KERN_INFO "IR data is invalid\n");
    }
    else
        ap3216c_device->ir_data = ((unsigned short)buf[1] << 2) | (buf[0] & 0X03);

    /* 读取ALS传感器的数据*/
    ap3216c_device->als_data = ((unsigned short)buf[3] << 8) | buf[2];

    /* IR_OF位为1,则数据无效*/
    if (buf[4] & 0x40)
    {
        ap3216c_device->ps_data = 65535;
        printk(KERN_INFO "PS data is invalid\n");
    }
        
    else
        ap3216c_device->ps_data = ((unsigned short)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);
}

static int ap3216c_open(struct inode *inode, struct file *file)
{
    // printk(KERN_INFO "LED opened\n");
    file->private_data = container_of(inode->i_cdev, ap3216c_dev_t, cdev);

    /* 复位AP3216C*/
    ap3216c_write_reg(file->private_data, AP3216C_SYSTEMCONG, 0x04);
    /* AP3216C复位最少10ms */
    mdelay(50);
    /* 开启ALS、PS+IR*/
    ap3216c_write_reg(file->private_data, AP3216C_SYSTEMCONG, 0X03);

    return 0;
}

static int ap3216c_release(struct inode *inode, struct file *file)
{
    // printk(KERN_INFO "LED closed\n");
    return 0;
}

static ssize_t ap3216c_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    short data[3];
    ap3216c_dev_t *ap3216c_device = file->private_data;
    ap3216c_read_data(file->private_data);

    data[0] = ap3216c_device->ir_data;
    data[1] = ap3216c_device->als_data;
    data[2] = ap3216c_device->ps_data;
    copy_to_user(buf, data, sizeof(data));

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ap3216c_open,
    .release = ap3216c_release,
    .read = ap3216c_read,
};

static int ap3216c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    ap3216c_dev_t *apc3216c_device = devm_kzalloc(&client->dev, sizeof(ap3216c_dev_t), GFP_KERNEL);
    // 将设备信息保存到i2c_client的上下文中, 以便在其他地方能够访问到
    i2c_set_clientdata(client, apc3216c_device);
    apc3216c_device->client = client;

    // 动态分配设备号
    if (apc3216c_device->major) // 已经定义了设备号
    {
        apc3216c_device->devid = MKDEV(apc3216c_device->major, 0);
        register_chrdev_region(apc3216c_device->devid, 1, "ap3216c"); // 在/proc/devices下的节点名为ap3216c
    }
    else // 未定义设备号
    {
        alloc_chrdev_region(&apc3216c_device->devid, 0, 1, "ap3216c"); // 申请设备号
        apc3216c_device->major = MAJOR(apc3216c_device->devid);        // 获取分配号的主设备号
        apc3216c_device->minor = MINOR(apc3216c_device->devid);        // 获取分配号的次设备号
    }
    printk("ap3216c major = %d, minor = %d\r\n", apc3216c_device->major, apc3216c_device->minor);

    // 注册字符设备
    apc3216c_device->cdev.owner = THIS_MODULE;
    cdev_init(&apc3216c_device->cdev, &fops);
    int ret =
        cdev_add(&apc3216c_device->cdev, apc3216c_device->devid, 1); // 将字符设备和设备号关联, 并插入内核
    if (ret)
    {
        printk(KERN_NOTICE "Error %d adding ap3216c", ret);
        unregister_chrdev_region(apc3216c_device->devid, 1);
        return ret;
    }

    // 在/sys/class下创建设备类
    apc3216c_device->class = class_create(THIS_MODULE, "ap3216c"); // 在/sys/class下的节点名为ap3216c
    if (IS_ERR(apc3216c_device->class))
    {
        return PTR_ERR(apc3216c_device->class);
    }

    // 在/dev下创建设备节点,并关联到设备类
    apc3216c_device->device = device_create(apc3216c_device->class, NULL, apc3216c_device->devid, NULL,
                                            "ap3216c"); // 在/dev下的节点名为ap3216c
    if (IS_ERR(apc3216c_device->device))
    {
        return PTR_ERR(apc3216c_device->device);
    }

    printk(KERN_INFO "ap3216c module loaded \n");

    return 0;
}

static int ap3216c_remove(struct i2c_client *client)
{
    ap3216c_dev_t *ap3216c_device = i2c_get_clientdata(client);

    device_destroy(ap3216c_device->class, ap3216c_device->devid);
    class_destroy(ap3216c_device->class);
    cdev_del(&ap3216c_device->cdev);
    unregister_chrdev_region(ap3216c_device->devid, 1);

    printk(KERN_INFO "ap3216c module unloaded \n");
}

static const struct of_device_id ap3216c_of_match[] = {{.compatible = "alientek,ap3216c"}, {}};
static struct i2c_device_id ap3216c_i2c_id[] = {{"ap3216c", 0}, {}};

static struct i2c_driver ap3216c_driver = {
    .driver =
        {
            .name = "ap3216c",                  // 名称匹配(用于板级代码静态注册时)
            .of_match_table = ap3216c_of_match, // 设备树匹配
        },
    .id_table = ap3216c_i2c_id, // id table匹配
    .probe = ap3216c_probe,
    .remove = ap3216c_remove,
};

static int ap3216c_module_init(void)
{
    return i2c_add_driver(&ap3216c_driver);
}

static void ap3216c_module_exit(void)
{
    i2c_del_driver(&ap3216c_driver);
}

module_init(ap3216c_module_init);
module_exit(ap3216c_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LRQ");
MODULE_DESCRIPTION("AP3216C Driver");
MODULE_VERSION("1.0");
