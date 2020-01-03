#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/device.h>

#define CNAME "hello"
int major = 0;
char kbuf[128] = {0};
struct class *cls = NULL;
struct device *dev = NULL;
//struct mutex lock;//���廥����
atomic_t atm = ATOMIC_INIT(1); //����ԭ�Ӳ�������
int led_open(struct inode *inode, struct file *file)
{
	/*
	//���� 
	if(!mutex_trylock(&lock)){
		return -EBUSY;
	}
	//��ԭ�ӱ�����ֵ��1����0�Ƚϣ�������Ϊ0����ʾ��ȡ
	  ���ɹ��ˣ�����ʧ�ܡ�
	*/
		if(!atomic_dec_and_test(&atm)){
		atomic_inc(&atm);
		return -EBUSY;
	}
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
ssize_t led_read(struct file *file, char __user *ubuf,
	size_t size, loff_t * offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf))size = sizeof(kbuf);
	ret = copy_to_user(ubuf, kbuf,size);
	if(ret){
		printk("copy data to user error\n");
		return -EIO;
	}

	return size;
}

ssize_t led_write(struct file *file, const char __user *ubuf,
	size_t size, loff_t *offs)
{
	int ret;
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	if(size > sizeof(kbuf))size = sizeof(kbuf);
	ret = copy_from_user(kbuf, ubuf,size);
	if(ret){
		printk("copy data from user error\n");
		return -EIO;
	}

	return size;
}

int led_close(struct inode *inode, struct file *file)
{
	//����
	//mutex_unlock(&lock);
	atomic_inc(&atm);
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}

const struct file_operations fops = {
	.open    = led_open,
	.read    = led_read,
	.write   = led_write,
	.release = led_close,
};

static int __init demo_init(void)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	major = register_chrdev(0,CNAME,&fops);
	if(major <= 0){
		printk("register char devices driver error\n");
		return  -EAGAIN;
	}
	printk("major = %d\n",major);

	cls = class_create(THIS_MODULE,CNAME);
	if(IS_ERR(cls)){
		printk("class create error\n");
		return PTR_ERR(cls);
	}
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,CNAME);
	if(IS_ERR(dev)){
		printk("device create error\n");
		return PTR_ERR(dev);
	}

	 //��ʼ��������
	 //mutex_init(&lock);
	
	return 0;
}

static void __exit demo_exit(void)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,CNAME);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");
