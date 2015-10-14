/*
 * =====================================================================================
 *
 *       Filename:  ath9k_csi.c
 *
 *    Description:  extrac csi and data together from hardware
 *
 *        Version:  1.0
 *        Created:  14/12/2013 22:55:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yaxiong Xie xieyaxiongfly@gmail.com, 
 *   Organization:  WANDS group @ Nanyang Technological University 
 *
 * =====================================================================================
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/netdevice.h>

#include "ar9003_csi.h"
#include "ar9003_mac.h"
#include "ar9003_phy.h"
#include "mac.h"
#include "hw.h"

#define PRINTBUF_LEN 20480                      // Generally, buffere with 20480 bits is enough
                                                // You can change the size freely

#define BITS_PER_BYTE 8
#define BITS_PER_SYMBOL 10
#define BITS_PER_COMPLEX_SYMBOL  (2 * BITS_PER_SYMBOL)

#define DEVICE_NAME "CSI_dev"
#define CLASS_NAME "CSI_class"
#define AH_MAX_CHAINS 3                         //maximum chain number, we set it to 3
#define NUM_OF_CHAINMASK (1 << AH_MAX_CHAINS)

volatile u32 csi_head;
volatile u32 csi_tail;
volatile u32 csi_len;
volatile u32 csi_valid;
volatile u32 recording;

static struct ath9k_csi csi_buf[16];
static char   printbuf[PRINTBUF_LEN];

static int    majorNumber;             	    ///< Stores the device number -- determined automatically
static struct class*  ebbcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* ebbcharDevice = NULL; ///< The device-driver device struct pointer

DECLARE_WAIT_QUEUE_HEAD(csi_queue);

static int     csi_open(struct inode *inode, struct file *file);
static int     csi_close(struct inode *inode, struct file *file);
static ssize_t csi_read(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos);
static ssize_t csi_write(struct file *file, const char __user *user_buf,
			     size_t count, loff_t *ppos);

static const struct file_operations csi_fops = {
	.read    = csi_read,
	.write   = csi_write,
	.open    = csi_open,
    .release = csi_close,
	.llseek  = default_llseek,
};

static u_int8_t Num_bits_on[NUM_OF_CHAINMASK] = {
    0 /*       000 */,
    1 /*       001 */,
    1 /*       010 */,
    2 /*       011 */,
    1 /*       100 */,
    2 /*       101 */,
    2 /*       110 */,
    3 /*       111 */
};

u_int8_t ar9300_get_nrx_csi(struct ath_hw *ah)
{
	return Num_bits_on[ah->rxchainmask];
}

static int __init csi_init(void)
{

    // initalize parameters 
    csi_head = 0;
    csi_tail = 0;
    recording = 0;
    csi_valid = 0;
    
    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &csi_fops);
    if (majorNumber<0){
       printk(KERN_ALERT "debug_csi: failed to register a major number\n");
       return majorNumber;
    }
    printk(KERN_INFO "debug_csi: registered correctly with major number %d\n", majorNumber);

    // Register the device class
    ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ebbcharClass)){                // Check for error and clean up if there is
       unregister_chrdev(majorNumber, DEVICE_NAME);
       printk(KERN_ALERT "debug_csi: Failed to register device class\n");
       return PTR_ERR(ebbcharClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "debug_csi: device class registered correctly\n");
    
    // Register the device driver
    ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ebbcharDevice)){               // Clean up if there is an error
       class_destroy(ebbcharClass);           // Repeated code but the alternative is goto statements
       unregister_chrdev(majorNumber, DEVICE_NAME);
       printk(KERN_ALERT "Failed to create the device\n");
       return PTR_ERR(ebbcharDevice);
    }
    printk(KERN_INFO "debug_csi: device class created correctly \n"); // Made it! device was initialized
    
    return 0;
}

static void __exit csi_exit(void)
{
    /* delete and unregister the devices we have created and registered */
    device_destroy(ebbcharClass, MKDEV(majorNumber, 0));     // remove the device
    class_unregister(ebbcharClass);                          // unregister the device class
    class_destroy(ebbcharClass);                             // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
    printk(KERN_INFO "debug_csi: Goodbye CSI device!\n");

}

