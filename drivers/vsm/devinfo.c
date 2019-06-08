#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/of_fdt.h>
#include <asm/setup.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/genhd.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <linux/ctype.h>

static int board_id;

//+nhatnd3,add ro.boot.hardware.revision
static int hw_revision_proc_show(struct seq_file *m, void *v)
{
    printk("vsm Read value from proinfo success! %d\n", board_id);
    seq_printf(m, "%d", board_id);

    return 0;
}
//-nhatnd3,add ro.boot.hardware.revision

#define PROC_FOPS_RO(name)    \
    static int name##_proc_open(struct inode *inode, struct file *file)    \
    {                                    \
        return single_open(file, name##_proc_show, PDE_DATA(inode));    \
    }                                    \
    static const struct file_operations name##_proc_fops = {        \
        .owner          = THIS_MODULE,                    \
        .open           = name##_proc_open,                \
        .read           = seq_read,                    \
        .llseek         = seq_lseek,                    \
        .release        = single_release,                \
    }

#define PROC_ENTRY(name) {__stringify(name), &name##_proc_fops}

//nhatnd3,add ro.boot.hardware.revision
PROC_FOPS_RO(hw_revision);

struct pentry {
    const char *name;
    const struct file_operations *fops;
};

const struct pentry lk_info_entries[] = {

//nhatnd3,add ro.boot.hardware.revision
    PROC_ENTRY(hw_revision)
};

static int board_id_probe(struct platform_device *pdev)
{

    static int board_id_pin0,board_id_pin1,board_id_pin2, board_id_pin3;
    printk("vsmart %s()\n", __func__);

    board_id_pin0 = of_get_named_gpio(pdev->dev.of_node,"board_id0_read",0);
    board_id_pin1 = of_get_named_gpio(pdev->dev.of_node,"board_id1_read",0);
    board_id_pin2 = of_get_named_gpio(pdev->dev.of_node,"board_id2_read",0);
    board_id_pin3 = of_get_named_gpio(pdev->dev.of_node,"board_id3_read",0);
    gpio_request_one(board_id_pin0, GPIOF_IN, "vsm gpio");
    gpio_request_one(board_id_pin1, GPIOF_IN, "vsm gpio");
    gpio_request_one(board_id_pin2, GPIOF_IN, "vsm gpio");
    gpio_request_one(board_id_pin3, GPIOF_IN, "vsm gpio");
    printk("vsmart %s() GPIO: %d ,%d ,%d, %d \n", __func__, board_id_pin0, board_id_pin1, board_id_pin2, board_id_pin3);
    board_id = gpio_get_value(board_id_pin0) \
               + (gpio_get_value(board_id_pin1)<<1) \
               + (gpio_get_value(board_id_pin2)<<2) \
               + (gpio_get_value(board_id_pin3)<<3);
    printk("vsmart %s() GPIO value: %d , %d ,%d, %d \n", __func__, gpio_get_value(board_id_pin0),
            gpio_get_value(board_id_pin1),gpio_get_value(board_id_pin2), gpio_get_value(board_id_pin3));
    printk("vsmart %s() board_id=%d\n", __func__, board_id);
    return 0;
}

static const struct of_device_id board_id_of_match[] = {
    {.compatible = "qcom,board_id"}
};

struct platform_driver board_id_driver = {
    .probe = board_id_probe,
    .driver = {
        .name = "board_id-driver",
        .of_match_table = board_id_of_match
    }
};

static int __init proc_lk_info_init(void)
{
    struct proc_dir_entry *dir_entry = NULL;
    int i = 0,ret;

    dir_entry = proc_mkdir("vsm_info", NULL);
    if (!dir_entry) {
        pr_err("Failed to create /proc/ entry\n");

        return -ENOMEM;

    }
    for (i = 0; i < ARRAY_SIZE(lk_info_entries); i++) {
        if (! proc_create(lk_info_entries[i].name, S_IRUGO, dir_entry, lk_info_entries[i].fops))
            pr_err("Failed to create /proc/lk_info entry nodes\n");
    }
    ret = platform_driver_register(&board_id_driver);
    if (ret < 0) {
        printk("devinfo platform_driver_register error\n");
    }

    return 0;
}

module_init(proc_lk_info_init);
