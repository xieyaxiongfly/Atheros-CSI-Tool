# Atheros-CSI-Tool
**Research-grade Wi-Fi CSI extraction for commodity Atheros 802.11n (ath9k) NICs on Linux**

[![Platform](https://img.shields.io/badge/platform-Linux-blue)](https://wands.hk/external/wifi/AtherosCSI/index.html)
[![Driver](https://img.shields.io/badge/driver-ath9k-informational)](https://wands.hk/external/wifi/AtherosCSI/index.html)
[![License](https://img.shields.io/badge/license-GPL-green)](COPYING)

> **Authorship / Origin**  
> Developed by **Yaxiong Xie** under the supervision of **Prof. Mo Li** (PhD advisor), WANDS group.

---

## Quick links
- 🌐 **Project website (docs / setup / examples / citation):** https://wands.hk/external/wifi/AtherosCSI/index.html  
- 🧰 **User-space capture & parser tools:** https://github.com/xieyaxiongfly/Atheros-CSI-Tool-UserSpace-APP

---

## Table of contents
- [What this is](#what-this-is)
- [Why CSI / what you can do](#why-csi--what-you-can-do)
- [Key features](#key-features)
- [Hardware support](#hardware-support)
- [Getting started](#getting-started)
- [CSI format](#csi-format)
- [Reproducibility notes](#reproducibility-notes)
- [Citation](#citation)
- [Contributing](#contributing)
- [Credits](#credits)
- [License](#license)

---

## What this is
**Atheros-CSI-Tool** extracts **fine-grained Wi-Fi Channel State Information (CSI)** from
off-the-shelf **Atheros 802.11n** NICs supported by the Linux **`ath9k`** driver stack.

It enables reproducible experiments on commodity platforms (PCs / embedded boards / routers), and has been
widely used by the research community for Wi-Fi sensing, localization, and wireless measurement.

---

## Why CSI / what you can do
CSI provides per-subcarrier complex channel responses at the PHY layer and enables research in:

| Area | Example topics |
|---|---|
| Wi-Fi sensing | activity recognition, gesture, respiration, tracking |
| Localization & analytics | fingerprinting, channel analytics, AoA/ToF-related processing |
| Wireless measurement | debugging, environment characterization, cross-layer studies |
| Security & robustness | device/channel signatures, anomaly detection |

---

## Key features
- ✅ **Per-packet CSI** (complex-valued across OFDM subcarriers)
- ✅ **Rich metadata** (platform/config dependent): timestamps, RSSI per chain, rate/MCS, payload, driver-level info
- ✅ **Commodity-friendly**: built on `ath9k` (**no firmware hacks**)
- ✅ **Multi-receiver ready**: one TX packet can be collected by multiple RX nodes (each logging CSI)
- ✅ **Tooling ecosystem**: pair kernel-side extraction with user-space logging/parsing utilities

---

## Hardware support
Target: **Atheros 802.11n chipsets supported by `ath9k`**.

Validated chipsets (commonly used in the community):

| Chipset | Status |
|---|---|
| AR9580 | ✅ validated |
| AR9590 | ✅ validated |
| AR9344 | ✅ validated |
| QCA9558 | ✅ validated |

For recommended NIC/router models and practical setup notes, see:  
https://wands.hk/external/wifi/AtherosCSI/index.html

---

## Getting started
This repository provides the **kernel-side components**. For end-to-end CSI logging (setup → capture → parse → export),
follow the documentation portal:

1. Read docs & pick your platform (Ubuntu / OpenWRT)  
   https://wands.hk/external/wifi/AtherosCSI/index.html
2. Install/build according to the website guide
3. Use the user-space tools to capture/parse CSI  
   https://github.com/xieyaxiongfly/Atheros-CSI-Tool-UserSpace-APP

---

## CSI format
In OFDM Wi-Fi systems, CSI is typically represented as a complex tensor per received packet:

- **20 MHz**: often `Ntx × Nrx × 56` subcarriers  
- **40 MHz**: often `Ntx × Nrx × 114` subcarriers  

Exact dimensions and header fields depend on bandwidth / NIC / configuration.  
For detailed format explanation & examples, see:  
https://wands.hk/external/wifi/AtherosCSI/index.html

---

## Reproducibility notes
If you are collecting datasets or evaluating algorithms, we recommend logging and reporting:

- **Hardware**: NIC/router model + chipset  
- **Software**: OS image, kernel version, `ath9k` version/commit  
- **Radio config**: channel, bandwidth, TX power, antenna chains, MCS/rate settings  
- **Data**: raw CSI + per-packet metadata (rate, RSSI, timestamps) and parsing scripts  

---

## Citation
If you use Atheros-CSI-Tool in academic work, please cite the tool page and any associated references listed there:  
https://wands.hk/external/wifi/AtherosCSI/index.html

---

## Contributing
Contributions are welcome—especially:
- expanding chipset/platform coverage
- improving documentation & troubleshooting
- adding parsers/exporters and reproducible experiment scripts

Please open an issue before large changes.

---

## Credits
- **Yaxiong Xie** — original developer & maintainer  
- **Prof. Mo Li** — supervision and research direction (PhD advisor)

If this project supports your research, starring the repo helps others discover it ⭐

---

## License
GPL (see `COPYING`).
