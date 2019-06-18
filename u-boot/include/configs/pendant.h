/*++

    Copyright 2015. OMRON NOHGATA Co.,Ltd. All Rights Reserved.

Module Name:

    pendant.h - Description: Definitions for iMX6Solo_PP BSP.

Abstract:


Revision History:
About this revision         Date         Author
--------------------------  -----------  ----------------------------------
first created.              2015/06/18   Kosuke Akita

--*/

#ifndef __PENDANT_H
#define __PENDANT_H

#if __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
//  Define:
//
//  Defines bootloader error code.
//
#define ERROR_PP_BL_NO_ERROR            0   // No error.
#define ERROR_PP_BL_NO_USB_SD           1   // USB memory and SD card not found.
#define ERROR_PP_BL_USB_INVALID_FS      2   // Invalid file system on USB memory.
#define ERROR_PP_BL_SD_INVALID_FS       3   // Invalid file system on SD card.
#define ERROR_PP_BL_USB_NO_REFTEXT      4   // Reference text file not found on USB memory.
#define ERROR_PP_BL_SD_NO_REFTEXT       5   // Reference text file not found on SD card.
#define ERROR_PP_BL_USB_NO_OSIMG        6   // OS image not found on USB memory.
#define ERROR_PP_BL_SD_NO_OSIMG         7   // OS image not found on SD card.
#define ERROR_PP_BL_FLASH_NO_OSIMG      8   // OS image not found on NOR Flash ROM.
#define ERROR_PP_BL_FLASH_ERROR         9   // Out of disk space or HW issue on NOR Flash ROM.

#ifdef	BSP_SUPPORT_SUMERR	//<kuri0512>
#define ERROR_PP_BL_NK_SUM_ERROR        10   // NK.BIN bad format (SUM ERROR)
#endif	//BSP_SUPPORT_SUMERR  <kuri0512>

#ifdef	BSP_SUPPORT_VERCHK	//<kuri0428>
#define ERROR_PP_BL_BOOT_VER_USB         8   // OS image allow boot version unmatch from USB.
#define ERROR_PP_BL_BOOT_VER_SD          9   // OS image allow boot version unmatch from SD.
#define ERROR_PP_BL_BOOT_VER_UNKNOWN     10  // OS image allow boot version unmatch from UNKNOWN DEVICE.
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

//
//  Defines OS boot mode.
//
#define BOOT_CFG_BOOTMODE_0     0               // 通常起動
#define BOOT_CFG_BOOTMODE_1     1               // “5” + “低”
#define BOOT_CFG_BOOTMODE_2     2               // “2” + “8”
#define BOOT_CFG_BOOTMODE_3     3               // “高速” + “低”
#define BOOT_CFG_BOOTMODE_4     4               // “5” + “高速” + “低”
#define BOOT_CFG_BOOTMODE_5     5               // “2” + “8” + “高速”
#define BOOT_CFG_BOOTMODE_6     6               // “.” + “-”
#define BOOT_CFG_BOOTMODE_7     7               // “.” + “高速”
#define BOOT_CFG_BOOTMODE_8     8               // “(右)シフト” + “ENTER”<ver0.09> 自己診断用

#define BOOT_DEVICE_SD          0
#define BOOT_DEVICE_NOR         5

#define BOOT_STORAGE_USB        1           // OS image was loaded from USB memory.
#define BOOT_STORAGE_SD         2           // OS image was loaded from SD card.

#define CSP_BASE_MEM_PA_EIM_CS0             (0x08000000UL)
#define BSP_NOR_FLASH_128MB

//------------------------------------------------------------------------------
// NOR Flash ROM image defines
#define IMAGE_BOOT_NORDEV_NOR_PA_START      CSP_BASE_MEM_PA_EIM_CS0  // 64MB or 128MB NOR flash on EIM CS0

#ifdef BSP_NOR_FLASH_128MB
#define IMAGE_BOOT_NORDEV_NOR_SIZE          (128*1024*1024)
#else
#define IMAGE_BOOT_NORDEV_NOR_SIZE          (64*1024*1024)
#endif // BSP_NOR_FLASH_128MB
#define IMAGE_BOOT_NORDEV_NOR_PA_END        (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_BOOT_NORDEV_NOR_SIZE-1)

#define IMAGE_BOOT_UBOOTIMAGE_NOR_OFFSET    (0)
#define IMAGE_BOOT_UBOOTIMAGE_NOR_PA_START  (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_BOOT_UBOOTIMAGE_NOR_OFFSET)
#define IMAGE_BOOT_UBOOTIMAGE_NOR_SIZE      (512*1024)
#define IMAGE_BOOT_UBOOTIMAGE_NOR_PA_END    (IMAGE_BOOT_UBOOTIMAGE_NOR_PA_START+IMAGE_BOOT_UBOOTIMAGE_NOR_SIZE-1)