static int csi_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "debug_csi: csi open! \n");
    recording = 1;                                            // we can begin to record when  
                                                              // the devices is open 
    return 0;
}

static int csi_close(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "debug_csi: csi close! \n");
    recording = 0;                                            // close and reset 
	return 0;
}

static ssize_t csi_read(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos)
{
    u_int16_t len;
    u_int8_t *csi_buf_addr;
    u_int8_t *payload_buf_addr;
    u_int16_t csi_len, payload_len;
    struct ath9k_csi* csi;
    struct csi_pkt_status *RxStatus;
    
    *ppos = 0;
    
    if (csi_head == csi_tail) 
    {                                                       // wait until time out
        wait_event_interruptible_timeout(csi_queue,csi_head != csi_tail,  5*HZ);
    } 
    if(csi_head != csi_tail){
        csi = (struct ath9k_csi*)&csi_buf[csi_tail];
        len = 0;
        
        RxStatus = &(csi->pkt_status);                      // the status struct
        
        csi_len          = RxStatus->csi_len;               // csi length (bytes) 
        csi_buf_addr     = csi->csi_buf;                    // csi buffer 
        payload_len      = csi->payload_len;                // payload length (bytes)
        payload_buf_addr = csi->payload_buf;                // payload buffer
        

        memcpy(printbuf,RxStatus,23);                       // copy the status to the buffer 
        len += 23;
        memcpy(printbuf+len,&payload_len, 2);               // record the length of payload 
        len += 2;
        if (csi_len > 0){
            memcpy(printbuf+len,csi_buf_addr,csi_len);      // copy csi to the buffer
            len += csi_len;
        }
        memcpy(printbuf+len,payload_buf_addr, payload_len); // copy payload to the buffer
        len += payload_len;
        memcpy(printbuf+len,&len, 2);                       // record how many bytes we copy 
        len += 2;
        copy_to_user(user_buf,printbuf,len);                // COPY
        
        csi_tail = (csi_tail+1) & 0x0000000F;               // delete the buffer 
        return len;
    }else
    {
        return 0;
    }

}

static ssize_t csi_write(struct file *file, const char __user *user_buf,
			     size_t count, loff_t *ppos)
{
    /* csi write is left blank, you can install your own function here! 
     * With csi write fucntion, you can pass some parameters to the ath9k driver
     * with those parameters you can control the behavor of the WiFi 
     * NIC cards, for example, control the transmiting rate, channel 
     * bandwidth and so on. If you have any problem installing the function,
     * please contact Yaxiong Xie (xieyaxiongfly@gmail.com)
     */

    printk(KERN_ALERT "debug_csi: csi write! \n");
	return 0;
}



//Record payload of the received packet
void csi_record_payload(void* data, u_int16_t data_len)
{
    struct ath9k_csi* csi;
    if(recording )
    {
        if( ((csi_head + 1) & 0x0000000F) == csi_tail)              // check and update 
            csi_tail = (csi_tail + 1) & 0x0000000F;
        
        csi = (struct ath9k_csi*)&csi_buf[csi_head];
        memcpy((void*)(csi->payload_buf),data, data_len);           // copy the payload
        csi->payload_len = data_len;                                // record the payload length (bytes)

        csi_valid = 1;
    }
}
EXPORT_SYMBOL(csi_record_payload);

