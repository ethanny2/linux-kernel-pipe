

#include <linux/miscdevice.h>
#include <linux/rtc.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/moduleparam.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("ETHAN SOO HON");
MODULE_DESCRIPTION("PIPE KERNEL CHAR DEVICE");

/*Defintions needed for this implementation. Since this module can be opened multiple times (have multiple instances being read/written to)
keep track of the number of times the device is opened.*/

static int openInstances;

/*Semphores
	1 for read() operation critical section
	1 for write() operation critical section
	1 for indicating if there are items to consume in Buffer
	1 for indicating if there are items able to fit inside buffer (If you can produce)
*/

static struct semaphore semReadAccess;
static struct semaphore semWriteAccess;
/* These are COUNTING SEMAPHORES*/
static struct semaphore semCountEmpty; //Counts the number of empty slots in the buffer. All empty to begin with. Where input size N, this would be N when started.
static struct semaphore semCountFull; //Counts  the number of full buffers in the 2D buffer array. (This is 0 when you start)


/*Buffers
	- 1d array (buffer) 
	-Size of buffer from command line
	- keep track of the empty slots in the buffer (C won't tell me that)
	-Read and write, which index they are in within the buffer
 */
int * pipeBuffer;
static int bufferSize;



/* Indices*/
static int bufferOpenIndex;
static int  currentReadIndex = 0;
static int currentWriteIndex = 0;


/* Declarations for command line arguements to be passed to module
	The module_param() macro takes 3 arguments: the name of the variable,
 	its type
	and permissions for the corresponding file in sysfs.
*/
module_param(bufferSize, int, 0000);




static int my_open(struct inode *inode, struct file *file)
{
	openInstances++;
	printk(KERN_NOTICE "Opened NUMPIPE misc char device\n");
	return 0;
}


/*When instance of this pipe is closed decrement the open devices */
static int my_close(struct inode *inodep, struct file *filp)
{
	openInstances--;
	printk(KERN_NOTICE "Closed NUMPIPE misc char device\n");
	return 0;
}

/*READ BY COSNUMER */
static ssize_t my_read(struct file *filp,char *buffer,size_t count,loff_t * position)
{
	int i;
	printk(KERN_NOTICE "Using read() function of NUMPIPE  in kernel space\n");
	if(currentReadIndex > bufferSize){
		printk(KERN_WARNING "Current index to read exceeds the size/bounds of the buffer EXITING\n");
		return ENOMEM; /*Not enough memory in buffer */
	}

	/*If the current amount of calculated empty slots in the buffer exceeds the size of the buffer then break */

	/* If copying the kernel data to user buffer is 0 the assignment failed
	Cannot derefernce user-space point "buffer" because it has different data in the kernel space
	Count is how many bytes we are copying into the user buffer*/

	/*Check if semaphore for read operation is avalible  */
	down_interruptible(&semReadAccess);
	/*Check if counting semaphore has enough empty slots to fit a new piece of content */
	down_interruptible(&semCountFull);
	/*Each entry in the buffer is to be read ONLY 1 time by 1 consumer. The modulus is to ensure the current read index is in 
	the bounds of the buffer. But if currentReadIndex > bufferSize then you can potential re-read indicies. So we check at the beginning conditional statement*/
	currentReadIndex %= bufferSize;

	/*
	-If bufferOpenSlots  > size of Buffer thats an error
	-If copy to user returns 0 thats an error
	- Copy 1 int from our pipe buffer to the userspace buffer.
	*/
	if( copy_to_user(buffer, pipeBuffer, sizeof(int)) !=0 ){
		printk(KERN_WARNING "ERROR: copy_to_user did not write all bytes to user buffer\n");
		return ENOMEM;
	}else{
		printk(KERN_WARNING "SUCCESS: copy_to_user wrote all bytes to user buffer\n");
	}
	++currentReadIndex;
	++bufferOpenIndex;
	/* Just consumed string from buffer increment the number of empty spaces and release the resource*/
	up(&semCountEmpty);
	/*Just consumed string from buffer so release the read lock */
	up(&semReadAccess);
	/*Return value is size of buffer (count) */
	return count;
}

static ssize_t my_write(struct file *file, const char __user *buffer,size_t count, loff_t *ppos)
{
	printk(KERN_NOTICE "Using write() function of numpipe module\n");
	if(currentWriteIndex > bufferSize){
		printk(KERN_WARNING "Current index to write to exceeds bounds/size of buffer.\n");
		return ENOMEM;
	}
	/*Check if semaphore write operation is availible*/
	down_interruptible(&semWriteAccess);
	/*Check if semaphore has any empty slots so it can accept a new write to buffer */
	down_interruptible(&semCountEmpty);
	/*Make sure write_index is in bound of buffer size. In conjuction with conditional statement at the top */
	currentWriteIndex %= bufferSize;
	/*Write to the currentWriteIndex - th row, one char in a column at a time */
	if( copy_from_user(pipeBuffer, buffer, sizeof(int)) != 0){
			/* Error did not write all bytes*/
		printk(KERN_WARNING "ERROR: copy_from_user did NOT read all bytes from user buffer\n");
		return ENOMEM; 
	}else{
		printk(KERN_NOTICE "SUCCESS: copy_from_user read all bytes from user buffer \n");
	}
	
	/* move the index to write to next row*/
	++currentWriteIndex;
	/* Decrement the number of open slots*/
	--bufferOpenIndex;
	/* release write semaphore */
	up(&semWriteAccess);
	/*Added a string to buffer increment counting semaphore */
	up(&semCountFull);
	/*Return the number of bytes to read */
	return count;
}




static const struct file_operations my_fops = {
    .owner			= THIS_MODULE, /* This is a predefined macro for convience*/
	.open			= my_open,
	.read                       = my_read,
	.write			= my_write,
	.release			= my_close
};

struct miscdevice numpipe_device = {
    .minor = MISC_DYNAMIC_MINOR, /* Dyanmic minor assigned with a macro*/
    .name = "numpipe",  /*Name of the misc device has to be same name as the .c file */
    .fops = &my_fops, /*Point to struct that has the file definitions */
};

static int __init misc_init(void)
{
	int error;
	int i;
    /*Attempting to register misc device. Pass in struct */
	error = misc_register(&numpipe_device);
	if (error){
		printk(KERN_ERR "ERROR TRYING TO REGISTER NUMPIPE \n");
		return error;
	}
	printk( KERN_NOTICE "SUCCESS Registering mytime misc char device NUMPIPE\n");
	printk(KERN_NOTICE "Allocating buffer with size %d\n",bufferSize);
    /*Allocate buffer here, array of ints */
    /* Initialize semaphores, openInstances is now 0*/
    sema_init(&semCountEmpty,bufferSize); // emptySize = bufferSize initially.
    sema_init(&semCountFull,0); //
    sema_init(&semReadAccess,1); //1 indicating that the key for the critical section is available
    sema_init(&semWriteAccess,1); // same as above
    pipeBuffer = kmalloc(bufferSize * sizeof(int),GFP_KERNEL);
    printk(KERN_NOTICE "Successfully created buffer\n");
    bufferOpenIndex = bufferSize; //All initially empty 
    openInstances = 0;
    return 0;
}

static void __exit misc_exit(void)
{
	//Freeing 1D array is easy
	kfree(pipeBuffer);
	printk(KERN_NOTICE "SUCCESSFULLY FREED NUMPIPE buffer\n");
	misc_deregister(&numpipe_device);
	printk(KERN_NOTICE "SUCCESSFULLY DEREGISTERED NUMPIPE  misc device\n");
}




module_init(misc_init);
module_exit(misc_exit);
