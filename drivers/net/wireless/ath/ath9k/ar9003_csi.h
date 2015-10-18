/*
 * =====================================================================================
 *
 *       Filename:  ar003_csi.h
 *
 *    Description:  ar003_csi data structure definiation
 *
 *        Version:  1.0
 *        Created:  14/12/2013 20:04:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yaxiong Xie xieyaxiongfly@gmail.com 
 *   Organization:  
 *
 * =====================================================================================
 */

#include "hw.h"
#include "mac.h"
#include "ar9003_mac.h"

#define DBG_CSI(fmt, args...) printk(fmt,## args)

#define AR_hw_upload_data     0x00400000
#define AR_hw_upload_data_S   22
#define AR_rx_not_sounding    0x00000010
#define AR_hw_upload_data_valid    0x00000080
#define AR_hw_upload_data_valid_S  7  
#define AR_hw_upload_data_type 0x06000000
#define AR_hw_upload_data_type_S   25


struct csi_pkt_status {
    u_int64_t   tstamp;    /* h/w assigned timestamp */
    u_int16_t   csi_len;    /* csi length */
    u_int16_t   channel;     /* receiving channel frequency */
	u_int8_t	phyerr;	    /* phy error code */
    
    u_int8_t    noise;       /* noise floor */
	u_int8_t	rate;	     /* h/w receive rate index */
    u_int8_t    ChanBW;      /* receiving channel bandwidth */
    u_int8_t    num_tones;   /* number of tones (subcarriers) */
    u_int8_t    nr;         /* number of receiving antennas */
    u_int8_t    nc;         /* number of transmitting anteannas */
    
    
    u_int8_t    rssi;        /* rx frame RSSI */
    u_int8_t    rssi_ctl0;   /* rx frame RSSI [ctl, chain 0] */
    u_int8_t    rssi_ctl1;   /* rx frame RSSI [ctl, chain 1] */
    u_int8_t    rssi_ctl2;   /* rx frame RSSI [ctl, chain 2] */
    //u_int8_t    rssi_ext0;   /* rx frame RSSI [ext, chain 0] */
    //u_int8_t    rssi_ext1;   /* rx frame RSSI [ext, chain 1] */
    //u_int8_t    rssi_ext2;   /* rx frame RSSI [ext, chain 2] */
   
    //u_int8_t    not_sounding;
    //u_int8_t    hw_upload_data;
    //u_int8_t    hw_upload_data_valid;
    //u_int8_t    hw_upload_data_type;
};

struct ath9k_csi {
	
    struct csi_pkt_status pkt_status;
    u_int8_t csi_buf[2800];     //buffer for csi value, 3 antena, each with 114 subcarriers, real and imagine part
    u_int8_t payload_buf[1500]; //buffer for the payload
    u_int16_t payload_len;

};
static int __init csi_init(void);

static void __exit csi_exit(void);

void   csi_record_payload(void* data, u_int16_t data_len);

void   csi_record_status(struct ath_hw *hw, struct ath_rx_status *rxs,struct ar9003_rxs *rxsp,void* data); 

//u_int8_t check_status(void);
int check_status(void);