#define IMAGE_BOOT_EBOOTIMAGE_NOR_OFFSET    (IMAGE_BOOT_UBOOTIMAGE_NOR_OFFSET+IMAGE_BOOT_UBOOTIMAGE_NOR_SIZE)
#define IMAGE_BOOT_EBOOTIMAGE_NOR_PA_START  (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_BOOT_EBOOTIMAGE_NOR_OFFSET)
#define IMAGE_BOOT_EBOOTIMAGE_NOR_SIZE      (512*1024)
#define IMAGE_BOOT_EBOOTIMAGE_NOR_PA_END    (IMAGE_BOOT_EBOOTIMAGE_NOR_PA_START+IMAGE_BOOT_EBOOTIMAGE_NOR_SIZE-1)

// +Modified for iMX6Solo_PP at 2015/08/06 by Akita
#define IMAGE_BOOT_BMPIMAGE_NOR_OFFSET      (IMAGE_BOOT_EBOOTIMAGE_NOR_OFFSET+IMAGE_BOOT_EBOOTIMAGE_NOR_SIZE)
//#define IMAGE_BOOT_BMPIMAGE_NOR_OFFSET      (IMAGE_BOOT_NKIMAGE_NOR_OFFSET+IMAGE_BOOT_NKIMAGE_NOR_SIZE)
// -Modified for iMX6Solo_PP at 2015/08/06 by Akita
#define IMAGE_BOOT_BMPIMAGE_NOR_PA_START    (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_BOOT_BMPIMAGE_NOR_OFFSET)
#if 0	//<kuri0925->
#define IMAGE_BOOT_BMPIMAGE_NOR_SIZE        (896*1024)
#else	//<kuri0925->
#define IMAGE_BOOT_BMPIMAGE_NOR_SIZE        (1*1024*1024)	//<kuri0926+> BMP 16/24bit
#endif	//<kuri0925->
#define IMAGE_BOOT_BMPIMAGE_NOR_PA_END      (IMAGE_BOOT_BMPIMAGE_NOR_PA_START+IMAGE_BOOT_BMPIMAGE_NOR_SIZE-1)

#define IMAGE_WINCE_REGISTRY_NOR_OFFSET     (IMAGE_BOOT_BMPIMAGE_NOR_OFFSET+IMAGE_BOOT_BMPIMAGE_NOR_SIZE)
#define IMAGE_WINCE_REGISTRY_NOR_PA_START   (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_WINCE_REGISTRY_NOR_OFFSET)
#define IMAGE_WINCE_REGISTRY_NOR_SIZE       (1*1024*1024)
#define IMAGE_WINCE_REGISTRY_NOR_PA_END     (IMAGE_WINCE_REGISTRY_NOR_PA_START+IMAGE_WINCE_REGISTRY_NOR_SIZE-1)

// Last 128K of NOR reserved for boot configuration include MAC Address.
#define IMAGE_BOOT_BOOTCFG_NOR_OFFSET       (IMAGE_WINCE_REGISTRY_NOR_OFFSET+IMAGE_WINCE_REGISTRY_NOR_SIZE)
#define IMAGE_BOOT_BOOTCFG_NOR_SIZE         (128*1024)
#define IMAGE_BOOT_BOOTCFG_NOR_PA_START     (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_BOOT_BOOTCFG_NOR_OFFSET)
#define IMAGE_BOOT_BOOTCFG_NOR_PA_END       (IMAGE_BOOT_BOOTCFG_NOR_PA_START+IMAGE_BOOT_BOOTCFG_NOR_SIZE-1)

// +Modified for iMX6Solo_PP at 2015/07/31 by Akita
// Reserve 1MByte
#define IMAGE_SHARE_RESERVED_NOR_OFFSET     (IMAGE_BOOT_BOOTCFG_NOR_OFFSET+IMAGE_BOOT_BOOTCFG_NOR_SIZE)
#define IMAGE_SHARE_RESERVED_NOR_PA_START   (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_SHARE_RESERVED_NOR_OFFSET)
#if 0	//<kuri0925>
#define IMAGE_SHARE_RESERVED_NOR_SIZE       (1*1024*1024)
#else	//<kuri0925>
#define IMAGE_SHARE_RESERVED_NOR_SIZE       (896*1024)	//<kuri0926+> リザーブエリアは1Mbyte-128Kbyte=896Kbyte
#endif	//<kuri0925>
#define IMAGE_SHARE_RESERVED_NOR_PA_END     (IMAGE_SHARE_RESERVED_NOR_PA_START+IMAGE_SHARE_RESERVED_NOR_SIZE-1)
// -Modified for iMX6Solo_PP at 2015/07/31 by Akita

#if 1	//<kuri0918> リザーブエリアを細分化する(RESERVE定義と重ねているので注意)
//OSレングス保管エリア
#define IMAGE_OS_LENGTH_NOR_OFFSET          (IMAGE_BOOT_BOOTCFG_NOR_OFFSET+IMAGE_BOOT_BOOTCFG_NOR_SIZE)  //このブロックのオフセット：１つ前のブロックのオフセット＋レングス
#define IMAGE_OS_LENGTH_NOR_PA_START        (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_OS_LENGTH_NOR_OFFSET)  //このブロックの先頭アドレス：ROM先頭＋このブロックのオフセット
#define IMAGE_OS_LENGTH_NOR_SIZE            (128*1024)                                                   //このブロックのサイズ
#define IMAGE_OS_LENGTH_NOR_PA_END          (IMAGE_OS_LENGTH_NOR_PA_START+IMAGE_OS_LENGTH_NOR_SIZE-1)    //このブロックの最終アドレス：このブロックの先頭アドレス＋このブロックのレングス－１

