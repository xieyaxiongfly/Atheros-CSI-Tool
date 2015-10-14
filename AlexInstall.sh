#!/bin/bash
sudo cp $(pwd)/drivers/net/wireless/ath/ath9k/ar9003_csi.ko   /lib/modules/$(uname -r)/kernel/drivers/net/wireless/ath/ath9k/ar9003_csi.ko
sudo cp $(pwd)/drivers/net/wireless/ath/ath9k/ath9k_common.ko /lib/modules/$(uname -r)/kernel/drivers/net/wireless/ath/ath9k/ath9k_common.ko
sudo cp $(pwd)/drivers/net/wireless/ath/ath9k/ath9k_htc.ko    /lib/modules/$(uname -r)/kernel/drivers/net/wireless/ath/ath9k/ath9k_htc.ko
sudo cp $(pwd)/drivers/net/wireless/ath/ath9k/ath9k_hw.ko     /lib/modules/$(uname -r)/kernel/drivers/net/wireless/ath/ath9k/ath9k_hw.ko
sudo cp $(pwd)/drivers/net/wireless/ath/ath9k/ath9k.ko        /lib/modules/$(uname -r)/kernel/drivers/net/wireless/ath/ath9k/ath9k.ko
sudo cp $(pwd)/drivers/net/wireless/ath/ath.ko                /lib/modules/$(uname -r)/kernel/drivers/net/wireless/ath/ath.ko
