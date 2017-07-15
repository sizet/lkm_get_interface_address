// ©.
// https://github.com/sizet/lkm_get_interface_address

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/inetdevice.h>




#define FILE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define DMSG(msg_fmt, msg_args...) \
    printk(KERN_INFO "%s(%04u): " msg_fmt "\n", FILE_NAME, __LINE__, ##msg_args)




static ssize_t node_read(
    struct file *file,
    char __user *buffer,
    size_t count,
    loff_t *pos);

static ssize_t node_write(
    struct file *file,
    const char __user *buffer,
    size_t count,
    loff_t *pos);

static char *node_name = "get_interface_address";
static struct proc_dir_entry *node_entry;
static struct file_operations node_fops =
{
    .read  = node_read,
    .write = node_write,
};




static int get_interface_address(
    char *if_name)
{
    int fret = -1;
    struct net_device *net_dev;
    struct in_device *in4_dev;
    struct in_ifaddr *addr4_list;


    net_dev = dev_get_by_name(&init_net, if_name);
    if(net_dev == NULL)
    {
        DMSG("call dev_get_by_name(%s) fail", if_name);
        goto FREE_01;
    }

    DMSG("");
    DMSG("%s MAC = %02X:%02X:%02X:%02X:%02X:%02X",
         net_dev->name,
         net_dev->dev_addr[0], net_dev->dev_addr[1], net_dev->dev_addr[2],
         net_dev->dev_addr[3], net_dev->dev_addr[4], net_dev->dev_addr[5]);

    in4_dev = __in_dev_get_rtnl(net_dev);
    if(in4_dev == NULL)
    {
        DMSG("call __in_dev_get_rtnl(%s) fail", if_name);
        goto FREE_02;
    }

    for(addr4_list = in4_dev->ifa_list; addr4_list != NULL; addr4_list = addr4_list->ifa_next)
    {
        DMSG("%s (IPv4)", addr4_list->ifa_label);
        DMSG("local     = %pi4", &addr4_list->ifa_local);
        DMSG("remote    = %pi4", &addr4_list->ifa_address);
        DMSG("mask      = %pi4", &addr4_list->ifa_mask);
        DMSG("broadcast = %pi4", &addr4_list->ifa_broadcast);
        DMSG("prefixlen = %u", addr4_list->ifa_prefixlen);
    }

    fret = 0;
FREE_02:
    dev_put(net_dev);
FREE_01:
    return fret;
}

static ssize_t node_read(
    struct file *file,
    char __user *buffer,
    size_t count,
    loff_t *pos)
{
    DMSG("%s <if_name>", node_name);
    DMSG("  if_name : network interface name. (ex : eth0)");

    return 0;
}

static ssize_t node_write(
    struct file *file,
    const char __user *buffer,
    size_t count,
    loff_t *pos)
{
    char read_buf[36];
    size_t rlen = sizeof(read_buf) - 1;


    memset(read_buf, 0, sizeof(read_buf));
    rlen = count >= rlen ? rlen : count;
    copy_from_user(read_buf, buffer, rlen);
    if(rlen > 0)
        if(read_buf[rlen - 1] == '\n')
        {
            rlen--;
            read_buf[rlen] = '\0';
        }

    if(rlen > 0)
        if(get_interface_address(read_buf) < 0)
        {
            DMSG("call get_interface_address() fail");
        }

    return count;
}

static int __init main_init(
    void)
{
    if((node_entry = proc_create(node_name, S_IFREG | S_IRUGO | S_IWUGO, NULL, &node_fops)) == NULL)
    {
        DMSG("call proc_create(%s) fail", node_name);
        return 0;
    }

    return 0;
}

static void __exit main_exit(
    void)
{
    remove_proc_entry(node_name, NULL);

    return;
}

module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Che-Wei Hsu");
MODULE_DESCRIPTION("Get Interface Address");