//MACアドレス保管エリア
#define IMAGE_MAC_ADDR_NOR_OFFSET           (IMAGE_OS_LENGTH_NOR_OFFSET+IMAGE_OS_LENGTH_NOR_SIZE)       //このブロックのオフセット：１つ前のブロックのオフセット＋レングス
#define IMAGE_MAC_ADDR_NOR_PA_START         (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_MAC_ADDR_NOR_OFFSET)  //このブロックの先頭アドレス：ROM先頭＋このブロックのオフセット
#define IMAGE_MAC_ADDR_NOR_SIZE             (128*1024)                                                  //このブロックのサイズ
#define IMAGE_MAC_ADDR_NOR_PA_END           (IMAGE_MAC_ADDR_NOR_PA_START+IMAGE_MAC_ADDR_NOR_SIZE-1)     //このブロックの最終アドレス：このブロックの先頭アドレス＋このブロックのレングス－１
#endif	//<kuri0918> リザーブエリアを細分化する(RESERVE定義と重ねているので注意)

// +Modified for iMX6Solo_PP at 2015/08/06 by Akita
#define IMAGE_BOOT_NKIMAGE_NOR_OFFSET       (IMAGE_SHARE_RESERVED_NOR_OFFSET+IMAGE_SHARE_RESERVED_NOR_SIZE)
//#define IMAGE_BOOT_NKIMAGE_NOR_OFFSET       (IMAGE_BOOT_EBOOTIMAGE_NOR_OFFSET+IMAGE_BOOT_EBOOTIMAGE_NOR_SIZE)
// -Modified for iMX6Solo_PP at 2015/08/06 by Akita
#define IMAGE_BOOT_NKIMAGE_NOR_PA_START     (IMAGE_BOOT_NORDEV_NOR_PA_START+IMAGE_BOOT_NKIMAGE_NOR_OFFSET)
#ifdef BSP_NOR_FLASH_128MB
// +Modified for iMX6Solo_PP at 2015/08/06 by Akita
//<kuri1014>	#define IMAGE_BOOT_NKIMAGE_NOR_SIZE         (124*1024*1024) // MAX 124MB NK.bin image
//<kuri1016>    #define IMAGE_BOOT_NKIMAGE_NOR_SIZE         (114*1024*1024) // MAX 114MB NK.bin image	//<kuri1014>
#define IMAGE_BOOT_NKIMAGE_NOR_SIZE         (120*1024*1024) // MAX 114MB NK.bin image	//<kuri1016>
//#define IMAGE_BOOT_NKIMAGE_NOR_SIZE         (64*1024*1024) // 64MB normal image
// -Modified for iMX6Solo_PP at 2015/08/06 by Akita
#else
#define IMAGE_BOOT_NKIMAGE_NOR_SIZE         (61*1024*1024) // 61MB small image
#endif // BSP_NOR_FLASH_128MB
#define IMAGE_BOOT_NKIMAGE_NOR_PA_END       (IMAGE_BOOT_NKIMAGE_NOR_PA_START+IMAGE_BOOT_NKIMAGE_NOR_SIZE-1)

// +Modified for iMX6Solo_PP at 2015/08/06 by Akita
#define FILE_NAME_NK_BIN                    "NK.bin"
#define FILE_NAME_LOGO_BMP                  "Logo.bmp"
// -Modified for iMX6Solo_PP at 2015/08/06 by Akita

//2015/07/19 kuri NewPP+
#define KEYINFO_EXIST 1    //for KEYINFO FILE MAKE in SYSTEM Driver/WinCE
//2015/07/19 kuri NewPP-

typedef struct {
    unsigned int  bootMode;
    unsigned int  errorCode;
    unsigned int  bootDevice;
    unsigned int  strage;
// +Modified for iMX6Solo_PP at 2015/08/06 by Akita
    unsigned int  osImageSize;
// -Modified for iMX6Solo_PP at 2015/08/06 by Akita

#ifdef KEYINFO_EXIST    //2015/07/19 kuri NewPP+
    unsigned char KeyInfo[16];    //KS[0]-[7],KST,DUMMY(7)
#endif //KEYINFO_EXIST  //2015/07/19 kuri NewPP-
    unsigned char UbootVer[16];   //U-Boot version Info.
    unsigned char EbootVer[16];   //E-Boot version Info.
    
#ifdef	BSP_SUPPORT_VERCHK	//<kuri0428>
	//バウンダリ防止のため下の1byte定義よりも上に配置した
	unsigned char AllowBootVersion[8];
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

#if 1	//<kuri1014>
	unsigned char boardVersion;	//Board version infomation
#endif	//<kuri1014>

} BSP_BOOT_INFO, *PBSP_BOOT_INFO;


#if __cplusplus
}
#endif

#endif
