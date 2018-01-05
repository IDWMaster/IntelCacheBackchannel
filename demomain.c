#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <asm/io.h>


static struct file_operations ops = {0};

static void* buffer;
static size_t size;

static int demo_open(struct inode* node, struct file* fptr) {
  
  return 0;
}

static int demo_mmap (struct file * fd, struct vm_area_struct * vma) {
  printk("PERFORM MAP\n");
  if(!buffer) {
    return -EFAULT;
  }
  
  //down_read(&vma->vm_mm->mmap_sem);
  remap_pfn_range(vma,vma->vm_start,virt_to_phys(buffer) >> PAGE_SHIFT,size,vma->vm_page_prot);
  //up_read(&vma->vm_mm->mmap_sem);
  printk("FINISH MAP\n");
  
  return 0;
}
const char secret[] = "TOP SECRET\nMIDDLE SECRET\nBOTTOM SECRET";
static long demo_ioctl (struct file * fd, unsigned int opcode, unsigned long arg) {
  
const char* addr = secret;
copy_to_user((void*)arg,&addr,sizeof(void*));
return 0;
}

static int demo_release(struct inode* node, struct file* fptr) {
  return 0;
}

static int majorno;

static int demo_init(void)
{
  ops.owner = THIS_MODULE;
  ops.open = demo_open;
  ops.release = demo_release;
  ops.unlocked_ioctl = demo_ioctl;
  ops.mmap = demo_mmap;
  
  majorno = register_chrdev(0,"masterblock",&ops);
  printk("Registered device with major number %i\n",majorno);
  buffer = 0;
return 0;
}

static void demo_exit(void)
{
  unregister_chrdev(majorno,"masterblock");
  if(buffer) {
    kfree(buffer);
  }
}


module_init(demo_init);
module_exit(demo_exit);

MODULE_DESCRIPTION("Example kernel module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brian Bosak <brielcccc@gmail.com>");
