/*
 * This module creates target for linear device mapper which maps a linear range of the device-mapper
 * device onto a linear range of another device.
 *
 * See http://narendrapal2020.blogspot.com/2014/03/device-mapper.html and
 * techgmm.blogspot.com/p/writing-your-own-device-mapper-target.html.
 *
 * Test on linux kernel 2.6.32.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>
#include <linux/slab.h>

/* For underlying device */
struct my_dm_target {
    struct dm_dev *dev;
    sector_t start;
};

/* Map function , called whenever target gets a bio request. */

static int hello_target_map(struct dm_target *target, struct bio *bio,union map_info *map_context)
{
    struct my_dm_target *mdt = (struct my_dm_target *) target->private;

    printk("\nEntry : %s",__func__);

    /*  bio should perform on our underlying device   */
    bio->bi_bdev = mdt->dev->bdev;

    if ((bio->bi_rw & WRITE) == WRITE)
        printk("\nbio is a write request");
    else
        printk("\nbio is a read request");

    submit_bio(bio->bi_rw,bio);
    printk("\nExit : %s",__func__);
    return DM_MAPIO_SUBMITTED;
}

/*
 * This is constructor function of target gets called when we create some device of type 'hello_target'.
 * i.e on execution of command 'dmsetup create'. It gets called per device.
 */
static int hello_target_ctr(struct dm_target *target,unsigned int argc,char **argv)
{
    struct my_dm_target *mdt;
    unsigned long long start;
    int ret = 0;
    printk("\nEntry : %s",__func__);

    if (argc != 2) {
        printk("\n Invalid no.of arguments.\n");
        target->error = "Invalid argument count";
        ret =  -EINVAL;
    }

    mdt = kmalloc(sizeof(struct my_dm_target), GFP_KERNEL);

    if (mdt==NULL) {
        printk("\n Error in kmalloc\n");
        target->error = "Cannot allocate linear context";
        ret = -ENOMEM;
    }

    if (sscanf(argv[1], "%llu", &start)!=1) {
        target->error = "Invalid device sector";
        kfree(mdt);
        ret = -EINVAL;
    }
    mdt->start=(sector_t)start;

    /*  To add device in target's table and increment in device count */

    if (dm_get_device(target, argv[0], start, target->len, dm_table_get_mode(target->table), &mdt->dev)) {
        target->error = "Device lookup failed";
        goto out;
    }

    target->private = mdt;

    printk("\nExit : %s ",__func__);
    return ret;

out:
    printk("\nExit : %s with ERROR",__func__);
    return ret;
}

/*
 *  This is destruction function, gets called per device.
 *  It removes device and decrement device count.
 */
static void hello_target_dtr(struct dm_target *ti)
{
    struct my_dm_target *mdt = (struct my_dm_target *) ti->private;
    printk("\nEntry : %s",__func__);
    dm_put_device(ti, mdt->dev);
    kfree(mdt);
    printk("\nExit : %s",__func__);
}
/*  This structure is fops for hello target */
static struct target_type hello_target = {

    .name = "hello_target",
    .version = {1,0,0},
    .module = THIS_MODULE,
    .ctr = hello_target_ctr,
    .dtr = hello_target_dtr,
    .map = hello_target_map,
};

/*---------Module Functions -----------------*/

static int init_hello_target(void)
{
    int result;
    printk("\nEntry : %s",__func__);
    result = dm_register_target(&hello_target);
    if (result < 0) {
        printk("\nError in registering target");
    } else {
        printk("\nTarget registered");
    }
    printk("\nExit : %s",__func__);
    return 0;
}


static void cleanup_hello_target(void)
{
    printk("\nEntry : %s",__func__);
    dm_unregister_target(&hello_target);
    printk("\nTarget unregistered");
    printk("\nExit : %s",__func__);
}

module_init(init_hello_target);
module_exit(cleanup_hello_target);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Narendra Pal Singh");