//Record the status of the packet
void csi_record_status(struct ath_hw *ah, struct ath_rx_status *rxs, struct ar9003_rxs *rxsp,void* data)
{
    struct ath9k_csi* csi;

    u_int8_t  nr;
    u_int8_t  chan_BW;
    u_int8_t  rx_not_sounding;
    u_int8_t  rx_hw_upload_data;
    u_int8_t  rx_hw_upload_data_valid;
    u_int8_t  rx_hw_upload_data_type;
    
    /* some parameters about the hardware upload data*/ 
    rx_hw_upload_data             = (rxsp->status2 & AR_hw_upload_data) ? 1 : 0;
    rx_not_sounding               = (rxsp->status4 & AR_rx_not_sounding) ? 1 : 0;
    rx_hw_upload_data_valid       = (rxsp->status4 & AR_hw_upload_data_valid) ? 1 : 0;
    rx_hw_upload_data_type        = MS(rxsp->status11, AR_hw_upload_data_type);
   
    /* filter out some packets without CSI value (e.g., the beacon)*/
    if(rxs->rs_phyerr == 0 && rx_hw_upload_data == 0 &&
                rx_hw_upload_data_valid == 0 && rx_hw_upload_data_type == 0){
        return;
    }

    if(recording && csi_valid == 1)
    {
        csi = (struct ath9k_csi*)&csi_buf[csi_head];
       
        csi->pkt_status.tstamp    = rxs->rs_tstamp;     // time stamp of the rx packet 
        
        csi->pkt_status.channel   = ah->curchan->channel;               
        
        chan_BW                   = (rxsp->status4 & AR_2040) >> 1;        
        csi->pkt_status.ChanBW    = chan_BW;            // channel bandwidth 
        nr                        = ar9300_get_nrx_csi(ah);                    
        csi->pkt_status.nr        = nr;                 // rx antennas number
        
        csi->pkt_status.phyerr    = rxs->rs_phyerr;     // PHY layer error code
        
        csi->pkt_status.rssi      = rxs->rs_rssi; 
        csi->pkt_status.rssi_ctl0 = rxs->rs_rssi_ctl[0];            
        csi->pkt_status.rssi_ctl1 = rxs->rs_rssi_ctl[1];
        csi->pkt_status.rssi_ctl2 = rxs->rs_rssi_ctl[2];
        
        csi->pkt_status.noise     = 0;                  // to be updated
        csi->pkt_status.rate      = rxs->rs_rate;       // data rate 
        
        rx_hw_upload_data         = (rxsp->status2 & AR_hw_upload_data) ? 1 : 0;
        rx_not_sounding           = (rxsp->status4 & AR_rx_not_sounding) ? 1 : 0;
        rx_hw_upload_data_valid   = (rxsp->status4 & AR_hw_upload_data_valid) ? 1 : 0;
        rx_hw_upload_data_type    = MS(rxsp->status11, AR_hw_upload_data_type);
        
        // Decides how many tones(subcarriers) are used according to the channel bandwidth
        if (chan_BW == 0){
            csi->pkt_status.num_tones = 56;             // 20MHz Channel
        }
        else if (chan_BW == 1){
            csi->pkt_status.num_tones = 114;            // 40MHz Channel
        } 
        else{
            csi->pkt_status.num_tones = 56;             // 20MHz Channel
            printk("Error happends for channel bandwidth recording!!\n");
        }
        
        /* tx antennas number 
         * NOTE: when the packet is received with error
         * The antenna number value is not correct
         */
        csi->pkt_status.nc        = (int) (rxs->rs_datalen * BITS_PER_BYTE) /
                        (int) (BITS_PER_COMPLEX_SYMBOL * csi->pkt_status.nr * csi->pkt_status.num_tones);
       
        /* copy the csi value to the allocated csi buffer */
        if ( rxs->rs_datalen >0 && rx_hw_upload_data == 1 &&
                rx_hw_upload_data_valid == 1 && rx_hw_upload_data_type == 1){
            csi->pkt_status.csi_len = rxs->rs_datalen;
            memcpy((void*)(csi->csi_buf),data,rxs->rs_datalen);
        }else {
            csi->pkt_status.csi_len = 0;
        }
        
        csi_valid = 0;                                  // update 
        csi_head = (csi_head + 1) & 0x0000000F;

        wake_up_interruptible(&csi_queue);              // wake up waiting queue 
    }
    
}
EXPORT_SYMBOL(csi_record_status);


module_init(csi_init);
module_exit(csi_exit);

MODULE_AUTHOR("YAXIONG XIE");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CSI EXTRACTION");

