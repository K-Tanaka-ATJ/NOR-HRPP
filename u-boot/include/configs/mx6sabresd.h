/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __MX6QSABRESD_CONFIG_H
#define __MX6QSABRESD_CONFIG_H

#ifdef CONFIG_SPL
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#include "imx6_spl.h"
#endif

#define CONFIG_MACH_TYPE    3980
#define CONFIG_MXC_UART_BASE    UART1_BASE
#define CONFIG_CONSOLE_DEV      "ttymxc0"
#define CONFIG_MMCROOT          "/dev/mmcblk1p2"
#if defined(CONFIG_MX6Q)
#define CONFIG_DEFAULT_FDT_FILE "imx6q-sabresd.dtb"
#elif defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#define CONFIG_DEFAULT_FDT_FILE "imx6dl-sabresd.dtb"
#endif
/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
#define IMX6SOLO_PP

/* for TOYOTA-PP <kuri0426>*/
/* TOYOTA向けの場合、下のコメントを外してフルビルドすること */
/* #define BSP_SUPPORT_TOYOTA */

/* 機種タイプを自動判別するマクロ <kuri0706> */
/* これにより、前述のTOYOTA向け区別のマクロは不要 */
#define BSP_SUPPORT_AUTOSELECT

/* 機種タイプを自動判別するマクロ有効時に判別結果をTOYOTA固定するマクロ <kuri0906>*/
/* 前述の自動判別マクロが有効時のみ本マクロも有効となる */
/* #define BSP_SELECT_FIX_TOYOTA */

/* for version check <kuri0428> */
/* OS書き込み時にBootLoaderとOS間で整合性をチェックする */
#define BSP_SUPPORT_VERCHK

/* for version check <kuri0512> ver0.16 */
/* OSをメディアから読み出し時にSUM ERRORだったらエラーアクションを変える修正 */
#define BSP_SUPPORT_SUMERR

#ifdef IMX6SOLO_PP
#define PHYS_SDRAM_SIZE     (1u * 256 * 1024 * 1024)
#else
/* #define PHYS_SDRAM_SIZE      (1u * 1024 * 1024 * 1024) */
#define PHYS_SDRAM_SIZE     (512 * 1024 * 1024)
#endif
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */

#include "mx6sabre_common.h"

/* +<kuri0423> */
#if 0	/*<org>*/
#define CONFIG_SYS_FSL_USDHC_NUM    4
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV      3   /* SDHC1 */
#endif
#else	/*<org>*/
#define CONFIG_SYS_FSL_USDHC_NUM    1	/* 4 -> 1 */
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV      0   /* 3 -> 0*/ /* SDHC1 */
#endif
#endif	/*<org>*/
/* -<kuri0423> */

/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
#ifdef IMX6SOLO_PP
#define CONFIG_SYS_PROMPT       "MX6Solo_PP U-Boot > "

/* +Modified for iMX6Solo_PP at 2015/09/11 by Akita */
#define CONFIG_BOARD_EARLY_INIT_F
/* -Modified for iMX6Solo_PP at 2015/09/11 by Akita */

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK             66000000

/* WEIM NOR Flash Configs */
/* +Modified by NOR patch 2015/09/14 */
#ifdef CONFIG_CMD_FLASH
    #undef CONFIG_SYS_NO_FLASH
    #define CONFIG_SYS_FLASH_BASE           WEIM_ARB_BASE_ADDR
    #define CONFIG_SYS_FLASH_SECT_SIZE      (128 * 1024)
    #define CONFIG_SYS_MAX_FLASH_BANKS 1    /* max number of memory banks */
    #define CONFIG_SYS_MAX_FLASH_SECT 1024  /* max number of sectors on one chip */
    #define CONFIG_SYS_FLASH_CFI            /* Flash memory is CFI compliant */
    #define CONFIG_FLASH_CFI_DRIVER         /* Use drivers/cfi_flash.c */
    #define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* Use buffered writes*/
    #define CONFIG_SYS_FLASH_EMPTY_INFO
#endif
/* -Modified by NOR patch 2015/09/14 */
#endif /* IMX6SOLO_PP */
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */

/* #define CONFIG_CMD_PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO  IMX_GPIO_NR(7, 12)
#define CONFIG_PCIE_IMX_POWER_GPIO  IMX_GPIO_NR(3, 19)
#endif

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C3     /* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED          100000

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR  0x08

/* USB Configs */
#define CONFIG_CMD_USB
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC       (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS        0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2 /* Enabled USB controller number */
#endif

/* +Modified for iMX6Solo_PP at 2015/09/09 by Akita */
#ifdef IMX6SOLO_PP
/*
 * SPLASH SCREEN Configs
 */
#ifdef CONFIG_SPLASH_SCREEN
    /*
     * Framebuffer and LCD
     */
    /*
     * iMX6Solo_PP_BSP image_cfg.h
     * #define IMAGE_BOOT_BMP_BUFFER_PA_START      (0x1FE00000)
     */
    /* <kuri1012> #define CONFIG_FB_BASE              (TEXT_BASE + 0x7600000)*/
    /* BMP:0x1F20_0000 = TEXT_BASE(0x1780_0000) + 0x07A0_0000 */
    #define CONFIG_FB_BASE              (TEXT_BASE + 0x7A00000)	 /*<kuri1012>*/
    #define CONFIG_CMD_BMP
#endif /* CONFIG_SPLASH_SCREEN */
#endif
/* -Modified for iMX6Solo_PP at 2015/09/09 by Akita */

#endif                         /* __MX6QSABRESD_CONFIG_H */
