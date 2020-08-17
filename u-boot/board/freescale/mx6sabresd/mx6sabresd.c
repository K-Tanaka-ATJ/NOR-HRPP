/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"
#include <asm/arch/mx6-ddr.h>
#include <usb.h>

#include <asm/types.h>
#include <asm/arch/mx6-ddr.h>
/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
#ifdef IMX6SOLO_PP
#include <fat.h>
#include <configs/pendant.h>
#endif
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |           \
    PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |         \
    PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |            \
    PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |         \
    PAD_CTL_SRE_FAST  | PAD_CTL_HYS)
    
#if 1	//<kuri0523>
#define USDHC_100PU_PAD_CTRL (PAD_CTL_PUS_100K_UP |            \
    PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |         \
    PAD_CTL_SRE_FAST  | PAD_CTL_HYS)
#endif	//<kuri0523>

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |           \
    PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
              PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |            \
    PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |   \
    PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define I2C_PMIC    0

#define I2C_PAD MUX_PAD_CTRL(I2C_PAD_CTRL)

#if 1	//<kuri0413>
	//非常停止SWとデッドマンSWは、LAN-PHY-RESETはPULL-DOWN設定にすること
#define	GPIO_PD_PAD_CTRL	(PAD_CTL_SRE_SLOW | PAD_CTL_DSE_40ohm | \
    PAD_CTL_SPEED_MED | PAD_CTL_PUS_100K_DOWN | PAD_CTL_HYS)
//参考：CEドライバ側
//		DDKIomuxSetPadConfig(DDK_IOMUX_PIN_SD3_DAT7, 
//			DDK_IOMUX_PAD_SLEW_SLOW, DDK_IOMUX_PAD_DRIVE_40_OHM, DDK_IOMUX_PAD_SPEED_MED_100MHZ,
//			DDK_IOMUX_PAD_ODT_NULL, DDK_IOMUX_PAD_OPENDRAIN_DISABLE, DDK_IOMUX_PAD_PULL_UP_47K, 
//			DDK_IOMUX_PAD_HYSTERESIS_ENABLE, DDK_IOMUX_PAD_DDRINPUT_NULL, DDK_IOMUX_PAD_DDRSEL_NULL);

#endif	//<kuri0413>

#if 1	//<kuri0523>
	//PULL-UP設定用
#define	GPIO_PU_PAD_CTRL	(PAD_CTL_SRE_SLOW | PAD_CTL_DSE_40ohm | \
    PAD_CTL_SPEED_MED | PAD_CTL_PUS_100K_UP | PAD_CTL_HYS)

#define	GPIO_47PU_PAD_CTRL	(PAD_CTL_SRE_SLOW | PAD_CTL_DSE_40ohm | \
    PAD_CTL_SPEED_MED | PAD_CTL_PUS_47K_UP | PAD_CTL_HYS)

#endif	//<kuri0523>

/* +Modified for iMX6Solo_PP at 2015/09/09 by Akita */
#ifdef IMX6SOLO_PP
#define DISP0_LCD_POWER IMX_GPIO_NR(2, 13)
#define DISP0_LCD_RESET IMX_GPIO_NR(2, 14)
#else
#define DISP0_PWR_EN    IMX_GPIO_NR(1, 21)
#endif /* IMX6SOLO_PP */
/* -Modified for iMX6Solo_PP at 2015/09/09 by Akita */

/* +Modified for iMX6Solo_PP at 2015/08/18 by Akita */
#ifdef IMX6SOLO_PP
#define WEIM_NOR_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |          \
    PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
    PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)
    
enum boot_device {
    WEIM_NOR_BOOT,
    ONE_NAND_BOOT,
    PATA_BOOT,
    SATA_BOOT,
    I2C_BOOT,
    SPI_NOR_BOOT,
    SD_BOOT,
    MMC_BOOT,
    NAND_BOOT,
    UNKNOWN_BOOT,
    BOOT_DEV_NUM = UNKNOWN_BOOT,
};

// NewPP+ 20150727 
//                                  123456789012345

#ifdef BSP_SUPPORT_AUTOSELECT	//<kuri0706> 自動判別対応

unsigned char g_AutoSel;	//機種自動判別結果保管。=0:標準、=1:TOYOTA

//TOYOTA向けバージョン情報           123456789012345
//const unsigned char UBOOT_VERT[] = {"1.00T          "};	//TOYOTA version. 
const unsigned char UBOOT_VERT[] = {"1.01T          "};	//TOYOTA version. --> PMUレジスタ値変更<tanaka20200114>
//標準版バージョン情報               123456789012345
//const unsigned char UBOOT_VER[] =  {"1.01           "};	//Original version. 
const unsigned char UBOOT_VER[] =  {"1.02           "};	//Original version.  --> PMUレジスタ値変更<tanaka20200114>
#else	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応

#ifdef	BSP_SUPPORT_TOYOTA	//<kuri0426>
//TOYOTA向けバージョン情報
const unsigned char UBOOT_VER[] = {"0.40T          "};	//Original version. 
#else	//BSP_SUPPORT_TOYOTA <kuri0426>
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20190819>
//バージョン情報
const unsigned char UBOOT_VER[] = {"1.00H          "};
#else	//BSP_SUPPORT_HIRATA <tanaka20190819>
//標準版バージョン情報
const unsigned char UBOOT_VER[] = {"1.00           "};	//Original version. 
#endif	//BSP_SUPPORT_HIRATA <tanaka20190819>
#endif	//BSP_SUPPORT_TOYOTA <kuri0426>

#endif	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応
//ver0.06 -----------------------------------------------------------------
// USB/SDからのNK.BIN高速読み出し対応
// USBのpower-ENポート(GPIO02-10)を読み出し、PCBの試作バージョンを区別対応
// (ただしBspArgsには入れない。E-BOOT側はE-BOOTでまた読み出すこととする。
//  U-BOOTなしでSD-BOOTさせることがあるため。)
// ------------------------------------------------------------------------
//ver0.07 -----------------------------------------------------------------
// 起動キーとそれ以外のキーを押していても起動キーが有効になる問題の修正。
// ------------------------------------------------------------------------
//ver0.08 -----------------------------------------------------------------
// ファイルサイズオーバー時にエラーにならず読み出してしまう問題の修正。
// ------------------------------------------------------------------------
//ver0.09 -----------------------------------------------------------------
// PCB自己診断用の起動モード追加。
// ------------------------------------------------------------------------
//ver0.10 -----------------------------------------------------------------
// SDカード電源をOFF->ONする処理を追加。(GPIO2_08 Low:OFF,High:ON)
// ------------------------------------------------------------------------
//ver0.11 -----------------------------------------------------------------
// 2016/04/13
// 3次試作の自動判別処理追加(2次試作と判断後、GPIO4_IO05をさらに読む)
// 2次試作以前と3次試作以降でENET関連の設定を変更する（RGMIIとRMII）
// ハードから指摘されているポート等の設定値見直し(SW関連のPULL-UP/DOWNなど)
// ------------------------------------------------------------------------
//ver0.12 -----------------------------------------------------------------
// 2016/04/13
// NOR-FLASHタイミングのマージン確保(ハード側からの指定を反映)
// 2016/04/21
// common\usb_storage.c のトランスファレングスを65535->256に変更
// usb_powerポートをバージョン確認で読み取り後に電源OFF状態に設定する。
// ------------------------------------------------------------------------
//ver0.13 -----------------------------------------------------------------
// 2016/04/23
// SDカードデフォルトスロット番号を3->0に変更し、SD1以外のSD兼用ポートを
// SDスロットとして設定しないよう修正。
// 合わせて、LAN-PHYのRGMII設定、PCI設定を行わないよう修正。
// ------------------------------------------------------------------------
//ver0.14/0.14T------------------------------------------------------------
// 2016/04/26
// TOYOTA仕様の追加。
// u-boot/include/configs/mx6sabresd.h において、
// #define BSP_SUPPORT_TOYOTA
// 定義を有効化してビルドすると、TOYOTA仕様としてビルドされる。
// ・キー配列が異なる
// ・LEDが異なる
// ・バージョン情報は、通常のバージョンの後ろに"T"が付加される（暫定）
// また、LAN-PHY_RESET信号(GPIO2_IO11)においてPULL-UP設定が残っていたのを
// 100K-PULL-DOWNに設定しなおし。
// ------------------------------------------------------------------------
//ver0.15/0.15T------------------------------------------------------------
// 2016/04/28
// OS書き込み時のBOOTバージョンチェック機能に対応。
// （BSP_SUPPORT_VERCHK 定義時に有効）
// ------------------------------------------------------------------------
//ver0.16/0.16T------------------------------------------------------------
// 2016/05/12
// OS読み込み時にSUM ERRORだったときの返り値を追加。
// （BSP_SUPPORT_SUMERR 定義時に有効）
// ------------------------------------------------------------------------
// ver0.20/0.20T-----------------------------------------------------------
// 2016/5/17
// 上記バージョンチェック版を正式化し、以前と区別するためマイナーバージョンを
// 0.20にあげる。
// ------------------------------------------------------------------------
// ver0.21/0.21T-----------------------------------------------------------
// 2016/5/23
// GPIOポートのPULL-UP/PULL-DOWN設定をハード指定に合わせる。
// 0.20と動作互換はあるので、バージョンは0.21とした。
// メディアからOSを読み出す際に、USBでNK.BINが無い以外のエラーの場合は
// SDカードがあればエラーを無視してSD側をチェックするよう修正。
// （YPPのBootLoader仕様がそのようになっているので合わせる）
// ------------------------------------------------------------------------
// ver0.30/0.30T-----------------------------------------------------------
// 2016/5/28
// DRAMコンフィグレーションをEBOOTに合わせる（DRAM256以上認識しない問題の解消）
// ------------------------------------------------------------------------
// ver0.40/0.40T-----------------------------------------------------------
// 2016/5/31
// USBにTXTあり、BINなし、SDカード無しのとき通常起動する不具合修正。
// USBにTXTありでBINなしの場合はUSBでエラー確定。（YPPで確認済み）
// ------------------------------------------------------------------------
// ver1.00/0.40T-----------------------------------------------------------
// 2016/6/7
// 標準版正式化
// ------------------------------------------------------------------------
// ver1.10/1.00T-----------------------------------------------------------
// 2016/7/6～2016/9/5
// 標準版とTOYOTA版をキーボードの折り返しで判別する機能の追加。正式版。
// BSP_SUPPORT_AUTOSELECT が定義されていれば有効。
// ------------------------------------------------------------------------
// ver1.02/1.01T-----------------------------------------------------------
// 2020/01/20
// PMU_REG_CORE、PMU_REG_3P0の設定処理を追加
// （ブルースクリーン不具合対策）
// ver1.00H---------------------------------------------------------------
// 2019/8/19～2019//
//      処理を追加。
//      mx6sabresd.hは以下のように定義を変更すること
//      BSP_SUPPORT_AUTOSELECT 無効
//      BSP_SUPPORT_TOYOTA        無効
//      BSP_SUPPORT_HIRATA         有効
//      BSP_SUPPORT_VERCHK        無効
// ------------------------------------------------------------------------
// NewPP+ 20150727 

#if 1	//<kuri0926>
#define		FASTLOAD	TRUE
#endif	//<kuri0926>

#define BSP_BOARD_3RD 1		//<kuri0413>

/* +Modified for iMX6Solo_PP at 2015/09/09 by Akita */
extern int pwm_init(int pwm_id, int div, int invert);
extern int pwm_config_direct(int pwm_id, u32 sample, u32 period);
extern int pwm_enable(int pwm_id);
extern void pwm_disable(int pwm_id);

extern void go_usb_start(void);	//<kuri0421> Warning削除のため定義

#if 0	//<kuri0926> Not Use WARNING Modify
static void DumpMemory(u32* pAddr, int words);
#endif	//<kuri0926> Not Use WARNING Modify

void BlinkAllLEDs(int on);
int os_boot_submain(void);

void SW_LED(int on);	//<kuri0926>
void print_pmu_reg(void);	//<tanaka20200114>


/* -Modified for iMX6Solo_PP at 2015/09/09 by Akita */

static enum boot_device boot_dev;

#if 1	//<kuri1014>
unsigned char Board_Version;	//Board version infomation
#endif	//<kuri1014>

#ifdef	BSP_SUPPORT_VERCHK	//<kuri0428>
unsigned char Allow_Boot_Version[8];	//NK.binの先頭に定義してある
										//BootLoaderバージョン許可情報保管エリア
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

static inline void setup_boot_device(void)
{
    uint soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
    uint bt_mem_ctl = (soc_sbmr & 0x000000FF) >> 4 ;
    uint bt_mem_type = (soc_sbmr & 0x00000008) >> 3;

    switch (bt_mem_ctl) {
    case 0x0:
        if (bt_mem_type)
            boot_dev = ONE_NAND_BOOT;
        else
            boot_dev = WEIM_NOR_BOOT;
        break;
    case 0x2:
            boot_dev = SATA_BOOT;
        break;
    case 0x3:
        if (bt_mem_type)
            boot_dev = I2C_BOOT;
        else
            boot_dev = SPI_NOR_BOOT;
        break;
    case 0x4:
    case 0x5:
        boot_dev = SD_BOOT;
        break;
    case 0x6:
    case 0x7:
        boot_dev = MMC_BOOT;
        break;
    case 0x8 ... 0xf:
        boot_dev = NAND_BOOT;
        break;
    default:
        boot_dev = UNKNOWN_BOOT;
        break;
    }
}

enum boot_device get_boot_device(void)
{
    return boot_dev;
}
#endif
/* -Modified for iMX6Solo_PP at 2015/08/18 by Akita */

int dram_init(void)
{
    gd->ram_size = imx_ddr_size();
    return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
    MX6_PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
    MX6_PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads[] = {
    MX6_PAD_ENET_MDIO__ENET_MDIO        | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET_MDC__ENET_MDC      | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_TXC__RGMII_TXC    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_TD0__RGMII_TD0    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_TD1__RGMII_TD1    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_TD2__RGMII_TD2    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_TD3__RGMII_TD3    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL  | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_ENET_REF_CLK__ENET_TX_CLK   | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_RXC__RGMII_RXC    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_RD0__RGMII_RD0    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_RD1__RGMII_RD1    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_RD2__RGMII_RD2    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_RD3__RGMII_RD3    | MUX_PAD_CTRL(ENET_PAD_CTRL),
    MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL  | MUX_PAD_CTRL(ENET_PAD_CTRL),
#if 0	//<kuri0413>
    /* AR8031 PHY Reset */
    MX6_PAD_ENET_CRS_DV__GPIO1_IO25     | MUX_PAD_CTRL(NO_PAD_CTRL),
#else	//<kuri0413>
    MX6_PAD_SD4_DAT3__GPIO2_IO11      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),      //PHY-Reset
#endif	//<kuri0413>
};

/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
#ifdef IMX6SOLO_PP

static int bootMode;
static void SetupBootMode(void);

#if BSP_BOARD_3RD	//<kuri0413>
static iomux_v3_cfg_t const enet_pads_3rd[] = {
    MX6_PAD_ENET_MDIO__ENET_MDIO      | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-22
                                                                        //1-23 No Use
    MX6_PAD_ENET_RX_ER__ENET_RX_ER    | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-24
    MX6_PAD_ENET_CRS_DV__ENET_RX_EN   | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-25
    MX6_PAD_ENET_RXD1__ENET_RX_DATA1  | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-26
    MX6_PAD_ENET_RXD0__ENET_RX_DATA0  | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-27
    MX6_PAD_ENET_TX_EN__ENET_TX_EN    | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-28
    MX6_PAD_ENET_TXD1__ENET_TX_DATA1  | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-29
    MX6_PAD_ENET_TXD0__ENET_TX_DATA0  | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-30
    MX6_PAD_ENET_MDC__ENET_MDC        | MUX_PAD_CTRL(ENET_PAD_CTRL),	//1-31
    
    MX6_PAD_SD4_DAT3__GPIO2_IO11      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL), //PHY-Reset
};

//ボードバージョンを読み取るためのポート関連
static iomux_v3_cfg_t const gpio_pads_fast[] = {
	MX6_PAD_SD4_DAT2__GPIO2_IO10    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO2_IO10 : 1st or 2nd
	MX6_PAD_GPIO_19__GPIO4_IO05     | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO05 : 2nd or 3rd
};

//GPIOポート関連設定
static iomux_v3_cfg_t const gpio_pads_3rd[] = {
    /* GPIO1 */
#if 0	//<kuri0523>
    MX6_PAD_GPIO_2__GPIO1_IO02      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止1
    MX6_PAD_GPIO_6__GPIO1_IO06      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止2
    MX6_PAD_SD2_CLK__GPIO1_IO10     | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止3
    MX6_PAD_SD2_DAT1__GPIO1_IO14    | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止4
    MX6_PAD_SD2_DAT0__GPIO1_IO15    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_ENET_REF_CLK__GPIO1_IO23| MUX_PAD_CTRL(NO_PAD_CTRL), //GPIO1_IO23はポート（未使用）
	// GPIO1_IO24～IO30はENETで使用する
#else	//<kuri0523>
    MX6_PAD_GPIO_0__GPIO1_IO00      | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR5(PU追加)
    MX6_PAD_GPIO_2__GPIO1_IO02      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止1
    MX6_PAD_GPIO_4__GPIO1_IO04      | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR7(PU追加)
    MX6_PAD_GPIO_6__GPIO1_IO06      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止2
    MX6_PAD_SD2_CLK__GPIO1_IO10     | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止3
    MX6_PAD_SD2_DAT3__GPIO1_IO12    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR6(PU追加)
    MX6_PAD_SD2_DAT1__GPIO1_IO14    | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止4
    MX6_PAD_SD2_DAT0__GPIO1_IO15    | MUX_PAD_CTRL(GPIO_47PU_PAD_CTRL),	//Remote-TR有無(47PU変更)
    MX6_PAD_ENET_REF_CLK__GPIO1_IO23| MUX_PAD_CTRL(NO_PAD_CTRL), //GPIO1_IO23はポート（未使用）
	// GPIO1_IO24～IO30はENETで使用する
#endif	//<kuri0523>


    /* GPIO2 */
//<kuri0426>    MX6_PAD_SD4_DAT3__GPIO2_IO11    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD4_DAT3__GPIO2_IO11    | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//<kuri0426>
    MX6_PAD_SD4_DAT4__GPIO2_IO12    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD4_DAT6__GPIO2_IO14    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD4_DAT7__GPIO2_IO15    | MUX_PAD_CTRL(NO_PAD_CTRL),

    /* GPIO3 */
    MX6_PAD_EIM_D16__GPIO3_IO16     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D17__GPIO3_IO17     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D18__GPIO3_IO18     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D19__GPIO3_IO19     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D20__GPIO3_IO20     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D22__GPIO3_IO22     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D23__GPIO3_IO23     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D26__GPIO3_IO26     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D27__GPIO3_IO27     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D29__GPIO3_IO29     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D30__GPIO3_IO30     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D31__GPIO3_IO31     | MUX_PAD_CTRL(NO_PAD_CTRL),
    
#if	1	//<kuri0523>
    MX6_PAD_KEY_COL0__GPIO4_IO06    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR0(PU追加)
    MX6_PAD_KEY_COL1__GPIO4_IO08    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR1(PU追加)
    MX6_PAD_KEY_COL2__GPIO4_IO10    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR2(PU追加)
    MX6_PAD_KEY_COL3__GPIO4_IO12    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR3(PU追加)
    MX6_PAD_KEY_COL4__GPIO4_IO14    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR4(PU追加)
#endif	//<kuri0523>

    /* GPIO5 */
    MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT21__GPIO5_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT22__GPIO5_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT23__GPIO5_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_CSI0_PIXCLK__GPIO5_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_CSI0_MCLK__GPIO5_IO19   | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//DEADMAN1

    /* GPIO6 */
    MX6_PAD_NANDF_CS1__GPIO6_IO14   | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//DEADMAN2
    MX6_PAD_NANDF_CS2__GPIO6_IO15   | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_NANDF_CS3__GPIO6_IO16   | MUX_PAD_CTRL(NO_PAD_CTRL),
	//GPIO6_IO17～IO19、GPIO7_IO00が3次試作ではIOポート
#if 0	//<kuri0523>
	MX6_PAD_SD3_DAT7__GPIO6_IO17    | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD3_DAT6__GPIO6_IO18    | MUX_PAD_CTRL(NO_PAD_CTRL),
#else	//<kuri0523>
	MX6_PAD_SD3_DAT7__GPIO6_IO17    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//DEADMAN ERROR(PU追加)
	MX6_PAD_SD3_DAT6__GPIO6_IO18    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//PMIC INT(未使用)(PU追加)
#endif	//<kuri0523>
	MX6_PAD_RGMII_TXC__GPIO6_IO19   | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD3_DAT5__GPIO7_IO00    | MUX_PAD_CTRL(NO_PAD_CTRL),
	
    /* GPIO7 */
    MX6_PAD_GPIO_16__GPIO7_IO11     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_GPIO_17__GPIO7_IO12     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_GPIO_18__GPIO7_IO13     | MUX_PAD_CTRL(NO_PAD_CTRL),
};

#endif	//BSP_BOARD_3RD <kuri0413>

static iomux_v3_cfg_t const gpio_pads[] = {
    /* GPIO1 */

#if 1	//<kuri0413> 非常停止SWを100K-PULL-DOWN
    MX6_PAD_GPIO_0__GPIO1_IO00      | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR5(PU追加) <kuri0523>
    MX6_PAD_GPIO_2__GPIO1_IO02      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止1
    MX6_PAD_GPIO_4__GPIO1_IO04      | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR7(PU追加) <kuri0523>
    MX6_PAD_GPIO_6__GPIO1_IO06      | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止2
    MX6_PAD_SD2_CLK__GPIO1_IO10     | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止3
    MX6_PAD_SD2_DAT3__GPIO1_IO12    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR6(PU追加) <kuri0523>
    MX6_PAD_SD2_DAT1__GPIO1_IO14    | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//非常停止4
#else	//<kuri0413> 非常停止SWを100K-PULL-DOWN
    MX6_PAD_GPIO_2__GPIO1_IO02      | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_GPIO_6__GPIO1_IO06      | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD2_CLK__GPIO1_IO10     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD2_DAT1__GPIO1_IO14    | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif	//<kuri0413> 非常停止SWを100K-PULL-DOWN
//<kuri0523>    MX6_PAD_SD2_DAT0__GPIO1_IO15    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD2_DAT0__GPIO1_IO15    | MUX_PAD_CTRL(GPIO_47PU_PAD_CTRL),	//Remote-TR有無(47PU変更) <kuri0523>
#if 1	//<kuri0413> デッドマンSWを100K-PULL-DOWN
    MX6_PAD_ENET_RX_ER__GPIO1_IO24  | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//DEADMAN1
    MX6_PAD_ENET_CRS_DV__GPIO1_IO25 | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//DEADMAN2
#else	//<kuri0413> デッドマンSWを100K-PULL-DOWN
    MX6_PAD_ENET_RX_ER__GPIO1_IO24  | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_ENET_CRS_DV__GPIO1_IO25 | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif	//<kuri0413> デッドマンSWを100K-PULL-DOWN
    MX6_PAD_ENET_RXD1__GPIO1_IO26   | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_ENET_RXD0__GPIO1_IO27   | MUX_PAD_CTRL(NO_PAD_CTRL),
//<kuri0523>    MX6_PAD_ENET_TX_EN__GPIO1_IO28  | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_ENET_TX_EN__GPIO1_IO28  | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),		//DEADMAN ERR (PU追加) <kuri0523>
//<kuri0523>    MX6_PAD_ENET_TXD1__GPIO1_IO29   | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_ENET_TXD1__GPIO1_IO29   | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),		//PMIC INT(PU追加) <kuri0523>
    MX6_PAD_ENET_TXD0__GPIO1_IO30   | MUX_PAD_CTRL(NO_PAD_CTRL),

    /* GPIO2 */
/*    MX6_PAD_SD4_DAT0__GPIO2_IO08    | MUX_PAD_CTRL(NO_PAD_CTRL),  */
//<kuri0426>    MX6_PAD_SD4_DAT3__GPIO2_IO11    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD4_DAT3__GPIO2_IO11    | MUX_PAD_CTRL(GPIO_PD_PAD_CTRL),	//<kuri0426>
    MX6_PAD_SD4_DAT4__GPIO2_IO12    | MUX_PAD_CTRL(NO_PAD_CTRL),
/* +Modified for iMX6Solo_PP at 2015/09/09 by Akita */
/*    MX6_PAD_SD4_DAT5__GPIO2_IO13    | MUX_PAD_CTRL(NO_PAD_CTRL), */
/* -Modified for iMX6Solo_PP at 2015/09/09 by Akita */
    MX6_PAD_SD4_DAT6__GPIO2_IO14    | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD4_DAT7__GPIO2_IO15    | MUX_PAD_CTRL(NO_PAD_CTRL),

    /* GPIO3 */
    MX6_PAD_EIM_D16__GPIO3_IO16     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D17__GPIO3_IO17     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D18__GPIO3_IO18     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D19__GPIO3_IO19     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D20__GPIO3_IO20     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D22__GPIO3_IO22     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D23__GPIO3_IO23     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D26__GPIO3_IO26     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D27__GPIO3_IO27     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D29__GPIO3_IO29     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D30__GPIO3_IO30     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D31__GPIO3_IO31     | MUX_PAD_CTRL(NO_PAD_CTRL),

    /* GPIO4 */
    MX6_PAD_GPIO_19__GPIO4_IO05     | MUX_PAD_CTRL(NO_PAD_CTRL),
    
#if	1	//<kuri0523>
    MX6_PAD_KEY_COL0__GPIO4_IO06    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR0(PU追加)
    MX6_PAD_KEY_COL1__GPIO4_IO08    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR1(PU追加)
    MX6_PAD_KEY_COL2__GPIO4_IO10    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR2(PU追加)
    MX6_PAD_KEY_COL3__GPIO4_IO12    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR3(PU追加)
    MX6_PAD_KEY_COL4__GPIO4_IO14    | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//メンブレンKR4(PU追加)
#endif	//<kuri0523>

    /* GPIO5 */
    MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT21__GPIO5_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT22__GPIO5_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT23__GPIO5_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_CSI0_PIXCLK__GPIO5_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_CSI0_MCLK__GPIO5_IO19   | MUX_PAD_CTRL(NO_PAD_CTRL),

    /* GPIO6 */
    MX6_PAD_NANDF_CS1__GPIO6_IO14   | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_NANDF_CS2__GPIO6_IO15   | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_NANDF_CS3__GPIO6_IO16   | MUX_PAD_CTRL(NO_PAD_CTRL),

    /* GPIO7 */
    MX6_PAD_GPIO_16__GPIO7_IO11     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_GPIO_17__GPIO7_IO12     | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_GPIO_18__GPIO7_IO13     | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const eimnor_pads[] = {
    MX6_PAD_EIM_A16__EIM_ADDR16         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A17__EIM_ADDR17         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A18__EIM_ADDR18         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A19__EIM_ADDR19         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A20__EIM_ADDR20         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A21__EIM_ADDR21         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A22__EIM_ADDR22         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A23__EIM_ADDR23         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A24__EIM_ADDR24         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_A25__EIM_ADDR25         | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_OE__EIM_OE_B            | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_RW__EIM_RW              | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_CS0__EIM_CS0_B          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA0__EIM_AD00           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA1__EIM_AD01           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA2__EIM_AD02           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA3__EIM_AD03           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA4__EIM_AD04           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA5__EIM_AD05           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA6__EIM_AD06           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA7__EIM_AD07           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA8__EIM_AD08           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA9__EIM_AD09           | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA10__EIM_AD10          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA11__EIM_AD11          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL) ,
    MX6_PAD_EIM_DA12__EIM_AD12          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA13__EIM_AD13          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA14__EIM_AD14          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_EIM_DA15__EIM_AD15          | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DATA_EN__EIM_DATA00    | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_VSYNC__EIM_DATA01      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT4__EIM_DATA02       | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT5__EIM_DATA03       | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT6__EIM_DATA04       | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT7__EIM_DATA05       | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT8__EIM_DATA06       | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT9__EIM_DATA07       | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT12__EIM_DATA08      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT13__EIM_DATA09      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT14__EIM_DATA10      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT15__EIM_DATA11      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT16__EIM_DATA12      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT17__EIM_DATA13      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT18__EIM_DATA14      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
    MX6_PAD_CSI0_DAT19__EIM_DATA15      | MUX_PAD_CTRL(WEIM_NOR_PAD_CTRL),
};

#if 1	//<kuri0317>
static iomux_v3_cfg_t const sdpower_pads[] = {
    MX6_PAD_SD4_DAT0__GPIO2_IO08    | MUX_PAD_CTRL(NO_PAD_CTRL),
};
#endif	//<kuri0317>


static void setup_iomux_eimnor(void)
{
    imx_iomux_v3_setup_multiple_pads(eimnor_pads, ARRAY_SIZE(eimnor_pads));
}

static void setup_eimnor_regs(void)
{
    struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

    writel(0x10010281, &weim_regs->cs0gcr1);
    writel(0x00000001, &weim_regs->cs0gcr2);
#if 0	//<kuri1017> for Micron
    writel(0x0b020000, &weim_regs->cs0rcr1);
    writel(0x00008000, &weim_regs->cs0rcr2);
#else   //<kuri1017>
 #if 0	//<kuri0418> マージン確保のため
    writel(0x0b030000, &weim_regs->cs0rcr1);
 #else	//<kuri0418> マージン確保のため
    writel(0x0c030000, &weim_regs->cs0rcr1);
 #endif	//<kuri0418> マージン確保のため
    writel(0x00009000, &weim_regs->cs0rcr2);
#endif	//<kuri1017>
    writel(0x0704a240, &weim_regs->cs0wcr1);
    writel(0x00000000, &weim_regs->cs0wcr2);
    writel(0x00000000, &weim_regs->wcr);

    set_chipselect_size(CS0_128);
}

static void setup_eimnor(void)
{
    setup_iomux_eimnor();
    setup_eimnor_regs();
}

#if BSP_BOARD_3RD //<kuri0413>
static void setup_iomux_gpio_fast(void)
{
	//最初にGPIO2_IO10とGPIO4_IO05のみをセット
    imx_iomux_v3_setup_multiple_pads(gpio_pads_fast, ARRAY_SIZE(gpio_pads_fast));
}
#endif	//BSP_BOARD_3RD <kuri0413>

#if BSP_BOARD_3RD //<kuri0413>
static void setup_iomux_gpio(void)
{
	if(Board_Version == 3)
	{
		//バージョン情報＝３のときは３次試作用の設定
	    imx_iomux_v3_setup_multiple_pads(gpio_pads_3rd, ARRAY_SIZE(gpio_pads_3rd));
	}
	else
	{
		//バージョン情報が３でないときは従来処理
		imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
	}
}
#else	//BSP_BOARD_3RD <kuri0413>
	//従来の処理
static void setup_iomux_gpio(void)
{
    imx_iomux_v3_setup_multiple_pads(gpio_pads, ARRAY_SIZE(gpio_pads));
}
#endif	//BSP_BOARD_3RD <kuri0413>


#if BSP_BOARD_3RD //<kuri0413>
static void setup_gpio_io_fast(void)
{
	unsigned char tmp1;
	unsigned char cnt=0;
	
	//まず、ボードのバージョン読み取りを実行する
	//GPIO2_10を入力にして、LOW:1次試作、High:2次試作以降
    gpio_direction_input(IMX_GPIO_NR(2, 10));	//GPIO2_10を入力セット
    while(1)
    {
    	tmp1 = (unsigned char)gpio_get_value(IMX_GPIO_NR(2, 10));		//一度読み取り
		udelay(10);														//10usecのディレイ
		if(tmp1 == (unsigned char)gpio_get_value(IMX_GPIO_NR(2, 10)))	//二度目の読み取りが一致したとき
		{
			if((tmp1 & 0x01) == 0)
			{
				//LOWのとき
				Board_Version = 1;		//1次試作以降
				break;					//判定終了
			}
			else
			{
				//LOW以外のとき
				Board_Version = 2;		//2次試作以降
				break;					//判定終了
			}
		}
		else
		{
			cnt++;	//リトライカウンタ＋１

			//100回実施しても一致しないとき＝強制的に2次試作
			if(cnt > 100)
			{
				Board_Version = 2;		//2次試作以降
				break;					//判定終了
			}
		}
	}
	#if 1	//<kuri0421>
	//ここでUSB_POWERを出力にしておくこと(OFF出力)
    if(Board_Version == 1)
    {
        //1次試作のとき(LOWでOFF)
        gpio_direction_output(IMX_GPIO_NR(2, 10), 0);
    }
    else
    {
        //2次試作以降のとき(HighでOFF)
        gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
    }
	
	#endif	//<kuri0421>
	
	gpio_direction_input(IMX_GPIO_NR(4, 5));	//GPIO4_IO05を入力セット(３次試作でなくても入力設定するためここでセット)
	//この時点で、Board_Version = 2なら、さらに３次試作かどうかの確認を行う
	
	if(Board_Version == 2)
	{
	    while(1)
	    {
	    	tmp1 = (unsigned char)gpio_get_value(IMX_GPIO_NR(4, 5));		//一度読み取り
			udelay(10);														//10usecのディレイ
			if(tmp1 == (unsigned char)gpio_get_value(IMX_GPIO_NR(4, 5)))	//二度目の読み取りが一致したとき
			{
				if((tmp1 & 0x01) == 0)
				{
					//LOWのとき
					Board_Version = 3;		//3次試作以降
					break;					//判定終了
				}
				else
				{
					//LOW以外のとき
					Board_Version = 2;		//2次試作以降
					break;					//判定終了
				}
			}
			else
			{
				cnt++;	//リトライカウンタ＋１
	
				//100回実施しても一致しないとき＝強制的に3次試作
				if(cnt > 100)
				{
					Board_Version = 3;		//2次試作以降
					break;					//判定終了
				}
			}
		}
	}
	//バージョン確定。バージョンを表示する。
	printf("New PP PCB version = %d , cnt = %d\n",Board_Version , cnt);
}
#endif	//BSP_BOARD_3RD <kuri0413>

static void setup_gpio_io(void)
{
#if BSP_BOARD_3RD	//<kuri0413>
	//BSP_BOARD_3RD定義時には変数不要
#else	//BSP_BOARD_3RD <kuri0413>
//#if 1	//<kuri1014>
	unsigned char tmp1;
	unsigned char cnt=0;
//#endif	//<kuri1014>
#endif	//BSP_BOARD_3RD <kuri0413>

#if BSP_BOARD_3RD	//<kuri0413>

	if(Board_Version == 3)
	{
		//GPIO1
														//GPIO1_IO00:KEY
														//GPIO1_IO01:SD Card Detect
		gpio_direction_input(IMX_GPIO_NR(1, 2));		//GPIO1_IO02:非常停止SW1
														//GPIO1_IO03:USB
														//GPIO1_IO04:KEY
														//GPIO1_IO05:KEY
    	gpio_direction_input(IMX_GPIO_NR(1, 6));		//GPIO1_IO06:非常停止SW2
														//GPIO1_IO07:TP_UART_TX
														//GPIO1_IO08:YP_UART_RX
														//GPIO1_IO09:LED
    	gpio_direction_input(IMX_GPIO_NR(1, 10));		//GPIO1_IO10:非常停止SW3
														//GPIO1_IO11:KEY
														//GPIO1_IO12:KEY
														//GPIO1_IO13:KEY
    	gpio_direction_input(IMX_GPIO_NR(1, 14));		//GPIO1_IO14:非常停止SW4
    	gpio_direction_input(IMX_GPIO_NR(1, 15));		//GPIO1_IO15:Remote-TR有無入力
														//GPIO1_IO16:SD1_DATA0
														//GPIO1_IO17:SD1_DATA1
														//GPIO1_IO18:SD1_CMD
														//GPIO1_IO19:SD1_DATA2
														//GPIO1_IO20:SD1_CLK
														//GPIO1_IO21:SD1_DATA3
														//GPIO1_IO22:ENET MDIO
    	gpio_direction_input(IMX_GPIO_NR(1, 23));		//GPIO1_IO23:23は空きなので入力にしておく（未使用）

    #if	0	//GPIO1_24～31は３次試作ではENET
    	gpio_direction_input(IMX_GPIO_NR(1, 24));		//GPIO1_IO24:ENET RX_ER
    	gpio_direction_input(IMX_GPIO_NR(1, 25));		//GPIO1_IO25:ENET CRS_DV
    	gpio_direction_input(IMX_GPIO_NR(1, 26));		//GPIO1_IO26:ENET RX_DATA1
    	gpio_direction_input(IMX_GPIO_NR(1, 27));		//GPIO1_IO27:ENET RX_DATA0
    	gpio_direction_input(IMX_GPIO_NR(1, 28));		//GPIO1_IO28:ENET TX_EN
    	gpio_direction_input(IMX_GPIO_NR(1, 29));		//GPIO1_IO29:ENET TX_DATA1
    	gpio_direction_input(IMX_GPIO_NR(1, 30));		//GPIO1_IO30:ENET TX_DATA0
														//GPIO1_IO31:ENET MDC
	#endif	//GPIO1_24～31は３次試作ではENET

		//GPIO2
														//GPIO2_IO00:NAND
														//GPIO2_IO01:NAND
														//GPIO2_IO02:NAND
														//GPIO2_IO03:NAND
														//GPIO2_IO04:NAND
														//GPIO2_IO05:NAND
														//GPIO2_IO06:NAND
														//GPIO2_IO07:NAND
    	gpio_direction_output(IMX_GPIO_NR(2, 8) , 0); 	//GPIO2_IO08:SD1 Power:最初はLOW
														//GPIO2_IO09:PWM3_OUT
														//GPIO2_IO10:USB Power
    	gpio_direction_output(IMX_GPIO_NR(2, 11), 0);	//GPIO2_IO11:LAN-PHY RESET
    	gpio_direction_output(IMX_GPIO_NR(2, 12), 1);	//GPIO2_IO12:Touch Pannel RESET
    	gpio_direction_output(IMX_GPIO_NR(2, 13), 1);	//GPIO2_IO13:LCD Power
    	gpio_direction_output(IMX_GPIO_NR(2, 14), 1);	//GPIO2_IO14:LCD RESET
    	gpio_direction_output(IMX_GPIO_NR(2, 15), 0);	//GPIO2_IO15:TP-DownLoad
														//GPIO2_IO16:EIM(NOR)
														//GPIO2_IO17:EIM(NOR)
														//GPIO2_IO18:EIM(NOR)
														//GPIO2_IO19:EIM(NOR)
														//GPIO2_IO20:EIM(NOR)
														//GPIO2_IO21:EIM(NOR)
														//GPIO2_IO22:EIM(NOR)
														//GPIO2_IO23:EIM(NOR)
														//GPIO2_IO24:EIM(NOR)
														//GPIO2_IO25:EIM(NOR)
														//GPIO2_IO26:EIM(NOR)
														//GPIO2_IO27:EIM(NOR)
														//GPIO2_IO28:EIM(NOR)
														//GPIO2_IO29:EIM(NOR)
														//GPIO2_IO30:EIM(NOR)
														//GPIO2_IO31:EIM(NOR)

		//GPIO3
														//GPIO3_IO00:EIM(NOR)
														//GPIO3_IO01:EIM(NOR)
														//GPIO3_IO02:EIM(NOR)
														//GPIO3_IO03:EIM(NOR)
														//GPIO3_IO04:EIM(NOR)
														//GPIO3_IO05:EIM(NOR)
														//GPIO3_IO06:EIM(NOR)
														//GPIO3_IO07:EIM(NOR)
														//GPIO3_IO08:EIM(NOR)
														//GPIO3_IO09:EIM(NOR)
														//GPIO3_IO10:EIM(NOR)
														//GPIO3_IO11:EIM(NOR)
														//GPIO3_IO12:EIM(NOR)
														//GPIO3_IO13:EIM(NOR)
														//GPIO3_IO14:EIM(NOR)
														//GPIO3_IO15:EIM(NOR)
    	gpio_direction_output(IMX_GPIO_NR(3, 16), 0);	//GPIO3_IO16:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 17), 0);	//GPIO3_IO17:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 18), 0);	//GPIO3_IO18:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 19), 0);	//GPIO3_IO19:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 20), 0);	//GPIO3_IO20:LED
														//GPIO3_IO21:PMIC I2C_SCL
    	gpio_direction_output(IMX_GPIO_NR(3, 22), 0);	//GPIO3_IO22:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);	//GPIO3_IO23:LED
														//GPIO3_IO24:RemoteTR UART3_TX
														//GPIO3_IO25:RemoteTR UART3_RX
    	gpio_direction_output(IMX_GPIO_NR(3, 26), 0);	//GPIO3_IO26:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 27), 0);	//GPIO3_IO27:LED
														//GPIO3_IO28:PMIC I2C_SDA
    	gpio_direction_output(IMX_GPIO_NR(3, 29), 0);	//GPIO3_IO29:LED(HOLD)
    	gpio_direction_output(IMX_GPIO_NR(3, 30), 0);	//GPIO3_IO30:LED(START)
    	gpio_direction_output(IMX_GPIO_NR(3, 31), 0);	//GPIO3_IO31:LED(SERVO ON)

		//GPIO4
														//GPIO4_IO00:なし
														//GPIO4_IO01:なし
														//GPIO4_IO02:なし
														//GPIO4_IO03:なし
														//GPIO4_IO04:なし
    #if	0	//GPIO4_IO5は既に設定済み
    	gpio_direction_input(IMX_GPIO_NR(4, 5));		//GPIO4_IO05:３次試作かどうかの区別ポート
    #endif	//GPIO4_IO5は既に設定済み
														//GPIO4_IO06:KEY
														//GPIO4_IO07:KEY
														//GPIO4_IO08:KEY
														//GPIO4_IO09:KEY
														//GPIO4_IO10:KEY
														//GPIO4_IO11:KEY
														//GPIO4_IO12:KEY
														//GPIO4_IO13:KEY
														//GPIO4_IO14:KEY
														//GPIO4_IO15:KEY
														//GPIO4_IO16:LCD
														//GPIO4_IO17:LCD
														//GPIO4_IO18:LCD
														//GPIO4_IO19:LCD
														//GPIO4_IO20:SD1 WP
														//GPIO4_IO21:LCD
														//GPIO4_IO22:LCD
														//GPIO4_IO23:LCD
														//GPIO4_IO24:LCD
														//GPIO4_IO25:LCD
														//GPIO4_IO26:LCD
														//GPIO4_IO27:LCD
														//GPIO4_IO28:LCD
														//GPIO4_IO29:LCD
														//GPIO4_IO30:LCD
														//GPIO4_IO31:LCD

		//GPIO5
														//GPIO5_IO00:EIM(NOR)未使用
														//GPIO5_IO01:なし
														//GPIO5_IO02:EIM(NOR)
														//GPIO5_IO03:なし
														//GPIO5_IO04:EIM(NOR)
														//GPIO5_IO05:LCD
														//GPIO5_IO06:LCD
														//GPIO5_IO07:LCD
														//GPIO5_IO08:LCD
														//GPIO5_IO09:LCD
														//GPIO5_IO10:LCD
														//GPIO5_IO11:LCD
														//GPIO5_IO12:EIM(NOR)未使用
														//GPIO5_IO13:EIM(NOR)未使用
    	gpio_direction_input(IMX_GPIO_NR(5, 14));		//GPIO5_IO14:START SW
    	gpio_direction_input(IMX_GPIO_NR(5, 15));		//GPIO5_IO15:HOLD SW
    	gpio_direction_input(IMX_GPIO_NR(5, 16));		//GPIO5_IO16:MODE SW
    	gpio_direction_input(IMX_GPIO_NR(5, 17));		//GPIO5_IO17:MODE SW
    	gpio_direction_input(IMX_GPIO_NR(5, 18));		//GPIO5_IO18:未使用
    	gpio_direction_input(IMX_GPIO_NR(5, 19));		//GPIO5_IO19:未使用
														//GPIO5_IO20:EIM(NOR)
														//GPIO5_IO21:EIM(NOR)
														//GPIO5_IO22:EIM(NOR)
														//GPIO5_IO23:EIM(NOR)
														//GPIO5_IO24:EIM(NOR)
														//GPIO5_IO25:EIM(NOR)
														//GPIO5_IO26:EIM(NOR)
														//GPIO5_IO27:EIM(NOR)
														//GPIO5_IO28:UART1 TX
														//GPIO5_IO29:UART1 RX
														//GPIO5_IO30:EIM(NOR)
														//GPIO5_IO31:EIM(NOR)

		//GPIO6
														//GPIO6_IOO0:EIM(NOR)
														//GPIO6_IOO1:EIM(NOR)
														//GPIO6_IOO2:EIM(NOR)
														//GPIO6_IOO3:EIM(NOR)
														//GPIO6_IOO4:EIM(NOR)
														//GPIO6_IOO5:EIM(NOR)
														//GPIO6_IOO6:EIM(NOR)
														//GPIO6_IOO7:NAND
														//GPIO6_IOO8:NAND
														//GPIO6_IOO9:NAND
														//GPIO6_IO1O:NAND
														//GPIO6_IO11:NAND
														//GPIO6_IO12:なし
														//GPIO6_IO13:なし
    	gpio_direction_input(IMX_GPIO_NR(6, 14));		//GPIO6_IO14:デッドマンSW-2
    	gpio_direction_input(IMX_GPIO_NR(6, 15));		//GPIO6_IO15:未使用
    	gpio_direction_input(IMX_GPIO_NR(6, 16));		//GPIO6_IO16:未使用

	#if 1	//GPIO6_IO17～IO19、GPIO7_IO00が3次試作ではIOポート
    	gpio_direction_input(IMX_GPIO_NR(6, 17));		//GPIO6_IO17:未使用
    	gpio_direction_input(IMX_GPIO_NR(6, 18));		//GPIO6_IO18:PMIC割り込み
    	gpio_direction_input(IMX_GPIO_NR(6, 19));		//GPIO6_IO19:未使用
														//GPIO6_IO20:未使用
														//GPIO6_IO21:未使用
														//GPIO6_IO22:未使用
														//GPIO6_IO23:未使用
														//GPIO6_IO24:未使用
														//GPIO6_IO25:未使用
														//GPIO6_IO26:未使用
														//GPIO6_IO27:ENET_REF_CLOCK
														//GPIO6_IO28:未使用
														//GPIO6_IO29:未使用
														//GPIO6_IO30:未使用
														//GPIO6_IO31:未使用

		//GPIO7
    	gpio_direction_input(IMX_GPIO_NR(7, 0));		//GPIO7_IO00:LAN PHY INT
	#endif	//
														//GPIO7_IO01:未使用
														//GPIO7_IO02:未使用
														//GPIO7_IO03:未使用
														//GPIO7_IO04:未使用
														//GPIO7_IO05:未使用
														//GPIO7_IO06:未使用
														//GPIO7_IO07:未使用
														//GPIO7_IO08:未使用
														//GPIO7_IO09:NAND
														//GPIO7_IO10:NAND
    	gpio_direction_input(IMX_GPIO_NR(7, 11));		//GPIO7_IO11:未使用
    	gpio_direction_input(IMX_GPIO_NR(7, 12));		//GPIO7_IO12:未使用
    	gpio_direction_input(IMX_GPIO_NR(7, 13));		//GPIO7_IO13:未使用
	}
	else
	{
		//従来の処理
		//GPIO1
														//GPIO1_IO00:KEY
														//GPIO1_IO01:SD Card Detect
		gpio_direction_input(IMX_GPIO_NR(1, 2));		//GPIO1_IO02:非常停止SW1
														//GPIO1_IO03:USB
														//GPIO1_IO04:KEY
														//GPIO1_IO05:KEY
    	gpio_direction_input(IMX_GPIO_NR(1, 6));		//GPIO1_IO06:非常停止SW2
														//GPIO1_IO07:TP_UART_TX
														//GPIO1_IO08:YP_UART_RX
														//GPIO1_IO09:LED
    	gpio_direction_input(IMX_GPIO_NR(1, 10));		//GPIO1_IO10:非常停止SW3
														//GPIO1_IO11:KEY
														//GPIO1_IO12:KEY
														//GPIO1_IO13:KEY
    	gpio_direction_input(IMX_GPIO_NR(1, 14));		//GPIO1_IO14:非常停止SW4
    	gpio_direction_input(IMX_GPIO_NR(1, 15));		//GPIO1_IO15:Remote-TR有無入力
														//GPIO1_IO16:SD1_DATA0
														//GPIO1_IO17:SD1_DATA1
														//GPIO1_IO18:SD1_CMD
														//GPIO1_IO19:SD1_DATA2
														//GPIO1_IO20:SD1_CLK
														//GPIO1_IO21:SD1_DATA3
														//GPIO1_IO22:ENET MDIO
														//GPIO1_IO23:ENET_REF_CLK
    	gpio_direction_input(IMX_GPIO_NR(1, 24));		//GPIO1_IO24:デッドマンSW1
    	gpio_direction_input(IMX_GPIO_NR(1, 25));		//GPIO1_IO25:デッドマンSW2
    	gpio_direction_input(IMX_GPIO_NR(1, 26));		//GPIO1_IO26:未使用
    	gpio_direction_input(IMX_GPIO_NR(1, 27));		//GPIO1_IO27:未使用
    	gpio_direction_input(IMX_GPIO_NR(1, 28));		//GPIO1_IO28:デッドマンERROR
    	gpio_direction_input(IMX_GPIO_NR(1, 29));		//GPIO1_IO29:PMIC INT
    	gpio_direction_input(IMX_GPIO_NR(1, 30));		//GPIO1_IO30:LAN PHY INT
														//GPIO1_IO31:ENET MDC

		//GPIO2
														//GPIO2_IO00:NAND
														//GPIO2_IO01:NAND
														//GPIO2_IO02:NAND
														//GPIO2_IO03:NAND
														//GPIO2_IO04:NAND
														//GPIO2_IO05:NAND
														//GPIO2_IO06:NAND
														//GPIO2_IO07:NAND
    	gpio_direction_output(IMX_GPIO_NR(2, 8) , 0); 	//GPIO2_IO08:SD1 Power:最初はLOW
														//GPIO2_IO09:PWM3_OUT
														//GPIO2_IO10:USB Power
    	gpio_direction_output(IMX_GPIO_NR(2, 11), 0);	//GPIO2_IO11:LAN-PHY RESET
    	gpio_direction_output(IMX_GPIO_NR(2, 12), 1);	//GPIO2_IO12:Touch Pannel RESET
    	gpio_direction_output(IMX_GPIO_NR(2, 13), 1);	//GPIO2_IO13:LCD Power
    	gpio_direction_output(IMX_GPIO_NR(2, 14), 1);	//GPIO2_IO14:LCD RESET
    	gpio_direction_output(IMX_GPIO_NR(2, 15), 0);	//GPIO2_IO15:TP-DownLoad
														//GPIO2_IO16:EIM(NOR)
														//GPIO2_IO17:EIM(NOR)
														//GPIO2_IO18:EIM(NOR)
														//GPIO2_IO19:EIM(NOR)
														//GPIO2_IO20:EIM(NOR)
														//GPIO2_IO21:EIM(NOR)
														//GPIO2_IO22:EIM(NOR)
														//GPIO2_IO23:EIM(NOR)
														//GPIO2_IO24:EIM(NOR)
														//GPIO2_IO25:EIM(NOR)
														//GPIO2_IO26:EIM(NOR)
														//GPIO2_IO27:EIM(NOR)
														//GPIO2_IO28:EIM(NOR)
														//GPIO2_IO29:EIM(NOR)
														//GPIO2_IO30:EIM(NOR)
														//GPIO2_IO31:EIM(NOR)

		//GPIO3
														//GPIO3_IO00:EIM(NOR)
														//GPIO3_IO01:EIM(NOR)
														//GPIO3_IO02:EIM(NOR)
														//GPIO3_IO03:EIM(NOR)
														//GPIO3_IO04:EIM(NOR)
														//GPIO3_IO05:EIM(NOR)
														//GPIO3_IO06:EIM(NOR)
														//GPIO3_IO07:EIM(NOR)
														//GPIO3_IO08:EIM(NOR)
														//GPIO3_IO09:EIM(NOR)
														//GPIO3_IO10:EIM(NOR)
														//GPIO3_IO11:EIM(NOR)
														//GPIO3_IO12:EIM(NOR)
														//GPIO3_IO13:EIM(NOR)
														//GPIO3_IO14:EIM(NOR)
														//GPIO3_IO15:EIM(NOR)
    	gpio_direction_output(IMX_GPIO_NR(3, 16), 0);	//GPIO3_IO16:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 17), 0);	//GPIO3_IO17:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 18), 0);	//GPIO3_IO18:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 19), 0);	//GPIO3_IO19:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 20), 0);	//GPIO3_IO20:LED
														//GPIO3_IO21:PMIC I2C_SCL
    	gpio_direction_output(IMX_GPIO_NR(3, 22), 0);	//GPIO3_IO22:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 23), 0);	//GPIO3_IO23:LED
														//GPIO3_IO24:RemoteTR UART3_TX
														//GPIO3_IO25:RemoteTR UART3_RX
    	gpio_direction_output(IMX_GPIO_NR(3, 26), 0);	//GPIO3_IO26:LED
    	gpio_direction_output(IMX_GPIO_NR(3, 27), 0);	//GPIO3_IO27:LED
														//GPIO3_IO28:PMIC I2C_SDA
    	gpio_direction_output(IMX_GPIO_NR(3, 29), 0);	//GPIO3_IO29:LED(HOLD)
    	gpio_direction_output(IMX_GPIO_NR(3, 30), 0);	//GPIO3_IO30:LED(START)
    	gpio_direction_output(IMX_GPIO_NR(3, 31), 0);	//GPIO3_IO31:LED(SERVO ON)

		//GPIO4
														//GPIO4_IO00:なし
														//GPIO4_IO01:なし
														//GPIO4_IO02:なし
														//GPIO4_IO03:なし
														//GPIO4_IO04:なし
    #if	0	//GPIO4_IO5は既に設定済み
    	gpio_direction_input(IMX_GPIO_NR(4, 5));		//GPIO4_IO05:３次試作かどうかの区別ポート
    #endif	//GPIO4_IO5は既に設定済み
														//GPIO4_IO06:KEY
														//GPIO4_IO07:KEY
														//GPIO4_IO08:KEY
														//GPIO4_IO09:KEY
														//GPIO4_IO10:KEY
														//GPIO4_IO11:KEY
														//GPIO4_IO12:KEY
														//GPIO4_IO13:KEY
														//GPIO4_IO14:KEY
														//GPIO4_IO15:KEY
														//GPIO4_IO16:LCD
														//GPIO4_IO17:LCD
														//GPIO4_IO18:LCD
														//GPIO4_IO19:LCD
														//GPIO4_IO20:SD1 WP
														//GPIO4_IO21:LCD
														//GPIO4_IO22:LCD
														//GPIO4_IO23:LCD
														//GPIO4_IO24:LCD
														//GPIO4_IO25:LCD
														//GPIO4_IO26:LCD
														//GPIO4_IO27:LCD
														//GPIO4_IO28:LCD
														//GPIO4_IO29:LCD
														//GPIO4_IO30:LCD
														//GPIO4_IO31:LCD

		//GPIO5
														//GPIO5_IO00:EIM(NOR)未使用
														//GPIO5_IO01:なし
														//GPIO5_IO02:EIM(NOR)
														//GPIO5_IO03:なし
														//GPIO5_IO04:EIM(NOR)
														//GPIO5_IO05:LCD
														//GPIO5_IO06:LCD
														//GPIO5_IO07:LCD
														//GPIO5_IO08:LCD
														//GPIO5_IO09:LCD
														//GPIO5_IO10:LCD
														//GPIO5_IO11:LCD
														//GPIO5_IO12:EIM(NOR)未使用
														//GPIO5_IO13:EIM(NOR)未使用
    	gpio_direction_input(IMX_GPIO_NR(5, 14));		//GPIO5_IO14:START SW
    	gpio_direction_input(IMX_GPIO_NR(5, 15));		//GPIO5_IO15:HOLD SW
    	gpio_direction_input(IMX_GPIO_NR(5, 16));		//GPIO5_IO16:MODE SW
    	gpio_direction_input(IMX_GPIO_NR(5, 17));		//GPIO5_IO17:MODE SW
    	gpio_direction_input(IMX_GPIO_NR(5, 18));		//GPIO5_IO18:未使用
    	gpio_direction_input(IMX_GPIO_NR(5, 19));		//GPIO5_IO19:未使用
														//GPIO5_IO20:EIM(NOR)
														//GPIO5_IO21:EIM(NOR)
														//GPIO5_IO22:EIM(NOR)
														//GPIO5_IO23:EIM(NOR)
														//GPIO5_IO24:EIM(NOR)
														//GPIO5_IO25:EIM(NOR)
														//GPIO5_IO26:EIM(NOR)
														//GPIO5_IO27:EIM(NOR)
														//GPIO5_IO28:UART1 TX
														//GPIO5_IO29:UART1 RX
														//GPIO5_IO30:EIM(NOR)
														//GPIO5_IO31:EIM(NOR)

		//GPIO6
														//GPIO6_IOO0:EIM(NOR)
														//GPIO6_IOO1:EIM(NOR)
														//GPIO6_IOO2:EIM(NOR)
														//GPIO6_IOO3:EIM(NOR)
														//GPIO6_IOO4:EIM(NOR)
														//GPIO6_IOO5:EIM(NOR)
														//GPIO6_IOO6:EIM(NOR)
														//GPIO6_IOO7:NAND
														//GPIO6_IOO8:NAND
														//GPIO6_IOO9:NAND
														//GPIO6_IO1O:NAND
														//GPIO6_IO11:NAND
														//GPIO6_IO12:なし
														//GPIO6_IO13:なし
    	gpio_direction_input(IMX_GPIO_NR(6, 14));		//GPIO6_IO14:未使用
    	gpio_direction_input(IMX_GPIO_NR(6, 15));		//GPIO6_IO15:未使用
    	gpio_direction_input(IMX_GPIO_NR(6, 16));		//GPIO6_IO16:未使用
    													//GPIO6_IO17:未使用
    													//GPIO6_IO18:未使用
    													//GPIO6_IO19:ENET RGMII TXC
														//GPIO6_IO20:ENET RGMII TD0
														//GPIO6_IO21:ENET RGMII TD1
														//GPIO6_IO22:ENET RGMII TD2
														//GPIO6_IO23:ENET RGMII TD3
														//GPIO6_IO24:ENET RGMII RX CTL
														//GPIO6_IO25:ENET RGMII RD0
														//GPIO6_IO26:ENET RGMII TX CTL
														//GPIO6_IO27:ENET RGMII RD1
														//GPIO6_IO28:ENET RGMII RD2
														//GPIO6_IO29:ENET RGMII RD3
														//GPIO6_IO30:ENET RGMII RXC
														//GPIO6_IO31:未使用

		//GPIO7
    													//GPIO7_IO00:未使用
														//GPIO7_IO01:未使用
														//GPIO7_IO02:未使用
														//GPIO7_IO03:未使用
														//GPIO7_IO04:未使用
														//GPIO7_IO05:未使用
														//GPIO7_IO06:未使用
														//GPIO7_IO07:未使用
														//GPIO7_IO08:未使用
														//GPIO7_IO09:NAND
														//GPIO7_IO10:NAND
    	gpio_direction_input(IMX_GPIO_NR(7, 11));		//GPIO7_IO11:未使用
    	gpio_direction_input(IMX_GPIO_NR(7, 12));		//GPIO7_IO12:未使用
    	gpio_direction_input(IMX_GPIO_NR(7, 13));		//GPIO7_IO13:未使用
	
	}
	
#else	//BSP_BOARD_3RD <kuri0413>
    gpio_direction_input(IMX_GPIO_NR(1, 2));
    gpio_direction_input(IMX_GPIO_NR(1, 6));
    gpio_direction_input(IMX_GPIO_NR(1, 10));
    gpio_direction_input(IMX_GPIO_NR(1, 14));
    gpio_direction_input(IMX_GPIO_NR(1, 15));
    gpio_direction_input(IMX_GPIO_NR(1, 24));
    gpio_direction_input(IMX_GPIO_NR(1, 25));
    gpio_direction_input(IMX_GPIO_NR(1, 26));
    gpio_direction_input(IMX_GPIO_NR(1, 27));
    gpio_direction_input(IMX_GPIO_NR(1, 28));
    gpio_direction_input(IMX_GPIO_NR(1, 29));
    gpio_direction_input(IMX_GPIO_NR(1, 30));

/*    gpio_direction_output(IMX_GPIO_NR(2, 8) , 1); */  /* SD1 Power */
    gpio_direction_output(IMX_GPIO_NR(2, 11), 0);
    gpio_direction_output(IMX_GPIO_NR(2, 12), 1);
    gpio_direction_output(IMX_GPIO_NR(2, 13), 1);
    gpio_direction_output(IMX_GPIO_NR(2, 14), 1);
    gpio_direction_output(IMX_GPIO_NR(2, 15), 0);

    gpio_direction_output(IMX_GPIO_NR(3, 16), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 17), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 18), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 19), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 20), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 22), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 23), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 26), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 27), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 29), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 30), 0);
    gpio_direction_output(IMX_GPIO_NR(3, 31), 0);

    gpio_direction_input(IMX_GPIO_NR(4, 5));

    gpio_direction_input(IMX_GPIO_NR(5, 14));
    gpio_direction_input(IMX_GPIO_NR(5, 15));
    gpio_direction_input(IMX_GPIO_NR(5, 16));
    gpio_direction_input(IMX_GPIO_NR(5, 17));
    gpio_direction_input(IMX_GPIO_NR(5, 18));
    gpio_direction_input(IMX_GPIO_NR(5, 19));

    gpio_direction_input(IMX_GPIO_NR(6, 14));
    gpio_direction_input(IMX_GPIO_NR(6, 15));
    gpio_direction_input(IMX_GPIO_NR(6, 16));

    gpio_direction_input(IMX_GPIO_NR(7, 11));
    gpio_direction_input(IMX_GPIO_NR(7, 12));
    gpio_direction_input(IMX_GPIO_NR(7, 13));

#if 1	//<kuri1014>
	//ボードのバージョンを読み取る。
	//GPIO2_10を入力にして、LOW:1次試作、High:2次試作以降
    gpio_direction_input(IMX_GPIO_NR(2, 10));	//GPIO2_10を入力セット
    while(1)
    {
    	tmp1 = (unsigned char)gpio_get_value(IMX_GPIO_NR(2, 10));		//一度読み取り
		udelay(10);														//10usecのディレイ
		if(tmp1 == (unsigned char)gpio_get_value(IMX_GPIO_NR(2, 10)))	//二度目の読み取りが一致したとき
		{
			if((tmp1 & 0x01) == 0)
			{
				//LOWのとき
				Board_Version = 1;		//1次試作以降
				break;					//判定終了
			}
			else
			{
				//LOW以外のとき
				Board_Version = 2;		//2次試作以降
				break;					//判定終了
			}
		}
		else
		{
			cnt++;	//リトライカウンタ＋１

			//100回実施しても一致しないとき＝強制的に2次試作
			if(cnt > 100)
			{
				Board_Version = 2;		//2次試作以降
				break;					//判定終了
			}
		}
	}
	printf("New PP PCB version = %d , cnt = %d\n",Board_Version , cnt);
#endif	//<kuri1014>
#endif	//BSP_BOARD_3RD <kuri0413>

}

static void setup_gpio(void)
{
#if BSP_BOARD_3RD	//<kuri0413>
	//最初にボード区別に必要なポートのみ設定してボード区別を実行
    setup_iomux_gpio_fast();	//GPIO2_IO10とGPIO4_IO05のみ設定
    setup_gpio_io_fast();		//上記ポートのリードとボードバージョン確定
#endif	//BSP_BOARD_3RD <kuri0413>
    setup_iomux_gpio();
    setup_gpio_io();
    
#if 1	//<kuri0317>
	//ここまで実測で約120msecなので、80msec待つ
	//(ドライバと同じくトータル200msec待たせる)
	udelay(80*1000);
	//SD-Power ON
	gpio_direction_output(IMX_GPIO_NR(2, 8) , 1);
	//ドライバと同じくON後に10msec待つ
	udelay(10*1000);
#endif	//<kuri0317>

#if 1	//<tanaka20200114>
	print_pmu_reg();
#endif
}

static void ccm_init(void)
{
    struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

    /* Enable clock for EIM NOR Flash. */
    writel(0x00000FFC, &ccm->CCGR6);
}

/* +Modified for iMX6Solo_PP at 2015/09/10 by Akita */
/* PWM1 device for backlight.  */
#define PWM1_ID                 0

#define PWM_SOURCE_CLK          32000
#define PWM_CLK                 200
#define BKL_PWM_PERIOD ((PWM_SOURCE_CLK / PWM_CLK) - 2)
#define BKL_PWM_SAMPLE          61  // 0xE0

#define IPUV3_REG_BASE_ADDR                             0x02400000

#define IPUV3_IPU_CONF_OFFSET                           0x000
#define IPUV3_IPU_DISP_GEN_OFFSET                       0x0C4


#define IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_OFFSET     24
#define IPU_IPU_CONF_DP_EN_OFFSET                       5
#define IPU_IPU_CONF_DI0_EN_OFFSET                      6
#define IPU_IPU_CONF_DC_EN_OFFSET                       9
#define IPU_IPU_CONF_DMFC_EN_OFFSET                     10

#define IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_MASK       (1 << IPU_IPU_DISP_GEN_DI0_COUNTER_RELEASE_OFFSET)
#define IPU_IPU_CONF_DP_EN_MASK                         (1 << IPU_IPU_CONF_DP_EN_OFFSET)
#define IPU_IPU_CONF_DI0_EN_MASK                        (1 << IPU_IPU_CONF_DI0_EN_OFFSET)
#define IPU_IPU_CONF_DC_EN_MASK                         (1 << IPU_IPU_CONF_DC_EN_OFFSET)
#define IPU_IPU_CONF_DMFC_EN_MASK                       (1 << IPU_IPU_CONF_DMFC_EN_OFFSET)


static iomux_v3_cfg_t const pwm1_pads[] = {
    MX6_PAD_GPIO_9__PWM1_OUT      | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_display(void);

static void LCD_backlight_init(void)
{
    /* Initialize PWM1 for LCD backlight.  */
    imx_iomux_v3_setup_multiple_pads(pwm1_pads, ARRAY_SIZE(pwm1_pads));

    pwm_init(PWM1_ID, 0, 0);
    pwm_config_direct(PWM1_ID, BKL_PWM_SAMPLE, BKL_PWM_PERIOD);

    /* Backlight turn on.  */
    pwm_enable(PWM1_ID);
}

static void enable_lcd(void)
{
//    printf("SABRE SD: +enable_lcd\n");

    gpio_direction_output(DISP0_LCD_RESET, 0);
    gpio_direction_output(DISP0_LCD_POWER, 1);
    udelay(5);
    gpio_direction_output(DISP0_LCD_RESET, 1);

    LCD_backlight_init();
//    printf("SABRE SD: -enable_lcd\n");
}
/* -Modified for iMX6Solo_PP at 2015/09/10 by Akita */
#endif /* IMX6SOLO_PP */
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */

#if BSP_BOARD_3RD	//<kuri0413>
static void setup_iomux_enet(void)
{
    if(Board_Version == 3)
    {
		//３次試作用のENET設定
	    imx_iomux_v3_setup_multiple_pads(enet_pads_3rd, ARRAY_SIZE(enet_pads_3rd));
	}
	else
	{
		//３次試作以前のENET設定
	    imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));
	}
	/* Reset KSZ9031/DP83620 PHY */
	//従来はSABRE_SDのままGPIO1_IO25に出力していたのをGPIO2_IO11出力に修正
    gpio_direction_output(IMX_GPIO_NR(2, 11) , 0);
    udelay(10*1000);	//10msec（ハードウェア仕様により）
    gpio_set_value(IMX_GPIO_NR(2, 11), 1);

}
#else	//BSP_BOARD_3RD <kuri0413>
	//従来処理
static void setup_iomux_enet(void)
{
    imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

    /* Reset AR8031 PHY */
    gpio_direction_output(IMX_GPIO_NR(1, 25) , 0);
    udelay(500);
    gpio_set_value(IMX_GPIO_NR(1, 25), 1);
}
#endif	//BSP_BOARD_3RD <kuri0413>


static iomux_v3_cfg_t const usdhc1_pads[] = {
    MX6_PAD_SD1_CLK__SD1_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_CMD__SD1_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DAT0__SD1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DAT1__SD1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DAT2__SD1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD1_DAT3__SD1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
#if	0	//<kuri0523>
    MX6_PAD_GPIO_1__GPIO1_IO01  | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#else	//<kuri0523>
    MX6_PAD_GPIO_1__GPIO1_IO01  | MUX_PAD_CTRL(USDHC_100PU_PAD_CTRL), /* CD(100K-PUに変更) */
#endif	//<kuri0523>


};

static iomux_v3_cfg_t const usdhc2_pads[] = {
#if 0	//<kuri0413> SD2は設定しない
    MX6_PAD_SD2_CLK__SD2_CLK    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD2_CMD__SD2_CMD    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD2_DAT0__SD2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD2_DAT1__SD2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD2_DAT2__SD2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD2_DAT3__SD2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NANDF_D4__SD2_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NANDF_D5__SD2_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NANDF_D6__SD2_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NANDF_D7__SD2_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NANDF_D2__GPIO2_IO02    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#endif	//<kuri0413> SD2は設定しない
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
#if 0	//<kuri0413>
    MX6_PAD_SD3_CLK__SD3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_CMD__SD3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_NANDF_D0__GPIO2_IO00    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
#endif	//<kuri0413>
};

static iomux_v3_cfg_t const usdhc4_pads[] = {
#if 0	//<kuri0413>
    MX6_PAD_SD4_CLK__SD4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_CMD__SD4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
    MX6_PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
#endif	//<kuri0413>
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
    MX6_PAD_KEY_COL0__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_KEY_COL1__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_KEY_ROW0__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
    MX6_PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const rgb_pads[] = {
    MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03 | MUX_PAD_CTRL(NO_PAD_CTRL),
/* Modified for iMX6Solo_PP at 2015/09/09 by Akita
    MX6_PAD_DI0_PIN4__IPU1_DI0_PIN04 | MUX_PAD_CTRL(NO_PAD_CTRL),
*/
    MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17 | MUX_PAD_CTRL(NO_PAD_CTRL),
/* Modified for iMX6Solo_PP at 2015/09/09 by Akita
    MX6_PAD_DISP0_DAT18__IPU1_DISP0_DATA18 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT19__IPU1_DISP0_DATA19 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT20__IPU1_DISP0_DATA20 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT21__IPU1_DISP0_DATA21 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT22__IPU1_DISP0_DATA22 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_DISP0_DAT23__IPU1_DISP0_DATA23 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_SD1_DAT3__GPIO1_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
*/
/* +Modified for iMX6Solo_PP at 2015/09/09 by Akita */
    /* LCD Power */
    MX6_PAD_SD4_DAT5__GPIO2_IO13    | MUX_PAD_CTRL(NO_PAD_CTRL),
    /* LCD Reset */
    MX6_PAD_SD4_DAT6__GPIO2_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
/* -Modified for iMX6Solo_PP at 2015/09/09 by Akita */
};

static void enable_rgb(struct display_info_t const *dev)
{
    imx_iomux_v3_setup_multiple_pads(rgb_pads, ARRAY_SIZE(rgb_pads));

/* +Modified for iMX6Solo_PP at 2015/09/09 by Akita */
#ifndef IMX6SOLO_PP
    gpio_direction_output(DISP0_PWR_EN, 1);
#endif /* IMX6SOLO_PP */
/* -Modified for iMX6Solo_PP at 2015/09/09 by Akita */

}

static struct i2c_pads_info i2c_pad_info0 = {
    .scl = {
        .i2c_mode = MX6_PAD_EIM_D21__I2C1_SCL | I2C_PAD,
        .gpio_mode = MX6_PAD_EIM_D21__GPIO3_IO21 | I2C_PAD,
        .gp = IMX_GPIO_NR(3, 21)
    },
    .sda = {
        .i2c_mode = MX6_PAD_EIM_D28__I2C1_SDA | I2C_PAD,
        .gpio_mode = MX6_PAD_EIM_D28__GPIO3_IO28 | I2C_PAD,
        .gp = IMX_GPIO_NR(3, 28)
    }
};

static void setup_spi(void)
{
    imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
}

#if 0	//<kuri0423> 未使用のため削除
iomux_v3_cfg_t const pcie_pads[] = {
    MX6_PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),    /* POWER */
    MX6_PAD_GPIO_17__GPIO7_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),    /* RESET */
};

static void setup_pcie(void)
{
    imx_iomux_v3_setup_multiple_pads(pcie_pads, ARRAY_SIZE(pcie_pads));
}
#endif	//<kuri0423> 未使用のため削除

iomux_v3_cfg_t const di0_pads[] = {
    MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK,    /* DISP0_CLK */
    MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02,       /* DISP0_HSYNC */
    MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03,       /* DISP0_VSYNC */
};

static void setup_iomux_uart(void)
{
    imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[4] = {
#if 0	//<kuri0423>
    {USDHC2_BASE_ADDR},
    {USDHC3_BASE_ADDR},
    {USDHC4_BASE_ADDR},
#endif	//<kuri0423>
    {USDHC1_BASE_ADDR},
};

#define USDHC1_CD_GPIO  IMX_GPIO_NR(1, 1)
#if 0	//<kuri0413> USDHCを1つだけにする
#define USDHC2_CD_GPIO  IMX_GPIO_NR(2, 2)
#define USDHC3_CD_GPIO  IMX_GPIO_NR(2, 0)
#endif	//<kuri0413> USDHCを1つだけにする

int board_mmc_getcd(struct mmc *mmc)
{
    struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
    int ret = 0;

    switch (cfg->esdhc_base) {
    case USDHC1_BASE_ADDR:
        ret = !gpio_get_value(USDHC1_CD_GPIO);
        break;
#if 1	//<kuri0413> USDHCを1つだけにする
	default:
		ret = 0;	//SD1以外はカード無し
		break;
#else	//<kuri0413> USDHCを1つだけにする
    case USDHC2_BASE_ADDR:
        ret = !gpio_get_value(USDHC2_CD_GPIO);
        break;
    case USDHC3_BASE_ADDR:
        ret = !gpio_get_value(USDHC3_CD_GPIO);
        break;
    case USDHC4_BASE_ADDR:
        ret = 1; /* eMMC/uSDHC4 is always present */
        break;
#endif	//<kuri0413> USDHCを1つだけにする
    }

    return ret;
}

int board_mmc_init(bd_t *bis)
{
#ifndef CONFIG_SPL_BUILD
    int ret;
    int i;

    printf("board_mmc_init in.\n");		//<kuri0413>

    /*
     * According to the board_mmc_init() the following map is done:
     * (U-boot device node)    (Physical Port)
     * mmc0                    SD2
     * mmc1                    SD3
     * mmc2                    eMMC
     * mmc3                    SD1
     */
     
#if 1	//<kuri0423>
	//SD1スロット(mmc ch.0に再セットしている)のみ初期化して使用する
    for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
        switch (i) {
        case 0:
            imx_iomux_v3_setup_multiple_pads(
                usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
            gpio_direction_input(USDHC1_CD_GPIO);
            usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
            usdhc_cfg[0].max_bus_width = 4;
            break;
        default:
            printf("Warning: you configured more USDHC controllers"
                   "(%d) then supported by the board (%d)\n",
                   i + 1, CONFIG_SYS_FSL_USDHC_NUM);
            return -EINVAL;
        }

        ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
        if (ret)
            return ret;
    }

#else	//<kuri0423>
    for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
        switch (i) {
        case 0:
            imx_iomux_v3_setup_multiple_pads(
                usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
                gpio_direction_input(USDHC2_CD_GPIO);
            //<kuri0413>	gpio_direction_input(USDHC2_CD_GPIO);
            usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
            break;
        case 1:
            imx_iomux_v3_setup_multiple_pads(
                usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
                gpio_direction_input(USDHC3_CD_GPIO);
            //<kuri0413>	gpio_direction_input(USDHC3_CD_GPIO);
            usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
            break;
        case 2:
            imx_iomux_v3_setup_multiple_pads(
                usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
            usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
            break;
        case 3:
            imx_iomux_v3_setup_multiple_pads(
                usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
            gpio_direction_input(USDHC1_CD_GPIO);
            usdhc_cfg[3].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
            usdhc_cfg[3].max_bus_width = 4;
            break;
        default:
            printf("Warning: you configured more USDHC controllers"
                   "(%d) then supported by the board (%d)\n",
                   i + 1, CONFIG_SYS_FSL_USDHC_NUM);
            return -EINVAL;
        }

        ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
        if (ret)
            return ret;
    }

#endif	//<kuri0423>


    return 0;
#else
    struct src *psrc = (struct src *)SRC_BASE_ADDR;
    unsigned reg = readl(&psrc->sbmr1) >> 11;
    /*
     * Upon reading BOOT_CFG register the following map is done:
     * Bit 11 and 12 of BOOT_CFG register can determine the current
     * mmc port
     * 0x1                  SD2
     * 0x2                  SD3
     * 0x3                  SD4
     * 0x4                  SD1
     */

    switch (reg & 0x3) {
    case 0x1:
        imx_iomux_v3_setup_multiple_pads(
            usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
        usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
        usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
        gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
        break;
    case 0x2:
        imx_iomux_v3_setup_multiple_pads(
            usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
        usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
        usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
        gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
        break;
    case 0x3:
        imx_iomux_v3_setup_multiple_pads(
            usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
        usdhc_cfg[0].esdhc_base = USDHC4_BASE_ADDR;
        usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
        gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
        break;
    case 0x4:
        imx_iomux_v3_setup_multiple_pads(
            usdhc4_pads, ARRAY_SIZE(usdhc1_pads));
        usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
        usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
        gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
        break;
    }

    return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
#endif
}
#endif

#if 0	//<kuri0423> U-BOOTでLAN-PHYは触らない
int mx6_rgmii_rework(struct phy_device *phydev)
{
    unsigned short val;

    /* To enable AR8031 ouput a 125MHz clk from CLK_25M */
    phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
    phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
    phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

    val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
    val &= 0xffe3;
    val |= 0x18;
    phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

    /* introduce tx clock delay */
    phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
    val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
    val |= 0x0100;
    phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

    return 0;
}
#endif	//<kuri0423> U-BOOTでLAN-PHYは触らない

int board_phy_config(struct phy_device *phydev)
{
    printf(" board_phy_config called.\n");
#if 0	//<kuri0423> U-BOOTでLAN-PHYは触らない
    mx6_rgmii_rework(phydev);

    if (phydev->drv->config)
        phydev->drv->config(phydev);
#endif	//<kuri0423> U-BOOTでLAN-PHYは触らない
    return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)

static void disable_lvds(struct display_info_t const *dev)
{
    struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

    int reg = readl(&iomux->gpr[2]);

    reg &= ~(IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
         IOMUXC_GPR2_LVDS_CH1_MODE_MASK);

    writel(reg, &iomux->gpr[2]);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
    disable_lvds(dev);
    imx_enable_hdmi_phy();
}

static void enable_lvds(struct display_info_t const *dev)
{
    struct iomuxc *iomux = (struct iomuxc *)
                IOMUXC_BASE_ADDR;
    u32 reg = readl(&iomux->gpr[2]);
    reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT |
           IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT;
    writel(reg, &iomux->gpr[2]);
}

struct display_info_t const displays[] = {{
    .bus    = -1,
    .addr   = 0,
    .pixfmt = IPU_PIX_FMT_RGB666,
    .detect = NULL,
    .enable = enable_lvds,
    .mode   = {
        .name           = "Hannstar-XGA",
        .refresh        = 60,
        .xres           = 1024,
        .yres           = 768,
        .pixclock       = 15385,
        .left_margin    = 220,
        .right_margin   = 40,
        .upper_margin   = 21,
        .lower_margin   = 7,
        .hsync_len      = 60,
        .vsync_len      = 10,
        .sync           = FB_SYNC_EXT,
        .vmode          = FB_VMODE_NONINTERLACED
/* +Modified for iMX6Solo_PP at 2015/09/08 by Akita */
#ifdef IMX6SOLO_PP
} }, {
    .bus    = 0,
    .addr   = 0,
    /* 18bit LCD panel -> RGB666 format */
    .pixfmt = IPU_PIX_FMT_RGB666,
    .detect = NULL,
    .enable = enable_rgb,
    .mode   = {
        .name           = "VGA-LCD",
        .refresh        = 62,
        .xres           = 640,
        .yres           = 480,
        .pixclock       = 39714,
        .left_margin    = 144,
        .right_margin   = 14,
        .upper_margin   = 35,
        .lower_margin   = 5,
        .hsync_len      = 30,
        .vsync_len      = 3,
        .sync           = 0,
        .vmode          = FB_VMODE_NONINTERLACED
#endif
/* -Modified for iMX6Solo_PP at 2015/09/08 by Akita */
} }, {
    .bus    = -1,
    .addr   = 0,
    .pixfmt = IPU_PIX_FMT_RGB24,
    .detect = detect_hdmi,
    .enable = do_enable_hdmi,
    .mode   = {
        .name           = "HDMI",
        .refresh        = 60,
        .xres           = 1024,
        .yres           = 768,
        .pixclock       = 15385,
        .left_margin    = 220,
        .right_margin   = 40,
        .upper_margin   = 21,
        .lower_margin   = 7,
        .hsync_len      = 60,
        .vsync_len      = 10,
        .sync           = FB_SYNC_EXT,
        .vmode          = FB_VMODE_NONINTERLACED
} }, {
    .bus    = 0,
    .addr   = 0,
    .pixfmt = IPU_PIX_FMT_RGB24,
    .detect = NULL,
    .enable = enable_rgb,
    .mode   = {
        .name           = "SEIKO-WVGA",
        .refresh        = 60,
        .xres           = 800,
        .yres           = 480,
        .pixclock       = 29850,
        .left_margin    = 89,
        .right_margin   = 164,
        .upper_margin   = 23,
        .lower_margin   = 10,
        .hsync_len      = 10,
        .vsync_len      = 10,
        .sync           = 0,
        .vmode          = FB_VMODE_NONINTERLACED
} } };
size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
    struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
    struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
    int reg;

//    printf("SABRE SD: +setup_display\n");
    /* Setup HSYNC, VSYNC, DISP_CLK for debugging purposes */
    imx_iomux_v3_setup_multiple_pads(di0_pads, ARRAY_SIZE(di0_pads));

    enable_ipu_clock();
/* +Modified for iMX6Solo_PP at 2015/09/08 by Akita */
#ifndef IMX6SOLO_PP
    imx_setup_hdmi();
#endif
/* -Modified for iMX6Solo_PP at 2015/09/08 by Akita */

    /* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
    reg = readl(&mxc_ccm->CCGR3);
    reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
    writel(reg, &mxc_ccm->CCGR3);

    /* set LDB0, LDB1 clk select to 011/011 */
    reg = readl(&mxc_ccm->cs2cdr);
    reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
         | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
    reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
          | (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
    writel(reg, &mxc_ccm->cs2cdr);

    reg = readl(&mxc_ccm->cscmr2);
    reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
    writel(reg, &mxc_ccm->cscmr2);

/* +Modified for iMX6Solo_PP at 2015/09/11 by Akita */
#ifdef IMX6SOLO_PP
    reg = readl(&mxc_ccm->chsccdr);
    reg &= ~(MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_MASK
        |MXC_CCM_CHSCCDR_IPU1_DI0_PODF_MASK
        |MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK);
    reg |= (CHSCCDR_CLK_SEL_LDB_DI0
        <<MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET)
          |(CHSCCDR_PODF_DIVIDE_BY_3
        <<MXC_CCM_CHSCCDR_IPU1_DI0_PODF_OFFSET)
          |(CHSCCDR_IPU_PRE_CLK_540M_PFD
        <<MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_OFFSET);
    writel(reg, &mxc_ccm->chsccdr);
#else
    reg = readl(&mxc_ccm->chsccdr);
    reg |= (CHSCCDR_CLK_SEL_LDB_DI0
        << MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
    reg |= (CHSCCDR_CLK_SEL_LDB_DI0
        << MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
    writel(reg, &mxc_ccm->chsccdr);
#endif
/* -Modified for iMX6Solo_PP at 2015/09/11 by Akita */

/* +Modified for iMX6Solo_PP at 2015/09/11 by Akita */
#ifdef IMX6SOLO_PP
    reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
         | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
         | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
         | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
         | IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
         | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
         | IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
         | IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
         | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED;
    writel(reg, &iomux->gpr[2]);

    reg = readl(&iomux->gpr[3]);
    reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK
            | IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
        | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
           << IOMUXC_GPR3_MIPI_MUX_CTL_OFFSET);
    writel(reg, &iomux->gpr[3]);
#else
    reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
         | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
         | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
         | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
         | IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
         | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
         | IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
         | IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
         | IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0;
    writel(reg, &iomux->gpr[2]);

    reg = readl(&iomux->gpr[3]);
    reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK
            | IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
        | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
           << IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET);
    writel(reg, &iomux->gpr[3]);
#endif
/* -Modified for iMX6Solo_PP at 2015/09/11 by Akita */

//    printf("SABRE SD: -setup_display\n");
}

/* +Modified for iMX6Solo_PP at 2015/09/08 by Akita */
#ifdef CONFIG_SPLASH_SCREEN
int splash_screen_prepare(void)
{
    char* pSrcAddr = (char *)IMAGE_BOOT_BMPIMAGE_NOR_PA_START;
    char* s = getenv("splashimage");
    char* pBuffer = NULL;

//    printf("SABRE SD: +splash_screen_prepare\n");
    if (s == NULL)
    {
        return -ENOENT;
    }

    pBuffer = (char *)simple_strtoul(s, NULL, 16);
    memcpy((void*)pBuffer, (void*)pSrcAddr, IMAGE_BOOT_BMPIMAGE_NOR_SIZE);

//    printf("SABRE SD: -splash_screen_prepare\n");
    return 0;
}
#endif /* CONFIG_SPLASH_SCREEN */
/* -Modified for iMX6Solo_PP at 2015/09/08 by Akita */
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
    return 1;
}

int board_eth_init(bd_t *bis)
{
	printf("board_eth_init in.\n");		//<kuri0413>
    setup_iomux_enet();
#if 0	//<kuri0423> PCIは未使用のため削除
    setup_pcie();
#endif	//<kuri0423> PCIは未使用のため削除

    return cpu_eth_init(bis);
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET    0x800
#define UCTRL_PWR_POL       (1 << 9)

static iomux_v3_cfg_t const usb_otg_pads[] = {
    MX6_PAD_EIM_D22__USB_OTG_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_ENET_RX_ER__USB_OTG_ID | MUX_PAD_CTRL(NO_PAD_CTRL),
#if	1	//<kuri0523>
    MX6_PAD_GPIO_3__USB_H1_OC | MUX_PAD_CTRL(GPIO_PU_PAD_CTRL),	//USB OC# Pull-UP
#endif	//<kuri0523>
};

static iomux_v3_cfg_t const usb_hc1_pads[] = {
//    MX6_PAD_SD4_DAT2__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
    //<kuri0423> USB Power(GPIO2_IO10)の設定だが、GPIOではUSBが動作しないのでSD設定のままとしている。
    MX6_PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL), //<kuri0423>なぜかGPIO設定だとうまくUSB認識しない
};

static void setup_usb(void)
{
    imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
                     ARRAY_SIZE(usb_otg_pads));

    /*
     * set daisy chain for otg_pin_id on 6q.
     * for 6dl, this bit is reserved
     */
    imx_iomux_set_gpr_register(1, 13, 1, 0);

    imx_iomux_v3_setup_multiple_pads(usb_hc1_pads,
                     ARRAY_SIZE(usb_hc1_pads));

}

int board_ehci_hcd_init(int port)
{
    u32 *usbnc_usb_ctrl;

    if (port > 1)
        return -EINVAL;

    usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
                 port * 4);

    setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

    return 0;
}

int board_ehci_power(int port, int on)
{
#if 0	//<kuri1014> ボードバージョン毎対応追加
    switch (port) {
    case 0:
        break;
    case 1:
        if (on)
            gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
        else
            gpio_direction_output(IMX_GPIO_NR(2, 10), 0);
        break;
    default:
        printf("MXC USB port %d not yet supported\n", port);
        return -EINVAL;
    }

    return 0;
#else	//<kuri1014> ボードバージョン毎対応追加
    //2次試作以降ボードにも対応
    switch (port) {
    case 0:
        break;
    case 1:
        if (on)
        {
            if(Board_Version == 1)
            {
                //1次試作のとき(HighでON)
                gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
            }
            else
            {
                //2次試作以降のとき(LOWでON)
                gpio_direction_output(IMX_GPIO_NR(2, 10), 0);
            }
        }
        else
        {
            if(Board_Version == 1)
            {
                //1次試作のとき(LOWでOFF)
                gpio_direction_output(IMX_GPIO_NR(2, 10), 0);
            }
            else
            {
                //2次試作以降のとき(HighでOFF)
                gpio_direction_output(IMX_GPIO_NR(2, 10), 1);
            }
        }
        break;
    default:
        printf("MXC USB port %d not yet supported\n", port);
        return -EINVAL;
    }

    return 0;
#endif	//<kuri1014> ボードバージョン毎対応追加
}
#endif

int board_early_init_f(void)
{
    setup_iomux_uart();
/* +Modified for iMX6Solo_PP at 2015/09/11 by Akita *
#if defined(CONFIG_VIDEO_IPUV3)
    setup_display();
#endif
 * -Modified for iMX6Solo_PP at 2015/09/11 by Akita */
    return 0;
}

int board_init(void)
{
    /* address of boot parameters */
    gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
#ifdef IMX6SOLO_PP
    SetupBootMode();

    /* Set Clock Control Module */
    ccm_init();
#endif
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */

#ifdef CONFIG_MXC_SPI
    setup_spi();
#endif
    setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);

/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
#ifdef IMX6SOLO_PP
#ifdef CONFIG_CMD_FLASH
    setup_eimnor();
#endif
    setup_gpio();
#endif
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */

#ifdef CONFIG_USB_EHCI_MX6
    setup_usb();
#endif

/* +Modified for iMX6Solo_PP at 2015/09/11 by Akita */
#if defined(CONFIG_VIDEO_IPUV3)
    setup_display();
#endif
/* -Modified for iMX6Solo_PP at 2015/09/11 by Akita */

    BlinkAllLEDs(0);
    
#if 1	//<kuri0925> START/HOLD off
	SW_LED(0);
#endif	//<kuri0925> START/HOLD off

    return 0;
}

#ifdef CONFIG_POWER
int power_init_board(void)
{
    struct pmic *p;
    unsigned int reg, ret;

/* +Modified for iMX6Solo_PP at 2015/08/13 by Akita */
    return 0;
/* -Modified for iMX6Solo_PP at 2015/08/13 by Akita */
    p = pfuze_common_init(I2C_PMIC);
    if (!p)
        return -ENODEV;

    ret = pfuze_mode_init(p, APS_PFM);
    if (ret < 0)
        return ret;

    /* Increase VGEN3 from 2.5 to 2.8V */
    pmic_reg_read(p, PFUZE100_VGEN3VOL, &reg);
    reg &= ~LDO_VOL_MASK;
    reg |= LDOB_2_80V;
    pmic_reg_write(p, PFUZE100_VGEN3VOL, reg);

    /* Increase VGEN5 from 2.8 to 3V */
    pmic_reg_read(p, PFUZE100_VGEN5VOL, &reg);
    reg &= ~LDO_VOL_MASK;
    reg |= LDOB_3_00V;
    pmic_reg_write(p, PFUZE100_VGEN5VOL, reg);

    return 0;
}
#endif

#ifdef CONFIG_MXC_SPI
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
    return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(4, 9)) : -1;
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
    /* 4 bit bus width */
//<kuri0423>    {"sd2",  MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
    {"sd3",  MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
    /* 8 bit bus width */
//<kuri0423>    {"emmc", MAKE_CFGVAL(0x40, 0x38, 0x00, 0x00)},
    {NULL,   0},
};
#endif

int board_late_init(void)
{
#ifdef CONFIG_CMD_BMODE
    add_board_boot_modes(board_boot_modes);
#endif

/* +Modified for iMX6Solo_PP at 2015/09/10 by Akita */
#ifdef IMX6SOLO_PP
    enable_lcd();
#endif
/* -Modified for iMX6Solo_PP at 2015/09/10 by Akita */

    return 0;
}


int checkboard(void)
{
#if 1	//<kuri0317>

	//GPIOポート設定：GPIO2_08
    imx_iomux_v3_setup_multiple_pads(sdpower_pads, ARRAY_SIZE(sdpower_pads));
	//SD-Power off
	gpio_direction_output(IMX_GPIO_NR(2, 8) , 0);

#endif	//<kuri0317>
/* +Modified for iMX6Solo_PP at 2015/08/18 by Akita */
#ifdef IMX6SOLO_PP
    printf("Board: i.MX6-SABRESD: Pendant iMX6_PP Board\n");
#else
    puts("Board: MX6-SabreSD\n");
#endif
/* +Modified for iMX6Solo_PP at 2015/08/18 by Akita */
    return 0;
}

#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#include <libfdt.h>

const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
    .dram_sdclk_0 =  0x00020030,
    .dram_sdclk_1 =  0x00020030,
    .dram_cas =  0x00020030,
    .dram_ras =  0x00020030,
    .dram_reset =  0x00020030,
    .dram_sdcke0 =  0x00003000,
    .dram_sdcke1 =  0x00003000,
    .dram_sdba2 =  0x00000000,
    .dram_sdodt0 =  0x00003030,
    .dram_sdodt1 =  0x00003030,
    .dram_sdqs0 =  0x00000030,
    .dram_sdqs1 =  0x00000030,
    .dram_sdqs2 =  0x00000030,
    .dram_sdqs3 =  0x00000030,
    .dram_sdqs4 =  0x00000030,
    .dram_sdqs5 =  0x00000030,
    .dram_sdqs6 =  0x00000030,
    .dram_sdqs7 =  0x00000030,
    .dram_dqm0 =  0x00020030,
    .dram_dqm1 =  0x00020030,
    .dram_dqm2 =  0x00020030,
    .dram_dqm3 =  0x00020030,
    .dram_dqm4 =  0x00020030,
    .dram_dqm5 =  0x00020030,
    .dram_dqm6 =  0x00020030,
    .dram_dqm7 =  0x00020030,
};

const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
    .grp_ddr_type =  0x000C0000,
    .grp_ddrmode_ctl =  0x00020000,
    .grp_ddrpke =  0x00000000,
    .grp_addds =  0x00000030,
    .grp_ctlds =  0x00000030,
    .grp_ddrmode =  0x00020000,
    .grp_b0ds =  0x00000030,
    .grp_b1ds =  0x00000030,
    .grp_b2ds =  0x00000030,
    .grp_b3ds =  0x00000030,
    .grp_b4ds =  0x00000030,
    .grp_b5ds =  0x00000030,
    .grp_b6ds =  0x00000030,
    .grp_b7ds =  0x00000030,
};

const struct mx6_mmdc_calibration mx6_mmcd_calib = {
    .p0_mpwldectrl0 =  0x001F001F,
    .p0_mpwldectrl1 =  0x001F001F,
    .p1_mpwldectrl0 =  0x00440044,
    .p1_mpwldectrl1 =  0x00440044,
    .p0_mpdgctrl0 =  0x434B0350,
    .p0_mpdgctrl1 =  0x034C0359,
    .p1_mpdgctrl0 =  0x434B0350,
    .p1_mpdgctrl1 =  0x03650348,
    .p0_mprddlctl =  0x4436383B,
    .p1_mprddlctl =  0x39393341,
    .p0_mpwrdlctl =  0x35373933,
    .p1_mpwrdlctl =  0x48254A36,
};

/* MT41K128M16JT-125 */
static struct mx6_ddr3_cfg mem_ddr = {
    .mem_speed = 1600,
    .density = 2,
    .width = 16,
    .banks = 8,
    .rowaddr = 14,
    .coladdr = 10,
    .pagesz = 2,
    .trcd = 1375,
    .trcmin = 4875,
    .trasmin = 3500,
};

static void ccgr_init(void)
{
    struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

    writel(0x00C03F3F, &ccm->CCGR0);
    writel(0x0030FC03, &ccm->CCGR1);
    writel(0x0FFFC000, &ccm->CCGR2);
    writel(0x3FF00000, &ccm->CCGR3);
    writel(0x00FFF300, &ccm->CCGR4);
    writel(0x0F0000C3, &ccm->CCGR5);
    writel(0x000003FF, &ccm->CCGR6);
}

static void gpr_init(void)
{
    struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

    /* enable AXI cache for VDOA/VPU/IPU */
    writel(0xF00000CF, &iomux->gpr[4]);
    /* set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7 */
    writel(0x007F007F, &iomux->gpr[6]);
    writel(0x007F007F, &iomux->gpr[7]);
}

/*
 * This section requires the differentiation between iMX6 Sabre boards, but
 * for now, it will configure only for the mx6q variant.
 */
static void spl_dram_init(void)
{
    struct mx6_ddr_sysinfo sysinfo = {
        /* width of data bus:0=16,1=32,2=64 */
        .dsize = 2,
        /* config for full 4GB range so that get_mem_size() works */
        .cs_density = 32, /* 32Gb per CS */
        /* single chip select */
        .ncs = 1,
        .cs1_mirror = 0,
        .rtt_wr = 1 /*DDR3_RTT_60_OHM*/,    /* RTT_Wr = RZQ/4 */
        .rtt_nom = 1 /*DDR3_RTT_60_OHM*/,   /* RTT_Nom = RZQ/4 */
        .walat = 1, /* Write additional latency */
        .ralat = 5, /* Read additional latency */
        .mif3_mode = 3, /* Command prediction working mode */
        .bi_on = 1, /* Bank interleaving enabled */
        .sde_to_rst = 0x10, /* 14 cycles, 200us (JEDEC default) */
        .rst_to_cke = 0x23, /* 33 cycles, 500us (JEDEC default) */
    };

    mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
    mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

void board_init_f(ulong dummy)
{
    printf("SABRE SD: +board_init_f\n");
    /* setup AIPS and disable watchdog */
    arch_cpu_init();

    ccgr_init();
    gpr_init();

    /* iomux and setup of i2c */
    board_early_init_f();

    /* setup GP timer */
    timer_init();

    /* UART clocks enabled and gd valid - init serial console */
    preloader_console_init();

    /* DDR initialization */
    spl_dram_init();

    /* Clear the BSS. */
    memset(__bss_start, 0, __bss_end - __bss_start);

    /* load/boot image from boot device */
    board_init_r(NULL, 0);

    printf("SABRE SD: -board_init_f\n");
}

void reset_cpu(ulong addr)
{
}
#endif

/* +Modified for iMX6Solo_PP at 2015/08/18 by Akita */
#ifdef IMX6SOLO_PP
/*---------------------------------------------------------------------------*/
/* iMX6Solo_PP main OS Boot flow                                             */
/*---------------------------------------------------------------------------*/
/* Defines EBOOT.nb0 common area                                             */
/*                                                                           */

#define IMAGE_EBOOT_RAM_BASE_ADDR       0x10041000   /* DRAM physical common area.  */
#define IMAGE_EBOOT_JUMP_ADDR           0x10042000   /* Jump address for EBOOT.nb0  */
#define IMAGE_SHARE_ARGS_RA_CA_START    0x10001000   /* Parameter area.             */
#define IMAGE_SHARE_ARGS_RAM_SIZE       0x00001000   /* Parameter area size.        */
#define IMAGE_BOOT_NKIMAGE_RAM_PA_START 0x10600000   /* Jump address for NK.nb0     */

#define EBOOT_FILENAME          "EBOOT.nb0"
#define INST_TEXT_FILENAME      "2016INST.TXT"
#define READ_FILE_MAX_SIZE      256*1024*1024       /* 256MB TEXT & Image Max size. */

#define TEST_TEXT_FILENAME      "MNT.KEY"	//<ver0.09>自己診断確認用ファイル


#ifdef FASTLOAD	//<kuri0926>
//<kuri1010>#define IMAGE_BOOT_NKIMAGE_RAM_PA_STORE_START 0x19000000   // NK.BIN を一旦保存するRAMエリア(128MBの後半部分)
#define IMAGE_BOOT_NKIMAGE_RAM_PA_STORE_START 0x18000000   // NK.BIN を一旦保存するRAMエリア(128MBの後半部分)
#define	READ_FILE_MAX_SIZE_120	120*1024*1024    // 114MB TEXT & Image Max size.#endif	//<kuri1012>
#endif //FASTLOAD <kuri0926>

#define STORAGE_DEV_NAME_USB    "usb"
#define STORAGE_DEV_NAME_MMC    "mmc"
#define STORAGE_DEV_NUM_USB     "0"
//<kuri0423> #define STORAGE_DEV_NUM_MMC     "3"
#define STORAGE_DEV_NUM_MMC     "0"		//<kuri0423>

/* PWM3 device for buzzer.  */
#define PWM3_ID                 2

/* Array for keypad matrix. */
static ulong KeyKrGPIOPort[8] = {4, 4, 4, 4, 4, 1, 1, 1};
static ulong KeyKsGPIOPort[8] = {4, 4, 4, 4, 4, 1, 1, 1};
static ulong KeyKrGPIOPin[8] = {6, 8, 10, 12, 14 ,0, 12, 4};
static ulong KeyKsGPIOPin[8] = {7, 9, 11, 13, 15 ,11, 13, 5};

iomux_v3_cfg_t key_pads[] = {
    MX6_PAD_KEY_COL0__GPIO4_IO06    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO06:KR0
    MX6_PAD_KEY_COL1__GPIO4_IO08    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO08:KR1
    MX6_PAD_KEY_COL2__GPIO4_IO10    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO10:KR2
    MX6_PAD_KEY_COL3__GPIO4_IO12    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO12:KR3
    MX6_PAD_KEY_COL4__GPIO4_IO14    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO14:KR4
    MX6_PAD_GPIO_0__GPIO1_IO00      | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO1_IO00:KR5
    MX6_PAD_SD2_DAT3__GPIO1_IO12    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO1_IO12:KR6
    MX6_PAD_GPIO_4__GPIO1_IO04      | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO1_IO04:KR7
    MX6_PAD_KEY_ROW0__GPIO4_IO07    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO07:KS0
    MX6_PAD_KEY_ROW1__GPIO4_IO09    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO09:KS1
    MX6_PAD_KEY_ROW2__GPIO4_IO11    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO11:KS2
    MX6_PAD_KEY_ROW3__GPIO4_IO13    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO13:KS3
    MX6_PAD_KEY_ROW4__GPIO4_IO15    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO4_IO15:KS4
    MX6_PAD_SD2_CMD__GPIO1_IO11     | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO1_IO11:KS5
    MX6_PAD_SD2_DAT2__GPIO1_IO13    | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO1_IO13:KS6
#ifdef KEYINFO_EXIST    //2015/07/19 kuri NewPP+
	MX6_PAD_GPIO_5__GPIO1_IO05      | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO1_IO05:KS7
	MX6_PAD_DISP0_DAT20__GPIO5_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),	//DISP0_DAT20 to GPIO5_IO14 for START SWITCH
	MX6_PAD_DISP0_DAT21__GPIO5_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),	//DISP0_DAT21 to GPIO5_IO15 for HOLD SWITCH
	MX6_PAD_DISP0_DAT22__GPIO5_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),	//DISP0_DAT22 to GPIO5_IO16 for MODE0 SWITCH
	MX6_PAD_DISP0_DAT23__GPIO5_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL),	//DISP0_DAT23 to GPIO5_IO17 for MODE1 SWITCH
#else //KEYINFO_EXIST  //2015/07/19 kuri NewPP
	MX6_PAD_GPIO_5__GPIO1_IO05      | MUX_PAD_CTRL(NO_PAD_CTRL)
#endif //KEYINFO_EXIST  //2015/07/19 kuri NewPP-
};

#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
iomux_v3_cfg_t les_pads[] = {
    MX6_PAD_EIM_D16__GPIO3_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO16:K_LED0
    MX6_PAD_EIM_D17__GPIO3_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO17:K_LED1
    MX6_PAD_EIM_D18__GPIO3_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO18:K_LED2
    MX6_PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO22:K_LED5
    MX6_PAD_EIM_D23__GPIO3_IO23 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO23:LED8
    MX6_PAD_EIM_D26__GPIO3_IO26 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO26:LED9
    MX6_PAD_EIM_D27__GPIO3_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO27:LED10
    MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D30__GPIO3_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO31:LED1a-1c
};

iomux_v3_cfg_t sw_les_pads[] = {
    MX6_PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO29:K_LED3
    MX6_PAD_EIM_D20__GPIO3_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO30:K_LED4
};
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
iomux_v3_cfg_t les_pads[] = {
    MX6_PAD_EIM_D16__GPIO3_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO16:LED2
    MX6_PAD_EIM_D17__GPIO3_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO17:LED3
    MX6_PAD_EIM_D18__GPIO3_IO18 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO18:LED4
    MX6_PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO19:LED5
    MX6_PAD_EIM_D20__GPIO3_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO20:LED6
    MX6_PAD_EIM_D22__GPIO3_IO22 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO22:LED7
    MX6_PAD_EIM_D23__GPIO3_IO23 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO23:LED8
    MX6_PAD_EIM_D26__GPIO3_IO26 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO26:LED9
    MX6_PAD_EIM_D27__GPIO3_IO27 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO27:LED10
#if 0	//<kuri0925> START/HOLD は処理を分ける
    MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
    MX6_PAD_EIM_D30__GPIO3_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL),
#endif	//<kuri0925> START/HOLD は処理を分ける
    MX6_PAD_EIM_D31__GPIO3_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO31:LED1a-1c
};

#if 1	//<kuri0925> START/HOLD は処理を分ける
iomux_v3_cfg_t sw_les_pads[] = {
    MX6_PAD_EIM_D29__GPIO3_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO29:HOLD LED
    MX6_PAD_EIM_D30__GPIO3_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO3_IO30:START LED
};
#endif	//<kuri0925> START/HOLD は処理を分ける
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>

static iomux_v3_cfg_t const pwm3_pads[] = {
    MX6_PAD_SD4_DAT1__PWM3_OUT      | MUX_PAD_CTRL(NO_PAD_CTRL),	//GPIO2_IO09:PWM OUT
};

#define BUZZER_PWM_LEVEL                0x0000012C * 0x00000004
#define BUZZER_PWM_PERIOD               0x00000260 * 0x00000004     /*((PWM_SOURCE_CLK / PWM_CLK) - 2)  */
#define BUZZER_BEEP_DURATION            50*1000

#define LED_GPIO3_16        IMX_GPIO_NR(3, 16)
#define LED_GPIO3_17        IMX_GPIO_NR(3, 17)
#define LED_GPIO3_18        IMX_GPIO_NR(3, 18)
#define LED_GPIO3_19        IMX_GPIO_NR(3, 19)
#define LED_GPIO3_20        IMX_GPIO_NR(3, 20)
#define LED_GPIO3_22        IMX_GPIO_NR(3, 22)
#define LED_GPIO3_23        IMX_GPIO_NR(3, 23)
#define LED_GPIO3_26        IMX_GPIO_NR(3, 26)
#define LED_GPIO3_27        IMX_GPIO_NR(3, 27)
#define LED_GPIO3_29        IMX_GPIO_NR(3, 29)
#define LED_GPIO3_30        IMX_GPIO_NR(3, 30)
#define LED_GPIO3_31        IMX_GPIO_NR(3, 31)

#define KEY_KR0             IMX_GPIO_NR(4, 6)
#define KEY_KR1             IMX_GPIO_NR(4, 8)
#define KEY_KR2             IMX_GPIO_NR(4, 10)
#define KEY_KR3             IMX_GPIO_NR(4, 12)
#define KEY_KR4             IMX_GPIO_NR(4, 14)
#define KEY_KR5             IMX_GPIO_NR(1, 0)
#define KEY_KR6             IMX_GPIO_NR(1, 12)
#define KEY_KR7             IMX_GPIO_NR(1, 4)
//<20150906 kuri->	#define KEY_KS0             IMX_GPIO_NR(1, 7)
#define KEY_KS0				IMX_GPIO_NR(4, 7)	//<20150906 kuri+> 定義間違いの修正
#define KEY_KS1             IMX_GPIO_NR(4, 9)
#define KEY_KS2             IMX_GPIO_NR(4, 11)
#define KEY_KS3             IMX_GPIO_NR(4, 13)
#define KEY_KS4             IMX_GPIO_NR(4, 15)
#define KEY_KS5             IMX_GPIO_NR(1, 11)
#define KEY_KS6             IMX_GPIO_NR(1, 13)
#define KEY_KS7             IMX_GPIO_NR(1, 5)

#ifdef KEYINFO_EXIST    //2015/07/19 kuri NewPP+
#define SW_START			IMX_GPIO_NR(5, 14)	//GPIO5_IO14 for START SWITCH
#define SW_HOLD				IMX_GPIO_NR(5, 15)	//GPIO5_IO15 for HOLD SWITCH
#define SW_MODE0			IMX_GPIO_NR(5, 16)	//GPIO5_IO16 for MODE0 SWITCH
#define SW_MODE1			IMX_GPIO_NR(5, 17)	//GPIO5_IO17 for MODE1 SWITCH
#endif //KEYINFO_EXIST  //2015/07/19 kuri NewPP-

#define KEY_MASK_KR0        0x01
#define KEY_MASK_KR1        0x02
#define KEY_MASK_KR2        0x04
#define KEY_MASK_KR3        0x08
#define KEY_MASK_KR4        0x10
#define KEY_MASK_KR5        0x20
#define KEY_MASK_KR6        0x40
#define KEY_MASK_KR7        0x80

#define KEY_MASK_HYPHEN     0x01
#define KEY_MASK_PERIOD     0x02
#define KEY_MASK_NUM2       0x04
#define KEY_MASK_NUM5       0x08
#define KEY_MASK_NUM8       0x10
#define KEY_MASK_LOW        0x20
#define KEY_MASK_HIGHSPPED  0x40
#define KEY_MASK_ANYKEY     0x80

#define KEY_MASK_RSHIFT     0x0100	//<ver0.09>
#define KEY_MASK_ENT        0x0200	//<ver0.09>

//<kuri0924->	#define KEY_SCAN_POLLING    2*1000     /* usec for key scan polling cycle.         */
#define KEY_SCAN_POLLING	10*1000	// usec for key scan polling cycle.<kuri0924+>
#define KEY_SCAN_COUNT      100         /* Num of times of key scan for chattering. */

#if 0	//<kuri0926> Not Use WARING Modify
static void DumpMemory(u32* pAddr, int words)
{
    int index = 0;

    if (pAddr == NULL)
    {
        return;
    }

    printf("DUMP:");
    for (index = 0; index < words; index++)
    {
        if (index % 8 == 0)
        {
            printf("\n0x%08X:", (unsigned int)pAddr);
        }
        printf(" %08X", (unsigned int)*pAddr);
        pAddr++;
    }
    printf("\n\n");
}
#endif	//<kuri0926>  Not Use WARING Modify

static void setup_led(void)
{
    imx_iomux_v3_setup_multiple_pads(les_pads, ARRAY_SIZE(les_pads));
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_direction_output(LED_GPIO3_16, 0);
    gpio_direction_output(LED_GPIO3_17, 0);
    gpio_direction_output(LED_GPIO3_18, 0);
    gpio_direction_output(LED_GPIO3_22, 0);
    gpio_direction_output(LED_GPIO3_23, 0);
    gpio_direction_output(LED_GPIO3_26, 0);
    gpio_direction_output(LED_GPIO3_27, 0);
    gpio_direction_output(LED_GPIO3_29, 0);
    gpio_direction_output(LED_GPIO3_30, 0);
    gpio_direction_output(LED_GPIO3_31, 0);
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_direction_output(LED_GPIO3_16, 0);
    gpio_direction_output(LED_GPIO3_17, 0);
    gpio_direction_output(LED_GPIO3_18, 0);
    gpio_direction_output(LED_GPIO3_19, 0);
    gpio_direction_output(LED_GPIO3_20, 0);
    gpio_direction_output(LED_GPIO3_22, 0);
    gpio_direction_output(LED_GPIO3_23, 0);
    gpio_direction_output(LED_GPIO3_26, 0);
    gpio_direction_output(LED_GPIO3_27, 0);
#if 0	//<kuri0925>  START/HOLD は処理を分ける
    gpio_direction_output(LED_GPIO3_29, 0);
    gpio_direction_output(LED_GPIO3_30, 0);
#endif	//<kuri0925>  START/HOLD は処理を分ける
    gpio_direction_output(LED_GPIO3_31, 0);
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
}

#if 1	//<kuri0925>  START/HOLD は処理を分ける
static void setup_sw_led(void)
{
    imx_iomux_v3_setup_multiple_pads(sw_les_pads, ARRAY_SIZE(sw_les_pads));
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_direction_output(LED_GPIO3_19, 0);
    gpio_direction_output(LED_GPIO3_20, 0);
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_direction_output(LED_GPIO3_29, 0);
    gpio_direction_output(LED_GPIO3_30, 0);
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
}
#endif	//<kuri0925>  START/HOLD は処理を分ける


void BlinkFourLEDs(int led)
{
#ifdef BSP_SUPPORT_AUTOSELECT	//<kuri0706> 自動判別対応

	if(g_AutoSel != 0)
	{
		//TOYOTA仕様の場合、LEDはGPIO3_31のみ
	    switch (led)
	    {
	    case 0:
	    case 2:
	        /* Turn the LED on.    */
	        gpio_set_value(LED_GPIO3_31, 1);
	        break;
	
	    case 1:
	    case 3:
	        /* Turn the LED OFF.    */
	        gpio_set_value(LED_GPIO3_31, 0);
	        break;
	
	    default:
	        break;
	    }
	}
	else
	{
		//標準仕様のとき
	    switch (led)
	    {
	    case 0:
	        /* Turn the LED4 on.    */
	        gpio_set_value(LED_GPIO3_18, 1);
	        gpio_set_value(LED_GPIO3_19, 0);
	        gpio_set_value(LED_GPIO3_22, 0);
	        gpio_set_value(LED_GPIO3_20, 0);
	        break;
	
	    case 1:
	        /* Turn the LED5 on.    */
	        gpio_set_value(LED_GPIO3_18, 0);
	        gpio_set_value(LED_GPIO3_19, 1);
	        gpio_set_value(LED_GPIO3_22, 0);
	        gpio_set_value(LED_GPIO3_20, 0);
	        break;
	
	    case 2:
	        /* Turn the LED7 on.    */
	        gpio_set_value(LED_GPIO3_18, 0);
	        gpio_set_value(LED_GPIO3_19, 0);
	        gpio_set_value(LED_GPIO3_22, 1);
	        gpio_set_value(LED_GPIO3_20, 0);
	        break;
	
	    case 3:
	        /* Turn the LED6 on.    */
	        gpio_set_value(LED_GPIO3_18, 0);
	        gpio_set_value(LED_GPIO3_19, 0);
	        gpio_set_value(LED_GPIO3_22, 0);
	        gpio_set_value(LED_GPIO3_20, 1);
	        break;
	
	    default:
	        break;
	    }	
	}
#else	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応
	
#ifdef	BSP_SUPPORT_TOYOTA	//<kuri0426>
	//TOYOTA仕様の場合、LEDはGPIO3_31のみ
    switch (led)
    {
    case 0:
    case 2:
        /* Turn the LED on.    */
        gpio_set_value(LED_GPIO3_31, 1);
        break;

    case 1:
    case 3:
        /* Turn the LED OFF.    */
        gpio_set_value(LED_GPIO3_31, 0);
        break;

    default:
        break;
    }
#else	//BSP_SUPPORT_TOYOTA <kuri0426>
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20190819>
	//仕様
    switch (led)
    {
    case 0:
    case 2:
        /* K_LED0:ON  K_LED1:OFF*/
        gpio_set_value(LED_GPIO3_16, 1);
        gpio_set_value(LED_GPIO3_17, 0);
        break;

    case 1:
    case 3:
        /* K_LED0:OFF  K_LED1:ON*/
        gpio_set_value(LED_GPIO3_16, 0);
        gpio_set_value(LED_GPIO3_17, 1);
        break;

    default:
        break;
    }
#else	//BSP_SUPPORT_HIRATA <tanaka20190819>
    switch (led)
    {
    case 0:
        /* Turn the LED4 on.    */
        gpio_set_value(LED_GPIO3_18, 1);
        gpio_set_value(LED_GPIO3_19, 0);
        gpio_set_value(LED_GPIO3_22, 0);
        gpio_set_value(LED_GPIO3_20, 0);
        break;

    case 1:
        /* Turn the LED5 on.    */
        gpio_set_value(LED_GPIO3_18, 0);
        gpio_set_value(LED_GPIO3_19, 1);
        gpio_set_value(LED_GPIO3_22, 0);
        gpio_set_value(LED_GPIO3_20, 0);
        break;

    case 2:
        /* Turn the LED7 on.    */
        gpio_set_value(LED_GPIO3_18, 0);
        gpio_set_value(LED_GPIO3_19, 0);
        gpio_set_value(LED_GPIO3_22, 1);
        gpio_set_value(LED_GPIO3_20, 0);
        break;

    case 3:
        /* Turn the LED6 on.    */
        gpio_set_value(LED_GPIO3_18, 0);
        gpio_set_value(LED_GPIO3_19, 0);
        gpio_set_value(LED_GPIO3_22, 0);
        gpio_set_value(LED_GPIO3_20, 1);
        break;

    default:
        break;
    }
#endif	//BSP_SUPPORT_HIRATA <tanaka20190819>
#endif	//BSP_SUPPORT_TOYOTA <kuri0426>
#endif	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応
}

void BlinkAllLEDs(int on)
{
    setup_led();

#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_set_value(LED_GPIO3_16, on);
    gpio_set_value(LED_GPIO3_17, on);
    gpio_set_value(LED_GPIO3_18, on);
    gpio_set_value(LED_GPIO3_22, on);
    //未使用
    gpio_set_value(LED_GPIO3_23, 0);
    gpio_set_value(LED_GPIO3_26, 0);
    gpio_set_value(LED_GPIO3_27, 0);
    gpio_set_value(LED_GPIO3_29, 0);
    gpio_set_value(LED_GPIO3_30, 0);
    gpio_set_value(LED_GPIO3_31, 0);
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_set_value(LED_GPIO3_16, on);
    gpio_set_value(LED_GPIO3_17, on);
    gpio_set_value(LED_GPIO3_18, on);
    gpio_set_value(LED_GPIO3_19, on);
    gpio_set_value(LED_GPIO3_20, on);
    gpio_set_value(LED_GPIO3_22, on);
    gpio_set_value(LED_GPIO3_23, on);
    gpio_set_value(LED_GPIO3_26, on);
    gpio_set_value(LED_GPIO3_27, on);
#if 0	//<kuri0925>  START/HOLD は処理を分ける
    gpio_set_value(LED_GPIO3_29, on);
    gpio_set_value(LED_GPIO3_30, on);
#endif	//<kuri0925>  START/HOLD は処理を分ける
    gpio_set_value(LED_GPIO3_31, on);
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
}

#if 1	//<kuri0925>  START/HOLD は処理を分ける
void SW_LED(int on)
{
    setup_sw_led();
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_set_value(LED_GPIO3_19, on);
    gpio_set_value(LED_GPIO3_20, on);
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_set_value(LED_GPIO3_29, on);
    gpio_set_value(LED_GPIO3_30, on);
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
}
#endif	//<kuri0925>  START/HOLD は処理を分ける

void BuzzerInit(void)
{
    /* Initialize PWM3 for buzzer.  */
    imx_iomux_v3_setup_multiple_pads(pwm3_pads, ARRAY_SIZE(pwm3_pads));

    pwm_init(PWM3_ID, 0, 0);
    pwm_config_direct(PWM3_ID, BUZZER_PWM_LEVEL, BUZZER_PWM_PERIOD);
}

void BuzzerBeep(u32 beepTime)
{
#ifdef	BSP_SUPPORT_HIRATA
    //APPと区別のため、変更
    pwm_enable(PWM3_ID);
    udelay(beepTime/2);
    pwm_disable(PWM3_ID);

    udelay(beepTime);

    pwm_enable(PWM3_ID);
    udelay(beepTime/2);
    pwm_disable(PWM3_ID);

#else	//BSP_SUPPORT_HIRATA
    /* Start beep.  */
    pwm_enable(PWM3_ID);
    udelay(beepTime);
    /* Stop beep.   */
    pwm_disable(PWM3_ID);
#endif	//BSP_SUPPORT_HIRATA
}

void key_init(void)
{
/*    printf ("+KeyPortInit\n"); */

    imx_iomux_v3_setup_multiple_pads(key_pads, ARRAY_SIZE(key_pads));

    /* KR0 GPIO4_IO6（IN)   */
    gpio_direction_input(KEY_KR0);
    /* KR1 GPIO4_IO8（IN)   */
    gpio_direction_input(KEY_KR1);
    /* KR2 GPIO4_IO10（IN)  */
    gpio_direction_input(KEY_KR2);
    /* KR3 GPIO4_IO12（IN)  */
    gpio_direction_input(KEY_KR3);
    /* KR4 GPIO4_IO14（IN)  */
    gpio_direction_input(KEY_KR4);
    /* KR5 GPIO1_IO0（IN)   */
    gpio_direction_input(KEY_KR5);
    /* KR6 GPIO1_IO12（IN)  */
    gpio_direction_input(KEY_KR6);
    /* KR7 GPIO1_IO4（IN)   */
    gpio_direction_input(KEY_KR7);
    /* KS0 GPIO4_IO7（OUT)  */
    gpio_direction_output(KEY_KS0, 1);
    /* KS1 GPIO4_IO9（OUT)  */
    gpio_direction_output(KEY_KS1, 1);
    /* KS2 GPIO4_IO11（OUT) */
    gpio_direction_output(KEY_KS2, 1);
    /* KS3 GPIO4_IO13（OUT) */
    gpio_direction_output(KEY_KS3, 1);
    /* KS4 GPIO4_IO15（OUT) */
    gpio_direction_output(KEY_KS4, 1);
    /* KS5 GPIO1_IO11（OUT) */
    gpio_direction_output(KEY_KS5, 1);
    /* KS6 GPIO1_IO13（OUT) */
    gpio_direction_output(KEY_KS6, 1);
    /* KS7 GPIO1_IO5（OUT)  */
    gpio_direction_output(KEY_KS7, 1);
    
#ifdef KEYINFO_EXIST    //2015/07/19 kuri NewPP+
	//GPIO5_IO14 for START SWITCH
	gpio_direction_input(SW_START);
	//GPIO5_IO15 for HOLD SWITCH
	gpio_direction_input(SW_HOLD);
	//GPIO5_IO16 for MODE0 SWITCH
	gpio_direction_input(SW_MODE0);
	//GPIO5_IO17 for MODE1 SWITCH
	gpio_direction_input(SW_MODE1);
#endif //KEYINFO_EXIST  //2015/07/19 kuri NewPP-

    udelay(100);
    /* 読み出し列セット */
    gpio_set_value(IMX_GPIO_NR(KeyKsGPIOPort[0], KeyKsGPIOPin[0]), 0);

/*    printf ("-KeyPortInit\n"); */
    return;
}

char KeyScan(void)
{
    char    mtrxScan = 0;               /* マトリクススキャン */
    u32     dwPinValue = 0;
    int     index = 0;

    for (index = 7; index >= 0; index--)
    {
        /* キー読み出し */
        dwPinValue = gpio_get_value(IMX_GPIO_NR(KeyKrGPIOPort[index], KeyKrGPIOPin[index]));
        mtrxScan = mtrxScan | (dwPinValue & 0x00000001);
        if (index != 0)
        {
            mtrxScan = mtrxScan << 1;
        }
    }
    mtrxScan = ~mtrxScan;

    return mtrxScan;
}

#ifdef BSP_SUPPORT_AUTOSELECT	//<kuri0706> 自動判別対応

//#define AutoSelectDBG 1		//正式化時に0もしくはコメントにすること。

//自動判別を行う関数。key_init()直後に呼び出すこと。
int AutoSelect(void)
{
    u32     dwPinValue = 0;

	//最初にKR0を出力にしておく。
    /* KR0 GPIO4_IO6(OUT)   */
    gpio_direction_output(KEY_KR0,1);		//High出力
	udelay(100);							//少し待つ
    gpio_set_value(KEY_KR0, 1);				//KR0=1出力
	udelay(100);							//少し待つ
	dwPinValue = gpio_get_value(KEY_KR1);	//KR1読み込み
	
	if(dwPinValue & 0x00000001)
	{
		#if		AutoSelectDBG
		printf("  u-boot AutoSelect debug 1.\n");	//debug
		#endif	//AutoSelectDBG
		//Highを読み取った＝ショートされている可能性があるので次はLOW出力と読み取り
		gpio_set_value(KEY_KR0, 0);				//KR0=0出力
		udelay(100);							//少し待つ
		dwPinValue = gpio_get_value(KEY_KR1);	//KR1読み込み
		
		if(dwPinValue & 0x00000001)
		{
			#if		AutoSelectDBG
			printf("  u-boot AutoSelect debug 2.\n");	//debug
			#endif	//AutoSelectDBG
			//Highを読み取った＝ショートされていないので標準確定
			g_AutoSel = 0;	//機種自動判別結果保管。=0:標準とする。
		}
		else
		{
			#if		AutoSelectDBG
			printf("  u-boot AutoSelect debug 3.\n");	//debug
			#endif	//AutoSelectDBG
			//Lowを読み取った＝ショートされているのでTOYOTA確定
			g_AutoSel = 1;	//機種自動判別結果保管。=1:TOYOTAとする。
		}
	}
	else
	{
		#if		AutoSelectDBG
		printf("  u-boot AutoSelect debug 4.\n");	//debug
		#endif	//AutoSelectDBG
		//LOWを読み取った＝ショートされていないので標準確定
		g_AutoSel = 0;	//機種自動判別結果保管。=0:標準とする。
	}
	
	//最後にKR0を入力に戻しておく。
    /* KR0 GPIO4_IO6(in)   */
    gpio_direction_input(KEY_KR0);
	udelay(100);							//少し待つ

#ifdef BSP_SELECT_FIX_TOYOTA //<kuri0906> 自動判別TOYOTA固定対応
	//BSP_SELECT_FIX_TOYOTAマクロ有効時には自動判別するが結果はTOYOTA固定(自動判別未対応用ハードのため)
	g_AutoSel = 1;	//機種自動判別結果保管。=1:TOYOTAとする。
	printf("APP Machine Type : AutoSelect Boot , but TOYOTA FIXED. \n");
#endif //BSP_SELECT_FIX_TOYOTA <kuri0906> 自動判別TOYOTA固定対応

	//機種選択結果表示
	if(g_AutoSel == 0)
	{
		printf("APP Machine Type = STANDARD. \n");
	}
	else
	{
		printf("APP Machine Type = TOYOTA. \n");
	}
	
	return 0;	//終了

}
#endif	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応


#ifdef KEYINFO_EXIST    //2015/07/19 kuri NewPP+
int DetectBootKey(void)
{
	int index;
	//int counter;
	int scancode = 0;
	char mtrxScan,mtrxScan_chk;
	PBSP_BOOT_INFO	pBoootInfo;		//起動情報
	pBoootInfo = (PBSP_BOOT_INFO)(IMAGE_SHARE_ARGS_RA_CA_START + 
					  IMAGE_SHARE_ARGS_RAM_SIZE - 
					  sizeof(BSP_BOOT_INFO));	// common PARAM area.

	//printf("+DetectBootKey New pBootInfo addr = %X\n",(unsigned int)pBoootInfo); //debug
    //printf(" pBootInfo size = %X\n",sizeof(BSP_BOOT_INFO));        //debug
    //printf(" unsigned int size = %X , unsigned char size = %X\n",sizeof(unsigned int),sizeof(unsigned char));      //debug
	//デバッグ用出力
	//printf(" KEY = ");

	//起動時のKS/KRキー読み取りは一度だけ。(現行機に合わせてチャタリング無視)
	for (index = 0; index < 8; index++)
	{
		//対象のKSをOFF
		gpio_set_value(IMX_GPIO_NR(KeyKsGPIOPort[index], KeyKsGPIOPin[index]), 0);
		//10msecまち
		udelay(KEY_SCAN_POLLING);
		//キーリード
		mtrxScan = KeyScan();
		//読み取ったKR情報を共有メモリにCOPY
		pBoootInfo->KeyInfo[index] = mtrxScan;
		//デバッグ用出力
	//	printf("0x%X ",mtrxScan);
		//対象のKSをON(戻す)
		gpio_set_value(IMX_GPIO_NR(KeyKsGPIOPort[index], KeyKsGPIOPin[index]), 1);
		//少しディレイ(100usec)
		udelay(KEY_SCAN_POLLING/100);
	}
	//KST読み取り
	mtrxScan_chk = 0;	//初期化
	mtrxScan = 0;		//初期化
	mtrxScan_chk = gpio_get_value(SW_MODE1);	//MODE1
	mtrxScan |= ((mtrxScan_chk << 3) & 0x08);	//b3 only
	mtrxScan_chk = gpio_get_value(SW_MODE0);	//MODE0
	mtrxScan |= ((mtrxScan_chk << 2) & 0x04);	//b2 only
	mtrxScan_chk = gpio_get_value(SW_HOLD);		//HOLD
	mtrxScan |= ((mtrxScan_chk << 1) & 0x02);	//b1 only
	mtrxScan_chk = gpio_get_value(SW_START);	//START
	mtrxScan |= (mtrxScan_chk & 0x01);			//b0 only
	
	mtrxScan &= 0x0F;							//High 4 bit = 0

	//読み取ったKR情報を共有メモリにCOPY
	pBoootInfo->KeyInfo[8] = mtrxScan;
	//デバッグ用出力
	//printf("0x%2X \n",mtrxScan);
	
	//あらかじめ読み取ったキー入力情報から１つずつ取り出して比較
	for (index = 0; index < 8; index++)
	{
		//共有メモリから１つずつ取り出して比較
		mtrxScan = pBoootInfo->KeyInfo[index];
		
#ifdef BSP_SUPPORT_AUTOSELECT	//<kuri0706> 自動判別対応

		if(g_AutoSel != 0)
		{
			//TOYOTA版のときのキー入力状態チェック

			switch (index)
			{
			case 0:	//KS0
				if (mtrxScan & 0xFC)	//b0,b1=empty
				{
					scancode |= KEY_MASK_ANYKEY;
				}
				break;
			case 6:	//KS6
				if (mtrxScan != 0)
				{
					scancode |= KEY_MASK_ANYKEY;
				}
				break;
			case 7:	//KS7
				if (mtrxScan & 0x03)	//b7-b2:empty
				{
					scancode |= KEY_MASK_ANYKEY;
				}
				break;
	
			case 1:
				//TOYOTA "RX-" ＝ 標準 "高速"
				if (mtrxScan & KEY_MASK_KR7)
				{
					printf("Key: RX-\n");
					scancode |= KEY_MASK_HIGHSPPED;
				}
				//上記以外のキー入力があったとき
	            if (mtrxScan & (~KEY_MASK_KR7))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 2:
				//TOYOTA "RZ+" ＝ 標準 "低"
				if (mtrxScan & KEY_MASK_KR4)
				{
					printf("Key: RZ+\n");
					scancode |= KEY_MASK_LOW;
				}
				//上記以外のキー入力があったとき
	            if (mtrxScan & (~KEY_MASK_KR4))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 3:
				if (mtrxScan & KEY_MASK_KR7)
				{
					printf("Key: 5\n");
					scancode |= KEY_MASK_NUM5;
				}
				if (mtrxScan & KEY_MASK_KR1)
				{
					printf("Key: 8\n");
					scancode |= KEY_MASK_NUM8;
				}
				//上記以外のキー入力があったとき
	            if (mtrxScan & ((~KEY_MASK_KR7) & (~KEY_MASK_KR1) & 0x9F))	//b6,b5=empty
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 4:
				if (mtrxScan & KEY_MASK_KR7)
				{
					printf("Key: .\n");
					scancode |= KEY_MASK_PERIOD;
				}
				if (mtrxScan & KEY_MASK_KR3)
				{
					printf("Key: 2\n");
					scancode |= KEY_MASK_NUM2;
				}
				//上記以外のキー入力があったとき
	            if (mtrxScan & ((~KEY_MASK_KR7) & (~KEY_MASK_KR3)))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 5:
				if (mtrxScan & KEY_MASK_KR0)
				{
					printf("Key: BS\n");
					scancode |= KEY_MASK_HYPHEN;
				}
				//上記以外のキー入力があったとき
		         if (mtrxScan & (~KEY_MASK_KR0))
		         {
		             scancode |= KEY_MASK_ANYKEY;
		         }
				break;
	
			default:
				break;
			}
		}

		//機種タイプが標準のとき
		else
		{
			//標準版のときのキー入力状態チェック

			switch (index)
			{
			case 0:
			case 1:
			case 3:
				if (mtrxScan != 0)
				{
					scancode |= KEY_MASK_ANYKEY;
				}
				break;
	
			case 2:
				if (mtrxScan & KEY_MASK_KR5)
				{
//					printf("Key: High Speed\n");
					scancode |= KEY_MASK_HIGHSPPED;
				}
				if (mtrxScan & KEY_MASK_KR7)
				{
//					printf("Key: Low\n");
					scancode |= KEY_MASK_LOW;
				}
	            if (mtrxScan & ((~KEY_MASK_KR5) & (~KEY_MASK_KR7)))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 4:
				if (mtrxScan & KEY_MASK_KR3)
				{
//					printf("Key: 8\n");
					scancode |= KEY_MASK_NUM8;
				}
				if (mtrxScan & KEY_MASK_KR6)
				{
//					printf("Key: R-Shift\n");
					scancode |= KEY_MASK_RSHIFT;
				}

	            if (mtrxScan & ((~KEY_MASK_KR3)&(~KEY_MASK_KR6)))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 5:
				if (mtrxScan & KEY_MASK_KR2)
				{
//					printf("Key: 5\n");
					scancode |= KEY_MASK_NUM5;
				}
	            if (mtrxScan & (~KEY_MASK_KR2))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			case 6:
				if (mtrxScan & KEY_MASK_KR1)
				{
//					printf("Key: 2\n");
					scancode |= KEY_MASK_NUM2;
				}
	            if (mtrxScan & (~KEY_MASK_KR1))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;

			case 7:
				if (mtrxScan & KEY_MASK_KR0)
				{
//					printf("Key: .\n");
					scancode |= KEY_MASK_PERIOD;
				}
				if (mtrxScan & KEY_MASK_KR1)
				{
//					printf("Key: -\n");
					scancode |= KEY_MASK_HYPHEN;
				}
				if (mtrxScan & KEY_MASK_KR3)
				{
//					printf("Key: ENT\n");
					scancode |= KEY_MASK_ENT;
				}
	            //KS7はKR7-5は空きなので見ないようにする
	            if (mtrxScan & (((~KEY_MASK_KR0) & (~KEY_MASK_KR1) & (~KEY_MASK_KR3)) &0x1F))
	            {
	                scancode |= KEY_MASK_ANYKEY;
	            }
				break;
	
			default:
				break;
			}
		}
		
#else	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応

#ifdef	BSP_SUPPORT_TOYOTA	//<kuri0426>
		switch (index)
		{
		case 0:	//KS0
			if (mtrxScan & 0xFC)	//b0,b1=empty
			{
				scancode |= KEY_MASK_ANYKEY;
			}
			break;
		case 6:	//KS6
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
			break;
		case 7:	//KS7
			if (mtrxScan & 0x03)	//b7-b2:empty
			{
				scancode |= KEY_MASK_ANYKEY;
			}
			break;

		case 1:
			//TOYOTA "RX-" ＝ 標準 "高速"
			if (mtrxScan & KEY_MASK_KR7)
			{
				printf("Key: RX-\n");
				scancode |= KEY_MASK_HIGHSPPED;
			}
			//上記以外のキー入力があったとき
            if (mtrxScan & (~KEY_MASK_KR7))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
			break;

		case 2:
			//TOYOTA "RZ+" ＝ 標準 "低"
			if (mtrxScan & KEY_MASK_KR4)
			{
				printf("Key: RZ+\n");
				scancode |= KEY_MASK_LOW;
			}
			//上記以外のキー入力があったとき
            if (mtrxScan & (~KEY_MASK_KR4))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
			break;

		case 3:
			if (mtrxScan & KEY_MASK_KR7)
			{
				printf("Key: 5\n");
				scancode |= KEY_MASK_NUM5;
			}
			if (mtrxScan & KEY_MASK_KR1)
			{
				printf("Key: 8\n");
				scancode |= KEY_MASK_NUM8;
			}
			//上記以外のキー入力があったとき
            if (mtrxScan & ((~KEY_MASK_KR7) & (~KEY_MASK_KR1) & 0x9F))	//b6,b5=empty
            {
                scancode |= KEY_MASK_ANYKEY;
            }
			break;

		case 4:
			if (mtrxScan & KEY_MASK_KR7)
			{
				printf("Key: .\n");
				scancode |= KEY_MASK_PERIOD;
			}
			if (mtrxScan & KEY_MASK_KR3)
			{
				printf("Key: 2\n");
				scancode |= KEY_MASK_NUM2;
			}
			//上記以外のキー入力があったとき
            if (mtrxScan & ((~KEY_MASK_KR7) & (~KEY_MASK_KR3)))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
			break;

		case 5:
			if (mtrxScan & KEY_MASK_KR0)
			{
				printf("Key: BS\n");
				scancode |= KEY_MASK_HYPHEN;
			}
			//上記以外のキー入力があったとき
            if (mtrxScan & (~KEY_MASK_KR0))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
			break;

		default:
			break;
		}
		
#else	//BSP_SUPPORT_TOYOTA <kuriu0426>
//#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20190819> -->APPと同じ起動キーのため、処理をわけない

//#else	//BSP_SUPPORT_HIRATA <tanaka20190819> -->APPと同じ起動キーのため、処理をわけない
		switch (index)
		{
		case 0:
		case 1:
		case 3:
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
			break;

		case 2:
			if (mtrxScan & KEY_MASK_KR5)
			{
//				printf("Key: High Speed\n");
				scancode |= KEY_MASK_HIGHSPPED;
			}
			if (mtrxScan & KEY_MASK_KR7)
			{
//				printf("Key: Low\n");
				scancode |= KEY_MASK_LOW;
			}
#if 0	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
#else	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
            if (mtrxScan & ((~KEY_MASK_KR5) & (~KEY_MASK_KR7)))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
#endif	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			break;

		case 4:
			if (mtrxScan & KEY_MASK_KR3)
			{
//				printf("Key: 8\n");
				scancode |= KEY_MASK_NUM8;
			}
	#if 1	//<ver0.09>
			if (mtrxScan & KEY_MASK_KR6)
			{
//				printf("Key: R-Shift\n");
				scancode |= KEY_MASK_RSHIFT;
			}
	#endif	//<ver0.09>

#if 0	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
#else	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
	#if 0	//<ver0.09>
            if (mtrxScan & (~KEY_MASK_KR3))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
    #else	//<ver0.09>
            if (mtrxScan & ((~KEY_MASK_KR3)&(~KEY_MASK_KR6)))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
    #endif	//<ver0.09>
#endif	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			break;

		case 5:
			if (mtrxScan & KEY_MASK_KR2)
			{
//				printf("Key: 5\n");
				scancode |= KEY_MASK_NUM5;
			}
#if 0	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
#else	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
            if (mtrxScan & (~KEY_MASK_KR2))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
#endif	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			break;

		case 6:
			if (mtrxScan & KEY_MASK_KR1)
			{
//				printf("Key: 2\n");
				scancode |= KEY_MASK_NUM2;
			}
#if 0	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
#else	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
            if (mtrxScan & (~KEY_MASK_KR1))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
#endif	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			break;

		case 7:
			if (mtrxScan & KEY_MASK_KR0)
			{
//				printf("Key: .\n");
				scancode |= KEY_MASK_PERIOD;
			}
			if (mtrxScan & KEY_MASK_KR1)
			{
//				printf("Key: -\n");
				scancode |= KEY_MASK_HYPHEN;
			}
	#if 1	//<ver0.09>
			if (mtrxScan & KEY_MASK_KR3)
			{
//				printf("Key: ENT\n");
				scancode |= KEY_MASK_ENT;
			}
	#endif	//<ver0.09>
#if 0	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			if (mtrxScan != 0)
			{
				scancode |= KEY_MASK_ANYKEY;
			}
#else	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
	#if 0	//<ver0.09>
            //KS7はKR7-5は空きなので見ないようにする
            if (mtrxScan & (((~KEY_MASK_KR0) & (~KEY_MASK_KR1)) &0x1F))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
    #else	//<ver0.09>
            //KS7はKR7-5は空きなので見ないようにする
            if (mtrxScan & (((~KEY_MASK_KR0) & (~KEY_MASK_KR1) & (~KEY_MASK_KR3)) &0x1F))
            {
                scancode |= KEY_MASK_ANYKEY;
            }
    #endif	//<ver0.09>
#endif	//<ver0.07> 起動キー以外がONしていたらKEY_MASK_ANYKEYをONする
			break;

		default:
			break;
		}
//#endif	//BSP_SUPPORT_HIRATA <tanaka20190819> -->APPと同じ起動キーのため、処理をわけない
#endif	//BSP_SUPPORT_TOYOTA <kuriu0426>

#endif	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応
	}
	
// NewPP+ 20150727 バージョン情報追加
	memset ( pBoootInfo->UbootVer , 0x00 , 16);
	//printf ("U-BOOT version memset\n");
	
#ifdef BSP_SUPPORT_AUTOSELECT	//<kuri0706> 自動判別対応
	if(g_AutoSel == 0)
	{
		//標準版
		memcpy ( pBoootInfo->UbootVer , UBOOT_VER , 16);
	}
	else
	{
		//TOYOTA版
		memcpy ( pBoootInfo->UbootVer , UBOOT_VERT , 16);
	}
	printf ("U-BOOT original version = %s\n", pBoootInfo->UbootVer);
	
#else	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応
	memcpy ( pBoootInfo->UbootVer , UBOOT_VER , 16);
	printf ("U-BOOT original version = %s\n", pBoootInfo->UbootVer);
#endif	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応

// NewPP- 20150727 バージョン情報追加

#ifdef BSP_SUPPORT_VERCHK	//<kuri0428>
	//NK.BIN内のBootバージョンチェック情報を保管する処理を追加
	//バージョンチェック情報を保管するエリア初期化
	memset ( Allow_Boot_Version , 0x00 , 8);
	//printf ("Allow_Boot_Version memset\n");
#endif	//BSP_SUPPORT_VERCHK <kuri0428>
	return scancode;
}

#else //KEYINFO_EXIST    //2015/07/19 kuri NewPP+

int DetectBootKey(void)
{
    int index;
    int counter;
    int scancode = 0;
    char mtrxScan;

/*    printf("+DetectBootKey\n"); */
    for (counter = 0; counter < KEY_SCAN_COUNT; counter++)
    {
        for (index = 0; index < 8; index++)
        {
            gpio_set_value(IMX_GPIO_NR(KeyKsGPIOPort[index], KeyKsGPIOPin[index]), 0);
            mtrxScan = KeyScan();
/*          printf("DetectBootKey: index %d : mtrxScan = %d\n", index, mtrxScan);   */

            switch (index)
            {
            case 0:
            case 1:
            case 3:
                if (mtrxScan != 0)
                {
                    scancode |= KEY_MASK_ANYKEY;
                }
                break;

            case 2:
                if (mtrxScan & KEY_MASK_KR5)
                {
                    scancode |= KEY_MASK_HIGHSPPED;
                }
                if (mtrxScan & KEY_MASK_KR7)
                {
                    scancode |= KEY_MASK_LOW;
                }
                if (mtrxScan != 0)
                {
                    scancode |= KEY_MASK_ANYKEY;
                }
                break;

                break;

            case 4:
                if (mtrxScan & KEY_MASK_KR3)
                {
                    scancode |= KEY_MASK_NUM8;
                }
                if (mtrxScan != 0)
                {
                    scancode |= KEY_MASK_ANYKEY;
                }
                break;

            case 5:
                if (mtrxScan & KEY_MASK_KR2)
                {
                    scancode |= KEY_MASK_NUM5;
                }
                if (mtrxScan != 0)
                {
                    scancode |= KEY_MASK_ANYKEY;
                }
               break;

            case 6:
                if (mtrxScan & KEY_MASK_KR1)
                {
                    scancode |= KEY_MASK_NUM2;
                }
                if (mtrxScan != 0)
                {
                    scancode |= KEY_MASK_ANYKEY;
                }
                break;

            case 7:
                if (mtrxScan & KEY_MASK_KR0)
                {
                    scancode |= KEY_MASK_PERIOD;
                }
                if (mtrxScan & KEY_MASK_KR1)
                {
                    scancode |= KEY_MASK_HYPHEN;
                }
                if (mtrxScan != 0)
                {
                    scancode |= KEY_MASK_ANYKEY;
                }
                break;

            default:
                break;
            }
            gpio_set_value(IMX_GPIO_NR(KeyKsGPIOPort[index], KeyKsGPIOPin[index]), 1);
        }
        udelay(KEY_SCAN_POLLING);
    }

    return scancode;
}

#endif //KEYINFO_EXIST  //2015/07/19 kuri NewPP-

static void SetupBootMode(void)
{
    int keycode;

/*    printf ("+SetupBootMode\n"); */
    /* Initialize key pad.  */
    key_init();

#ifdef BSP_SUPPORT_AUTOSELECT	//<kuri0706> 自動判別対応
	AutoSelect();
#endif	//BSP_SUPPORT_AUTOSELECT <kuri0706> 自動判別対応

    /* Get boot key code.   */
    keycode = DetectBootKey();
    printf ("SetupBootMode: keycode = %x\n", keycode);
    if (keycode)
    {
        BuzzerInit();
        BuzzerBeep(BUZZER_BEEP_DURATION);
    }
#if 0	//<ver0.07> 起動キー以外がONしていたら(KEY_MASK_ANYKEYがON時)有効キーとしない

    if ((keycode & KEY_MASK_NUM5) &&
        (keycode & KEY_MASK_LOW)  &&
        (keycode & KEY_MASK_HIGHSPPED))
    {
        bootMode = BOOT_CFG_BOOTMODE_4;
    }
    else if ((keycode & KEY_MASK_NUM2) &&
            (keycode & KEY_MASK_NUM8)  &&
            (keycode & KEY_MASK_HIGHSPPED))
    {
        bootMode = BOOT_CFG_BOOTMODE_5;
    }
    else if ((keycode & KEY_MASK_NUM5) &&
            (keycode & KEY_MASK_LOW))
    {
        bootMode = BOOT_CFG_BOOTMODE_1;
    }
    else if ((keycode & KEY_MASK_NUM2) &&
            (keycode & KEY_MASK_NUM8))
    {
        bootMode = BOOT_CFG_BOOTMODE_2;
    }
    else if ((keycode & KEY_MASK_HIGHSPPED) &&
            (keycode & KEY_MASK_LOW))
    {
        bootMode = BOOT_CFG_BOOTMODE_3;
    }
    else if ((keycode & KEY_MASK_PERIOD) &&
            (keycode & KEY_MASK_HYPHEN))
    {
        bootMode = BOOT_CFG_BOOTMODE_6;
    }
    else if ((keycode & KEY_MASK_PERIOD) &&
            (keycode & KEY_MASK_HIGHSPPED))
    {
        bootMode = BOOT_CFG_BOOTMODE_7;
    }
    else
    {
        bootMode = BOOT_CFG_BOOTMODE_0;
    }
#else	//<ver0.07> 起動キー以外がONしていたら(KEY_MASK_ANYKEYがON時)有効キーとしない
    if(keycode & KEY_MASK_ANYKEY)
    {
        //無効キーが含まれるので同時押し無効：bootModeは0
        bootMode = BOOT_CFG_BOOTMODE_0;
        printf("Invalid Key exist.\n");
    }
    else
    {
        if ((keycode & KEY_MASK_NUM5) &&
            (keycode & KEY_MASK_LOW)  &&
            (keycode & KEY_MASK_HIGHSPPED))
        {
            bootMode = BOOT_CFG_BOOTMODE_4;
        }
        else if ((keycode & KEY_MASK_NUM2) &&
                (keycode & KEY_MASK_NUM8)  &&
                (keycode & KEY_MASK_HIGHSPPED))
        {
            bootMode = BOOT_CFG_BOOTMODE_5;
        }
        else if ((keycode & KEY_MASK_NUM5) &&
                (keycode & KEY_MASK_LOW))
        {
            bootMode = BOOT_CFG_BOOTMODE_1;
        }
        else if ((keycode & KEY_MASK_NUM2) &&
                (keycode & KEY_MASK_NUM8))
        {
            bootMode = BOOT_CFG_BOOTMODE_2;
        }
        else if ((keycode & KEY_MASK_HIGHSPPED) &&
                (keycode & KEY_MASK_LOW))
        {
            bootMode = BOOT_CFG_BOOTMODE_3;
        }
        else if ((keycode & KEY_MASK_PERIOD) &&
                (keycode & KEY_MASK_HYPHEN))
        {
            bootMode = BOOT_CFG_BOOTMODE_6;
        }
        else if ((keycode & KEY_MASK_PERIOD) &&
                (keycode & KEY_MASK_HIGHSPPED))
        {
            bootMode = BOOT_CFG_BOOTMODE_7;
        }

    #if 1	//<ver0.09>
        else if ((keycode & KEY_MASK_RSHIFT) &&
                (keycode & KEY_MASK_ENT))
        {
            bootMode = BOOT_CFG_BOOTMODE_8;
        }

    #endif	//<ver0.09>
        
        else
        {
            bootMode = BOOT_CFG_BOOTMODE_0;
        }
    }
    
#endif	//<ver0.07> 起動キー以外がONしていたら(KEY_MASK_ANYKEYがON時)有効キーとしない

    printf ("SetupBootMode: bootMode = %d\n", bootMode);
}

int osBoot_KEYIn_Mode(void)
{
    return bootMode;
}

static ulong timePast = 0;
static ulong timeInterval = 0;
static int ledCount = 0;
static int ledEnable = 0;

void TimerInit(ulong interval)
{
    timePast = get_timer(0);
    timeInterval = interval;
}

/* caling from drivers/mmc/mmc.c - mmc_bread()  */
void osBoot_LED_Blink(void)
{
    /* LED Binking  */
    ulong timeNow;

    if (ledEnable != 0)
    {
/*      printf ("LED: timeNow = %d, Interval = %d\n", timeNow, timeInterval); */
        timeNow = get_timer(timePast);
        if (timeNow > timeInterval)
        {
            timePast = get_timer(0);
            BlinkFourLEDs(ledCount);
            ledCount++;
            if (ledCount > 3)
            {
                ledCount = 0;
            }
        }
    }
}

int is_valid_name_nk(const char *filename)
{
//<kuri0926-> WARNING Modify   char* fname = filename;
    char* fname = (char *)filename;	//<kuri0926> WARNING Modify
    
    if ((0 == strncmp(fname, "NK.", 3)) || (0 == strncmp(fname, "nk.", 3)))
    {
        fname += 3;
        if ((0 == strncmp(fname, "BIN", 3)) || (0 == strncmp(fname, "bin", 3)))
        {
            printf ("INFO: os image found.\n");
            return 1;
        }
    }
    printf ("WARN: %s is not a name of os image.\n", filename);
    return 0;
}

/*
    NK.bin file format

    +----------------+
    |  'B' 0x42      |  +0     .bin file header is total 15 bytes.
    |                |
    | Sync bytes     |
    : (7 bytes)      :
    |                |
    +----------------+
    |                |  +7
    | Image address  |
    | (4 bytes)      |
    |                |
    +----------------+
    |                | +11
    | Image length   |
    | (4 bytes)      |
    |                |         Record header is total 12 bytes.
    +----------------+ - - - - - - - - +----------------+
    |                | +15             |                |  +0
    | Record #1      |                 | Record address |
    |                |                 | (4 bytes)      |
    +----------------+.                |                |
    |                | .               +----------------+
    | Record #2      |  .              |                |  +4
    |                |   .             | Record length  |
    +----------------+    .            | (4 bytes)      |
    :                :     .           |                |
    :                :      .          +----------------+
    +----------------+       .         |                |  +8
    |                |        .        | Record checksum|
    | Record #n      |         .       | (4 bytes)      |
    |                |          .      |                |
    +----------------+           .     +----------------+
                                  .    |                | +12
                                   .   :                :
                                    .  | Data filed     |
                                     . | (n bytes)      |
                                      .|                |
                                       +----------------+

*/
#define NKIMG_HEADER_LENGTH         15
#define NKIMG_RECORD_HEAD_LENGTH    12
#define NKIMG_HEADER_SYNC_BYTE      0x42


/* #define USE_FS_LAYER */
/*
 * Read bin format file from FAT filesystem and convert a flat image
 * on to RAM.
 */
#ifndef FASTLOAD	//<kuri0926>
int ReadBinImageToFlat(const char *devname, const char *devnum, const char *filename,
                        u32 ofs, u32 RomStart, u32 *pRsize)
{
    char    *pBuf;
    loff_t   FileSize;
    u32   Size;
    loff_t   ReadSize;
    loff_t   Offset = 0;
    char    HdrBuf[16] = {0};
    u32   ImageAddress;
    u32   ImageLength;
    u32   RecordAddress;
    u32   RecordLength;
    u32   RecordCheckSum;
    u32   OffsetAddress;
    char    *pDst;
    u32   CheckSum;
    u32   DataCount;
    u32   MaxFlatImageSize;
    u32   SkipLength;
    block_dev_desc_t *dev_desc = NULL;
    int   dev = 0;
    char  *ep;

    printf("Reading image... devname %s, devnum %s.\n", devname, devnum);

    pBuf = (char *)IMAGE_BOOT_NKIMAGE_RAM_PA_START + ofs;

    dev = (int)simple_strtoul (devnum, &ep, 16);
    dev_desc = get_dev(devname, dev);
    if (dev_desc == NULL) {
        printf("ERROR: Failed to get dev_desc of MMC.\n");
        return -1;
    }

    if (fat_size(filename, &FileSize) == -1)
    {
        printf("ERROR: Failed to get size of NK.bin.\n");
        return -1;
    }
    Size = (u32)FileSize;
    printf("INFO: Size of NK.bin is %d bytes.\n", Size);

    if (Size < (NKIMG_HEADER_LENGTH + NKIMG_RECORD_HEAD_LENGTH + 1))
    {
        printf("ERROR: NK.bin file is too small.\n");
        return -1;
    }

    if (file_fat_read(filename, HdrBuf, NKIMG_HEADER_LENGTH) == -1)
    {
        printf("ERROR: Failed to read NK.bin.\n");
        return -1;
    }

    Size -= NKIMG_HEADER_LENGTH;
    Offset += NKIMG_HEADER_LENGTH;

    if (HdrBuf[0] != 0x42)
    {
        printf("ERROR: Invalid bin format for NK.\n");
        return -1;
    }

    ImageAddress = ((u32)(HdrBuf[7]))          | (((u32)(HdrBuf[8])) << 8) |
                   (((u32)(HdrBuf[9])) << 16)  | (((u32)(HdrBuf[10])) << 24);
    ImageLength  = ((u32)(HdrBuf[11]))         | (((u32)(HdrBuf[12])) << 8) |
                   (((u32)(HdrBuf[13])) << 16) | (((u32)(HdrBuf[14])) << 24);
    MaxFlatImageSize = 0;

    printf("INFO: %s : Image address = 0x%x Image length = 0x%x\n",
            filename, ImageAddress, ImageLength);

    /*
     * Now read records of bin format.
     */
    while (Size >= NKIMG_RECORD_HEAD_LENGTH)
    {
        memset(HdrBuf, 0, sizeof(HdrBuf));
        if (fat_read_file(filename, HdrBuf, Offset, (loff_t)NKIMG_RECORD_HEAD_LENGTH, &ReadSize) == -1)
        {
            printf("ERROR: Failed to raed record header.\n");
            return -1;
        }
#if 0
        if (ReadSize != NKIMG_RECORD_HEAD_LENGTH)
        {
            printf("ERROR: Failed to raed record header. ReadSize = %llu bytes, HdrBuf = %X\n",
                    ReadSize, HdrBuf);
            return -1;
        }
#endif
        Size -= NKIMG_RECORD_HEAD_LENGTH;
        Offset += NKIMG_RECORD_HEAD_LENGTH;
        if (Size == 0)
        {
            break;
        }
        RecordAddress =  ((u32)(HdrBuf[0]))          | (((u32)(HdrBuf[1])) << 8) |
                         (((u32)(HdrBuf[2])) << 16)  | (((u32)(HdrBuf[3])) << 24);
        RecordLength =   ((u32)(HdrBuf[4]))          | (((u32)(HdrBuf[5])) << 8) |
                         (((u32)(HdrBuf[6])) << 16)  | (((u32)(HdrBuf[7])) << 24);
        RecordCheckSum = ((u32)(HdrBuf[8]))          | (((u32)(HdrBuf[9])) << 8) |
                         (((u32)(HdrBuf[10])) << 16)  | (((u32)(HdrBuf[11])) << 24);
        if (RecordLength > 0)
        {
            if (RecordLength > Size)
            {
                printf("\n");
                printf("ERROR: Invalid record length = %d bytes\n",
                    RecordLength);
                return -1;
            }

            if (RecordAddress < ImageAddress)
            {
                printf("\n");
                printf("ERROR: Invalid record address = 0x%x , Base address = 0x%x\n",
                    RecordAddress,ImageAddress);
                return -1;
            }
            OffsetAddress = RecordAddress - ImageAddress;

            if (OffsetAddress >= RomStart)
            {
                OffsetAddress = OffsetAddress - RomStart;

                if ((OffsetAddress >= IMAGE_BOOT_NKIMAGE_NOR_SIZE) ||
                    ((OffsetAddress + RecordLength) >= IMAGE_BOOT_NKIMAGE_NOR_SIZE))
                {
                    printf("\n");
                    printf("ERROR: Invalid record address offset = 0x%x length = 0x%x\n",OffsetAddress,RecordLength);
                    return -1;
                }

                pDst = pBuf + OffsetAddress;

                if (fat_read_file(filename, pDst, Offset, (loff_t)RecordLength, &ReadSize) == -1)
                {
                    printf("ERROR: Failed to raed record header.\n");
                    return -1;
                }
#if 0
                if (ReadSize != NKIMG_RECORD_HEAD_LENGTH)
                {
                    printf("ERROR: Failed to raed record header.\n");
                    return -1;
                }
#endif
                Size -= RecordLength;
                Offset += RecordLength;

                // Calculate checksum.
                CheckSum = 0;
                for (DataCount = 0; DataCount < RecordLength; DataCount++)
                {
                    CheckSum += (u32)pDst[DataCount];
                }

                if (RecordCheckSum != CheckSum)
                {
                    printf("\n");
                    printf("ERROR: Invalid checksum.\n");
                    printf("INFO: Address = 0x%x Length = 0x%x Sum = [ 0x%x , 0x%x ]\n",
                        RecordAddress,RecordLength,RecordCheckSum,CheckSum);
                    return -1;
                }

                if ((OffsetAddress + RecordLength) > MaxFlatImageSize)
                {
                    MaxFlatImageSize = (OffsetAddress + RecordLength);
                }

/*                printf("INFO: ADR = 0x%x OFF = 0x%x Len = 0x%x SUM = 0x%x\n",
                    RecordAddress,OffsetAddress,RecordLength,RecordCheckSum);
*/
#if 0
                printf("INFO:     %x %x %x %x\n",
                    *(pDst + 0),*(pDst + 1),*(pDst + 2),*(pDst + 3)   );
#endif
            }
            else
            {
                // Skip
                SkipLength = RecordLength;
                while (SkipLength > 0)
                {
                    if (SkipLength < sizeof(HdrBuf))
                    {
                        if (fat_read_file(filename, HdrBuf, Offset, (loff_t)SkipLength, &ReadSize) == -1)
                        {
                            printf("ERROR: Failed to raed record header.\n");
                            return -1;
                        }
#if 0
                        if (ReadSize != NKIMG_RECORD_HEAD_LENGTH)
                        {
                            printf("ERROR: Failed to raed record header.\n");
                            return -1;
                        }
#endif
                        Offset += SkipLength;
                        SkipLength = 0;
                    }
                    else
                    {
                        if (fat_read_file(filename, HdrBuf, Offset, (loff_t)sizeof(HdrBuf), &ReadSize) == -1)
                        {
                            printf("ERROR: Failed to raed record header.\n");
                            return -1;
                        }
                        SkipLength -= sizeof(HdrBuf);
                        Offset += sizeof(HdrBuf);
                    }
                }
                Size -= RecordLength;
            }
        }
        printf("\r");
        printf("INFO: Now deploying NK image ... remainder %d bytes, Offset %llu.", Size, Offset);
    }

#if 0
    printf("INFO: First 8 Bytes : %x %x %x %x %x %x %x %x\n",
        *(pBuf + 0),*(pBuf + 1),*(pBuf + 2),*(pBuf + 3),*(pBuf + 4),*(pBuf + 5),*(pBuf + 6),*(pBuf + 7));

    pBuf = pBuf + (MaxFlatImageSize - 8);
    printf("INFO: Last  8 Bytes : %x %x %x %x %x %x %x %x\n",
        *(pBuf + 0),*(pBuf + 1),*(pBuf + 2),*(pBuf + 3),*(pBuf + 4),*(pBuf + 5),*(pBuf + 6),*(pBuf + 7));
#endif

    if (Size != 0)
    {
        printf("ERROR: Last remain size = %d bytes.\n",Size);
        return -1;
    }

    *pRsize = MaxFlatImageSize;

    printf("INFO: .bin file to flat image OK.\n");

    return 1;
}
#endif	//FASTLOAD <kuri0926>

void osBoot_init(void)
{
#if 0
    printf ("osBoot_init: (mmc 0 more set)\n");
    mxc_iomux_v3_setup_multiple_pads(usdhc1_pads,
        sizeof(usdhc1_pads) /
        sizeof(usdhc1_pads[0]));

    //xmxc_iomux_v3_setup_pad(MX6_PAD_SD4_DAT0__GPIO_2_8);
    //gpio_direction_output(((2-1)*32)+8, 1);       // SD Power ON
#endif
}

#if 1	//<kuri0526>
//カード有無チェック
//=0でカード無し、=1でカード有
int	SDCardExistCheck(void)
{
	int ret = 0;
    ret = !gpio_get_value(USDHC1_CD_GPIO);

	printf("SDCardExistCheck:ret = %X\n",ret);

	return(ret);
}

#endif	//<kuri0526>



/* USB or MMC/SD memory FAT system / Read File                                  */
/*  ret code                                                                    */
/*    0: No error                                                               */
/*    1: Device not found                                                       */
/*    2: Invalid FAT system                                                     */
/*    3: File not found                                                         */
/*                                                                              */
int osBoot_fat_fsload(const char *devname, const char *devnum, unsigned char *Offset,
                        char *filename, unsigned int *filesize)
{
    u32 size;
    unsigned long count;
    block_dev_desc_t *dev_desc=NULL;
    int dev=0;
    int part=1;
    char *ep;
    int ret = 0;

    printf ("osBoot_fat_fsload: (%s)\n", filename);

    dev = (int)simple_strtoul (devnum, &ep, 16);
    dev_desc=get_dev(devname,dev);
    if (dev_desc==NULL) {
        puts ("\n** Invalid boot device **\n");
        return 1;
    }
    if (*ep) {
        if (*ep != ':') {
            puts ("\n** Invalid boot device, use `dev[:part]' **\n");
            return 1;
        }
        part = (int)simple_strtoul(++ep, NULL, 16);
    }

    for (part = 1; part < 4; part++)
    {
        ret = fat_register_device(dev_desc,part);
        if (ret == 0)
        {
            printf ("Use %s %d:%d for fatload.\n",devname,dev,part);
            break;
        }
        else if (ret == -2)
        {
            printf ("\n** Unable to use %s %d:%d for fatload **\n",devname,dev,part);
            return 2;
        }
    }
    if (ret != 0)
    {
        printf ("\n** Unable to use %s %d:%d for fatload **\n",devname,dev,part);
        return 2;
    }

    /* Store memory offset  */
    count = READ_FILE_MAX_SIZE;     /* Max Count        */
#ifndef FASTLOAD	//<kuri0926>
    if (is_valid_name_nk(filename))
    {
        /* Initialize LED.  */
        setup_led();
        if (ReadBinImageToFlat(devname, devnum, filename, 0, 0, &size) == -1)
        {
            printf("\n** Unable to read \"%s\" from %s %d:%d **\n", filename, devname, dev, part);
            /* Turn LEDs off.   */
            BlinkAllLEDs(0);
            return 3;
        }
        /* Turn LEDs off.   */
        BlinkAllLEDs(0);
    }
    else
    {
#endif	//FASTLOAD <kuri0926>
        size = file_fat_read(filename, Offset, count);
#ifndef FASTLOAD	//<kuri0926>
    }
#endif	//FASTLOAD <kuri0926>
    if(size==-1) {
        printf("\n** Unable to read \"%s\" from %s %d:%d **\n",filename,devname,dev,part);
        return 3;
    }

    printf ("\nINFO: %d bytes read\n", size);
    if (NULL != filesize)
    {
        *filesize = size;
    }

    printf("osBoot_fat_fsload: (read complete)\n");
    return 0;
}
#ifdef FASTLOAD	//<kuri0926+> ========================================================================================
//NK.BINのヘッダー。開始アドレスとサイズ。
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef BYTE * PBYTE;
typedef void * PVOID;
//typedef DWORD *PDWORD;

typedef	struct {
	DWORD	dwAddress;
	DWORD	dwSize;
} NkHdr, *NkHdrPtr;

//モジュールヘッダー。
//このヘッダー以降に、dwSize分のデータがいて、そこの全データのSUMが
//dwSumになる。
typedef	struct	{
	DWORD	dwAddress;
	DWORD	dwSize;
	DWORD	dwSum;
} MdHdr, *MdHdrPtr;

#if 0	//<kuri0926> 未使用なので削除
typedef	struct	{
	NkHdr	NkHeader;
	DWORD	dwLaunchAddress;
} NkInfo, *NkInfoPtr;
#endif	//<kuri0926>
#endif	//FASTLOAD<kuri0926+> ========================================================================================

#ifdef FASTLOAD	//<kuri0926+> =============================================================================================
// NK.BIN のロード用
// NK.BIN をメディアから読み出すときにレコード分析するのではなく、最初に一括でRAM上に読み出しておき、その後
// RAM->RAMへレコード分析しながら再配置する方式とする。（その方が読み出し速度が速いため）
DWORD	ataMakeSum(PVOID pStart, DWORD dwSize)
{
	DWORD	sum = 0;
	PBYTE	p = (PBYTE)pStart;

	while (dwSize--)
		sum += (DWORD)(*p++);

	return sum;
}

int osBoot_fat_fsload_bin(const char *devname, const char *devnum, unsigned char *Offset,
                        char *filename, unsigned int *filesize)
{
	long size;
	unsigned long count;
	//char buf [12];
	block_dev_desc_t *dev_desc=NULL;
	int dev=0;
	int part=1;
	char *ep;
	int ret = 0;
#if 1	//<kuri1201> ver0.08 size check
	loff_t chk_size = 0;
	loff_t *pchk_size;
	int	retval;
#endif	//<kuri1201> ver0.08 size check

//YPP-EBOOT
	DWORD		dwReadSize;
	BYTE		Header[7];
	NkHdr		NkHeader;
	MdHdr		ModuleHeader;
	PBYTE		pBuffer;
	PBYTE		pBuffer2;
	DWORD		sum;
	DWORD		dwTotalReadSize = 0;
	//DWORD		dwSize = 0;
	//MdHdrPtr	pHdr;
	//PDWORD		pdwReadSize;
	//PDWORD		pdwTotalReadSize;
	//NkInfo		Info;
	unsigned char *pCurrentAddr = (unsigned char *)IMAGE_BOOT_NKIMAGE_RAM_PA_STORE_START;
	//<tanaka20190902>
	bool			ram_init=false;

//YPP-EBOOT

	printf ("osBoot_fat_fsload_bin: (in)\n");	// @CG

	dev = (int)simple_strtoul (devnum, &ep, 16);
	dev_desc=get_dev(devname,dev);
	if (dev_desc==NULL) {
		puts ("\n** Invalid boot device **\n");
		return 1;
	}
	//printf ("osBoot_fat_fsload_bin: (1)\n");	// @CG
	if (*ep) {
		if (*ep != ':') {
			puts ("\n** Invalid boot device, use `dev[:part]' **\n");
			return 1;
		}
		part = (int)simple_strtoul(++ep, NULL, 16);
	}

	//printf ("osBoot_fat_fsload_bin: (2)\n");	// @CG
	for (part = 1; part < 4; part++)
	{
		ret = fat_register_device(dev_desc,part);
		if (ret == 0)
		{
			printf ("Use %s %d:%d for fatload.\n",devname,dev,part);
			break;
		}
	}
	if (ret != 0)
	{
		printf ("\n** Unable to use %s %d:%d for fatload **\n",devname,dev,part);
		return 2;
	}

	//printf ("osBoot_fat_fsload_bin: (3)\n");	// @CG

	//NK.NB0はすべて一発で読み出している
	// Store memory offset
	//<kuri1010>	count = READ_FILE_MAX_SIZE_112;	// Max Count:112MBとする
	count = READ_FILE_MAX_SIZE_120;	// Max Count:114MBとする <kuri1010>
	
	// Initialize LED.
	setup_led();
	TimerInit(100);
	
	//debug
	//printf("LOAD FILE : %s , Offset = %x , count = %x \n",filename , Offset , count);
	
#if 1	//<kuri1201> ver0.08 size check
	
	pchk_size = &chk_size;
	retval = fat_size(filename, pchk_size);	//サイズチェック関数CALL
	
	//関数戻り値チェック
	if(retval != 0)
	{
		printf("\n** Unable to read \"%s\"  **\n",filename);
	#if 1	//<kuri0531> 不具合修正で返り値は元に戻す
//<kuri0531>	#if 0	//<kuri0526>
		return 3;	//読み出しエラーで終了
	#else	//<kuri0526>
		return 4;	//ファイルなしエラーで終了（ステータス追加）
	#endif	//<kuri0526>
	}
	//ファイル長チェック
	if(chk_size > READ_FILE_MAX_SIZE_120)
	{
		printf("\n** File size error %d **\n",(unsigned int)chk_size);
		return(-1);	//サイズオーバーで終了
	}

#endif	//<kuri1201> ver0.08 size check

	size = file_fat_read(filename, Offset, count);

	BlinkAllLEDs(0);

	if(size==-1) {
		printf("\n** Unable to read \"%s\" from %s %d:%d **\n",filename,devname,dev,part);
		return 3;
	}

	printf ("\n%ld bytes read\n", size);
	
	//ここまでで一旦、RAM(0x1800_0000)上にNK.BINがそのままCOPYされているので、ここから
	//RAM上のNK.BINを解析して、正しいエリアに正しいバイナリで置きなおす。
	
	//printf ("osBoot_fat_fsload_bin: (4)\n");	// @CG
	
	//EBOOTのLoadATAの中でやっている処理をここで実行する。
	
	//ヘッダーチェック
	//  NK.BINの一時保存エリアからヘッダー情報を読み出し
	dwReadSize = sizeof(Header);
	memcpy(&Header[0] , pCurrentAddr , dwReadSize);

	dwTotalReadSize += dwReadSize;
	pCurrentAddr += dwReadSize;

	//printf("HEARDER = %x,%x,%x,%x,%x,%x,%x, dwTotalReadSize = %x , pCurrentAddr = %x \n"
	//	,Header[0],Header[1],Header[2],Header[3],Header[4],Header[5],Header[6],Header[7]
	//	,dwTotalReadSize
	//	,pCurrentAddr );

#ifdef	BSP_SUPPORT_VERCHK		//<kuri0428>
	//バージョンチェック情報を保管
	//（Headerは7byteのunsigned char配列なので、そのままsizeofを使用している。
	// Allow_Boot_Versionは8byteのunsigned char配列なので、各々のサイズ定義を変更したらここも変更必要。）
	// （Allow_Boot_VersionはDetectBootKey()関数内最後で0x00で初期化済み）
	memcpy ( Allow_Boot_Version , Header , sizeof(Header));
	printf ("OS allows boot version = %s.\n", Allow_Boot_Version);
#else	//BSP_SUPPORT_VERCHK <kuri0428>
	//  ヘッダー列をチェック
	if (memcmp(Header, "B000FF\n", sizeof(Header))) {
		printf("[LoadATA]IMAGE LOAD NG. (Header error)\r\n");
		return (-1);
	}
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

	//エントリーアドレスとトータルサイズ取得
	dwReadSize = sizeof(NkHdr);
	memcpy( &NkHeader , pCurrentAddr ,dwReadSize);
	
#if 1	//<kuri1014> 
	if(NkHeader.dwSize > IMAGE_BOOT_NKIMAGE_NOR_SIZE)
	{
		return(-1);	//サイズオーバーで終了
	}
#endif	//<kuri1014>

	dwTotalReadSize += dwReadSize;
	pCurrentAddr += dwReadSize;

	//printf("NkHeader.dwAddress = %x ,NkHeader.dwSize = %x , dwTotalReadSize = %x , pCurrentAddr = %x\n"
	//	, NkHeader.dwAddress , NkHeader.dwSize , dwTotalReadSize , pCurrentAddr);

	ledCount = 0;

	//ここからはモジュールヘッダー読み出しながらループ
	while(1)
	{
		//モジュールヘッダーを読み出し
		dwReadSize = sizeof(MdHdr);
		memcpy ( &ModuleHeader , pCurrentAddr , dwReadSize);
		dwTotalReadSize += dwReadSize;
		pCurrentAddr += dwReadSize;
		
		//LED制御
		BlinkFourLEDs(ledCount);
		ledCount++;
		if (ledCount > 3)
		{
			ledCount = 0;
		}
		
		//printf("ModuleHeader.dwAddress = %x , ModuleHeader.dwSize = %x , ModuleHeader.dwSum = %x\n"
		//	,ModuleHeader.dwAddress,ModuleHeader.dwSize,ModuleHeader.dwSum);
		//printf("pCurrentAddr = %x , dwTotalReadSize = %x\n"
		//	,pCurrentAddr , dwTotalReadSize);

		if (!ModuleHeader.dwAddress && !ModuleHeader.dwSum)
		{
			printf("Address or SUM is NULL ERROR!!\n");
			break;
		}
		else
		{
			//実データ読み出し
			//pBuffer = (PBYTE)(ModuleHeader.dwAddress + CACHED_TO_UNCACHED_OFFSET);
			pBuffer = (PBYTE)(ModuleHeader.dwAddress - 0x70000000);	//NK.BIN内のアドレスは0x8060_0000ベースだが、書き込み先は0x1060_0000なので0x7000_0000を引く
			pBuffer2 = pBuffer;
			//<tanaka20190902>
			if(!ram_init)
			{
				printf("NK.bin RAM initial start...\n");
				memset(pBuffer, 0xff, IMAGE_BOOT_NKIMAGE_NOR_SIZE);
				ram_init = true;
			}

			//BOOL LoadModule()関数
			{
				DWORD	dwReadReqSize;
				//DWORD	dwReadEndSize=0;
				//DWORD	dwSize = (*pHdr).dwSize;
				DWORD	dwSize = ModuleHeader.dwSize;

				//printf("pBuffer = %x , dwSize = %x\n", pBuffer , dwSize);

				dwReadSize = 0;

				while (dwSize) {
					//printf("Data read loop in.\n");
					
					dwReadReqSize = dwSize >= 0x40000 ? 0x40000 : dwSize;

					//printf("pBuffer = %x , pCurrentAddr = %x , dwSize = %x\n"
					//	, pBuffer , pCurrentAddr , dwSize);

					memcpy(pBuffer , pCurrentAddr , dwReadReqSize);

					pCurrentAddr += dwReadReqSize;

					dwTotalReadSize += dwReadReqSize;
					pBuffer += dwReadReqSize;
					dwReadSize += dwReadReqSize;
					dwSize -= dwReadReqSize;
					
					//printf("pCurrentAddr = %x , pBuffer = %x , pdwReadSize = %x\n"
					//	,pCurrentAddr , pBuffer , dwReadSize);
					
				}
				//printf("Data read loop out.\n");
			}
			//読み出しサイズの確認
			if (dwReadSize != ModuleHeader.dwSize) {
				return (-1);
			}

			//チェックサム確認
			// チェックサム確認
			if (ModuleHeader.dwSum != (sum=ataMakeSum(pBuffer2, ModuleHeader.dwSize))) {
				printf("\r\n[LoadATA] sumerr  %X : %X\r\n",
								(unsigned int)sum,
								(unsigned int)ModuleHeader.dwSum);
				printf("[LoadATA]OS IMAGE LOAD NG. (Sum error)\r\n");
			#ifdef	BSP_SUPPORT_SUMERR	//<kuri0512>
			    return (-3);		// チェックサムエラーコード追加
			#else   //BSP_SUPPORT_SUMERR  <kuri0512>
				return (-1);		// チェックサムエラー
			#endif	//BSP_SUPPORT_SUMERR  <kuri0512>
			}
		}
		//モジュールヘッダー読み出しに戻る
	}
	
#if 0	//<kuri0926> 未使用なので削除
	Info.NkHeader.dwAddress	= NkHeader.dwAddress;
	Info.NkHeader.dwSize	= NkHeader.dwSize;
	Info.dwLaunchAddress	= ModuleHeader.dwSize;
#endif	//<kuri0926>


	if (NULL != filesize)
	{
		*filesize = NkHeader.dwSize;
	}
	
	BlinkAllLEDs(0);

	printf ("osBoot_fat_fsload_bin: (read complete)\n");	// @CG
	return 0;
}

#endif	//FASTLOAD<kuri0926+> =============================================================================================


int load_eboot(void)
{
    unsigned char* pRreadOff = (unsigned char *)IMAGE_EBOOT_RAM_BASE_ADDR;
    unsigned char* pAddr = (unsigned char*)IMAGE_BOOT_EBOOTIMAGE_NOR_PA_START;
    int ret = 0;

    printf ("INFO: Boot from NOR Flash.\n");
    memset(pRreadOff, 0, IMAGE_BOOT_EBOOTIMAGE_NOR_SIZE);

    /* Load eboot.nb0 from NOR Flash.   */
    memcpy(pRreadOff, pAddr, IMAGE_BOOT_EBOOTIMAGE_NOR_SIZE);

    return ret;
}

int osBoot_Mode_0367(int mode)
{
    PBSP_BOOT_INFO  pBoootInfo;
    int     bootMedia = 0;                  /* 1: USB, 2: SD/MMC    */
    int     bootDevice = get_boot_device();
    int     errCode = ERROR_PP_BL_NO_ERROR;

    /* ------------------------------------ */
    /* Sets Boot inforation to common area. */
    /* ------------------------------------ */

    pBoootInfo = (PBSP_BOOT_INFO)(IMAGE_SHARE_ARGS_RA_CA_START + 
                                  IMAGE_SHARE_ARGS_RAM_SIZE - 
                                  sizeof(BSP_BOOT_INFO));   // common PARAM area.

    if (WEIM_NOR_BOOT == bootDevice)
    {
        pBoootInfo->bootDevice = BOOT_DEVICE_NOR;
    }
    else if (SD_BOOT == bootDevice)
    {
        pBoootInfo->bootDevice = BOOT_DEVICE_SD;
    }
    pBoootInfo->strage      = bootMedia;
    pBoootInfo->errorCode   = errCode;
    pBoootInfo->osImageSize = 0;

    if(mode == 0) {
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_0;
    } else if(mode == 3) {
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_3;
    } else if(mode == 6) {
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_6;
    } else { /* if(mode == 7)   */
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_7;
    }

    printf ("INFO: Mode = %d, Device = %d, Error = %d.\n",
            pBoootInfo->bootMode, pBoootInfo->bootDevice, pBoootInfo->errorCode);

#if 1	//<kuri1014>
	//PCBバージョン情報をCOPY
	pBoootInfo->boardVersion = Board_Version;
    printf ("INFO: Borad Version = %d.\n",pBoootInfo->boardVersion);
#endif	//<kuri1014>

#ifdef	BSP_SUPPORT_VERCHK	//<kuri0428>
	//NK.BINロード時の先頭情報(Bootバージョン比較用)をCOPY。
	//(NK.BINロードしていないときは0x00で初期化されている)
	memcpy(pBoootInfo->AllowBootVersion , Allow_Boot_Version , 8);
    printf ("INFO: Allow_Boot_Version = %s.\n",pBoootInfo->AllowBootVersion);
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

    return 0;
}

int osBoot_Mode_1245(int mode)
{
    int     bootMedia = 0;          /* 1: USB, 2: SD/MMC    */
    unsigned char   *pRreadOff;
    PBSP_BOOT_INFO  pBoootInfo;
    char    FileName[128];
    int     ret, i;
    int     bootDevice = get_boot_device();
    int     errCode = ERROR_PP_BL_NO_ERROR;
    unsigned int    FileSize;

    /* ------------------------- */
    /* USB or MMC/SD memory Boot */
    /* ------------------------- */
    printf ("osBoot_Mode_1245: Media check\n");

    /* Read text file ext. "2016INST.TXT"   */
    memset(FileName, 0, sizeof(FileName));

#if 1	//<kuri0926>
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_set_value(LED_GPIO3_19, 1);		//K_LED3 ON
    gpio_set_value(LED_GPIO3_20, 0);		//K_LED4 OFF
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_set_value(LED_GPIO3_30, 1);		//START LED ON
    gpio_set_value(LED_GPIO3_29, 0);		//HOLD LED OFF
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
#endif	//<kuri0926>

    TimerInit(250);
    
    printf ("osBoot_Mode_1245: Media search start.\n");

    /* Search on USB memory */
    while (1)
    {
        printf ("osBoot_Mode_1245: USB reset\n");

#if 0	//<kuri1012>
        /* Reset USB Host. */
        run_command("usb reset", 0);
#else   //<kuri1012> 新規に準備したUSBリセット専用コマンドを使用する
		go_usb_start();
#endif	//<kuri1012> 新規に準備したUSBリセット専用コマンドを使用する

        printf ("INFO: Search OS image on USB memory.\n");
        ret = osBoot_fat_fsload(STORAGE_DEV_NAME_USB,
                                STORAGE_DEV_NUM_USB,
                                (unsigned char *)FileName,
                                INST_TEXT_FILENAME, NULL);
        if(ret == 0)
        {
            for(i=0; i<sizeof(FileName); i++) {
                if(FileName[i]==0x00) {
                    break;
                }
                if( (FileName[i]==0x0d) || (FileName[i]==0x0a) ) {
                    FileName[i] = 0;    // NULL
                }
            }
#if 0	//<kuri1014> NK.BINファイル名チェックしない
            if (!is_valid_name_nk(FileName))
            {
                // Invalid file name found.
                errCode = ERROR_PP_BL_USB_NO_OSIMG;
                break;
            }
#endif	//<kuri1014> NK.BINファイル名チェックしない
            printf ("osBoot_Mode_1245: boot Image [%s]\n", FileName);

            /* Read OS boot image file To RAM and blinking LED  */
            /* Read Image file ext. "NK.nb0"                    */
            ledEnable = 1;
#ifdef FASTLOAD	//<kuri0926+>
            pRreadOff = (unsigned char *)IMAGE_BOOT_NKIMAGE_RAM_PA_STORE_START;		//読み出し先はNK.BINを一旦保存するエリア
            ret = osBoot_fat_fsload_bin(STORAGE_DEV_NAME_USB,
                                    STORAGE_DEV_NUM_USB,
                                    pRreadOff,
                                    FileName,
                                    &FileSize);
#else	//FASTLOAD<kuri0926+>
            pRreadOff = (unsigned char *)IMAGE_BOOT_NKIMAGE_RAM_PA_START;
            ret = osBoot_fat_fsload(STORAGE_DEV_NAME_USB,
                                    STORAGE_DEV_NUM_USB,
                                    pRreadOff,
                                    FileName,
                                    &FileSize);
#endif	//FASTLOAD<kuri0926+>
            ledEnable = 0;
            
            if(ret != ERROR_PP_BL_NO_ERROR) {
            #if 0	//<kuri0531> TXTありでBIN読み出せない場合はエラー確定とする
                    //<kuri0531> 上記ケースでSDなしのとき通常起動してしまう不具合の修正
        //<kuri0531>    #if 1	//<kuri0526>
                if(ret == 4)
                {
                    printf ("osBoot_Mode_1245(2): boot Image [%s] size error!\n", FileName);
                    if(SDCardExistCheck() != 0)
                    {
                        //SDカード有のときはSDカード側をチェックするので一旦抜ける
                        ret = 0;
                        errCode = ERROR_PP_BL_NO_ERROR;
                        bootMedia = 0;
                        break;
                    }
                    bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
                    
                    //SDカード無しのときそのまま通常起動に進む不具合の修正<kuri0531>
                    errCode = ERROR_PP_BL_USB_NO_OSIMG;     /* <kuri0531> No OS Image  */
                }
                else if(ret == 3) {
             #else	//<kuri0526>
                if(ret == 3) {
             #endif	//<kuri0526>
                    printf ("osBoot_Mode_1245(2): boot Image [%s] read error!\n", FileName);
                    errCode = ERROR_PP_BL_USB_NO_OSIMG;     /* No OS Image  */
                    
                #if 1	//<kuri1201> ver0.08
                    bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
                #endif	//<kuri1201> ver0.08
                    
                }
                
            #if 1	//<kuri1014>
            	//テキストファイルまでは発見できている状態で、今度はNK.bin読み込みでのエラー
                else if(ret == 1)	//USBデバイスがない、異常
                {
                    printf ("osBoot_Mode_1245(2): boot Image not found error!\n");
                    errCode = ERROR_PP_BL_USB_NO_OSIMG;     /* size or image invalid */
                    
                #if 1	//<kuri1201> ver0.08
                    bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
                #endif	//<kuri1201> ver0.08
                    
                }
                else if(ret == 2)	//ファイルシステム異常
                {
                    printf ("osBoot_Mode_1245(2): boot device filesystem invalid!\n");
                    errCode = ERROR_PP_BL_USB_INVALID_FS;     /* filesystem invalid */
                    
                #if 1	//<kuri1201> ver0.08
                    bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
                #endif	//<kuri1201> ver0.08
                    
                }
            #ifdef	BSP_SUPPORT_SUMERR    //<kuri0512>
                else if(ret == -3)
                {
                    printf ("osBoot_Mode_1245(-3): NK.BIN SUM ERROR !\n");
                    errCode = ERROR_PP_BL_NK_SUM_ERROR;     /* SUM ERROR */
                    
                    bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
 
                }
            #else   //BSP_SUPPORT_SUMERR  <kuri0512>
            #endif  //BSP_SUPPORT_SUMERR  <kuri0512>
                else				//(-1)含む：サイズ異常 or NK.BINフォーマット異常、他
                {
                    printf ("osBoot_Mode_1245(2): boot Image format or size error !\n");
                    errCode = ERROR_PP_BL_FLASH_ERROR;     /* size or image invalid */
                    
                #if 1	//<kuri1201> ver0.08
                    bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
                #endif	//<kuri1201> ver0.08
                    
                }
            #endif	//<kuri1014>
            
            }
            else
            {
                /* OS image load from USB memory.   */
                printf ("INFO: Found OS image on USB memory.\n");
                bootMedia = BOOT_STORAGE_USB;
                break;
            }
        }
        //2016INST.txt読み出しでエラー時
        else if (ret == 1)
        {
            /* USB memory is not exist. Go to SD card.  */
            ret = 0;
            break;
        }
        else if (ret == 2)
        {
            /* Found invalid file system on USB memory.   */
            printf ("osBoot_Mode_1245(1): boot device filesystem invalid!\n");	//<kuri1014>

        #if 1	//<kuri0526>
            if(SDCardExistCheck() != 0)
            {
                //SDカード有のときはSDカード側をチェックするので一旦抜ける
                ret = 0;
                errCode = ERROR_PP_BL_NO_ERROR;
                bootMedia = 0;
                break;
            }
        #endif	//<kuri0526>
        
            errCode = ERROR_PP_BL_USB_INVALID_FS;
            
        #if 1	//<kuri1201> ver0.08
            bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
        #endif	//<kuri1201> ver0.08
            
            break;
        }
        else if (ret == 3)
        {
            /* Reference text is not exist                */
            printf ("osBoot_Mode_1245(1): boot reference file not exist!\n");	//<kuri1014>

        #if 1	//<kuri0526>
            if(SDCardExistCheck() != 0)
            {
                //SDカード有のときはSDカード側をチェックするので一旦抜ける
                ret = 0;
                errCode = ERROR_PP_BL_NO_ERROR;
                bootMedia = 0;
                break;
            }
        #endif	//<kuri0526>

            errCode = ERROR_PP_BL_USB_NO_REFTEXT;
            
        #if 1	//<kuri1201> ver0.08
            bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
        #endif	//<kuri1201> ver0.08
            
            break;
        }
        else
        {
            printf ("osBoot_Mode_1245(1): Unknown status ret = %d.\n", ret);	//<kuri1014>
        #if 1	//<kuri0526>
            if(SDCardExistCheck() != 0)
            {
                //SDカード有のときはSDカード側をチェックするので一旦抜ける
                ret = 0;
                errCode = ERROR_PP_BL_NO_ERROR;
                bootMedia = 0;
                break;
            }
        #endif	//<kuri0526>
            
        #if 1	//<kuri1201> ver0.08
            bootMedia = BOOT_STORAGE_USB;	//起動デバイスUSBで確定
        #endif	//<kuri1201> ver0.08
            
        }
        break;
    }

#if 1	//<kuri0926>
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_set_value(LED_GPIO3_19, 0);		//K_LED3 OFF
    gpio_set_value(LED_GPIO3_20, 1);		//K_LED4 ON
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_set_value(LED_GPIO3_30, 0);		//START LED OFF
    gpio_set_value(LED_GPIO3_29, 1);		//HOLD LED ON
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
#endif	//<kuri0926>

    /* Search on SD card */
    if ((errCode == ERROR_PP_BL_NO_ERROR) && (bootMedia == 0))
    {

#if 1	//<kuri1201> ver0.08
        bootMedia = BOOT_STORAGE_SD;	//起動デバイスSDとする
#endif	//<kuri1201> ver0.08

        printf ("INFO: Search OS image on SD card.\n");
        ret = osBoot_fat_fsload(STORAGE_DEV_NAME_MMC,
                                STORAGE_DEV_NUM_MMC,
                                (unsigned char *)FileName,
                                INST_TEXT_FILENAME, NULL);
        if(ret == 0)
        {
            for(i=0; i<sizeof(FileName); i++) {
                if(FileName[i]==0x00) {
                    break;
                }
                if( (FileName[i]==0x0d) || (FileName[i]==0x0a) ) {
                    FileName[i] = 0;    // NULL
                }
            }
#if 0	//<kuri1014> NK.BINファイル名チェックしない
            if (is_valid_name_nk(FileName))
            {
#endif	//<kuri1014> NK.BINファイル名チェックしない
                printf ("osBoot_Mode_1245: boot Image [%s]\n", FileName);

                /* Read OS boot image file To RAM and blinking LED  */
                /* Read Image file ext. "NK.nb0"                    */
                ledEnable = 1;
#ifdef FASTLOAD	//<kuri0926+>
                pRreadOff = (unsigned char *)IMAGE_BOOT_NKIMAGE_RAM_PA_STORE_START;		//読み出し先はNK.BINを一旦保存するエリア
                ret = osBoot_fat_fsload_bin(STORAGE_DEV_NAME_MMC,
                                        STORAGE_DEV_NUM_MMC,
                                        pRreadOff,
                                        FileName,
                                        &FileSize);
#else	//FASTLOAD<kuri0926+>
                pRreadOff = (unsigned char *)IMAGE_BOOT_NKIMAGE_RAM_PA_START;
                ret = osBoot_fat_fsload(STORAGE_DEV_NAME_MMC,
                                        STORAGE_DEV_NUM_MMC,
                                        pRreadOff,
                                        FileName,
                                        &FileSize);
#endif	//FASTLOAD<kuri0926+>
                ledEnable = 0;
                if(ret != 0) {

            #if 0	//<kuri0531> 不具合修正で返り値は=3のみに戻す
        //<kuri0531> #if 1	//<kuri0526>
                    if((ret == 3)||(ret == 4)) {
            #else	//<kuri0526>
                    if(ret == 3) {
            #endif	//,kuri0526>
                        printf ("osBoot_Mode_1245(4): boot Image [%s] read error!\n", FileName); //<kuri1014>
                        errCode = ERROR_PP_BL_SD_NO_OSIMG;  /* No OS Image  */
                    }
                #if 1	//<kuri1014>
                    //テキストファイルまでは発見できている状態で、今度はNK.bin読み込みでのエラー
                    else if(ret == 1)	//SDデバイスがない、異常
                    {
                        printf ("osBoot_Mode_1245(4): boot Image not found error!\n");
                        errCode = ERROR_PP_BL_SD_NO_OSIMG;     /* size or image invalid */
                    }
                    else if(ret == 2)	//ファイルシステム異常
                    {
                        printf ("osBoot_Mode_1245(4): boot device filesystem invalid!\n");
                        errCode = ERROR_PP_BL_SD_INVALID_FS;     /* filesystem invalid */
                    }
                #ifdef	BSP_SUPPORT_SUMERR    //<kuri0512>
                    else if(ret == -3)
                    {
                        printf ("osBoot_Mode_1245(-3): NK.BIN SUM ERROR !\n");
                        errCode = ERROR_PP_BL_NK_SUM_ERROR;     /* SUM ERROR */
                    
                        //bootMedia はSDカードで確定済み
                    }
                #else   //BSP_SUPPORT_SUMERR  <kuri0512>
                #endif  //BSP_SUPPORT_SUMERR  <kuri0512>
                    else				//(-1)含む：サイズ異常 or NK.BINフォーマット異常、他
                    {
                        printf ("osBoot_Mode_1245(4): boot Image format or size error !\n");
                        errCode = ERROR_PP_BL_FLASH_ERROR;     /* size or image invalid */
                    }
                #endif	//<kuri1014>
                }
                else
                {
                    /* OS image load from SD card.  */
                    printf ("INFO: Found OS image on SD card.\n");
                    bootMedia = BOOT_STORAGE_SD;
                }
#if 0	//<kuri1014> NK.BINファイル名チェックしない
            }
            else
            {
                /* Invalid file name found. */
                errCode = ERROR_PP_BL_SD_NO_OSIMG;
            }
#endif	//<kuri1014> NK.BINファイル名チェックしない
        }
        else if (ret == 1)
        {
            /* SD card is not exist.    */
            printf ("osBoot_Mode_1245(3): SD card not exist!\n");				//<kuri1014>
            errCode = ERROR_PP_BL_NO_USB_SD;
        }
        else if (ret == 2)
        {
            /* Found invalid file system on SD card.    */
            printf ("osBoot_Mode_1245(3): boot device filesystem invalid!\n");	//<kuri1014>
            errCode = ERROR_PP_BL_SD_INVALID_FS;
        }
        else if (ret == 3)
        {
            /* Reference text is not exist. */
            printf ("osBoot_Mode_1245(3): boot reference file not exist!\n");	//<kuri1014>
            errCode = ERROR_PP_BL_SD_NO_REFTEXT;
        }
        else
        {
            printf ("osBoot_Mode_1245: Unknown status ret = %d.\n", ret);
        }

    }
    
#if 1	//<kuri0926>
#ifdef	BSP_SUPPORT_HIRATA	//<tanaka20200610>
    gpio_set_value(LED_GPIO3_19, 0);		//K_LED3 OFF
    gpio_set_value(LED_GPIO3_20, 0);		//K_LED4 OFF
#else	//BSP_SUPPORT_HIRATA <tanaka20200610>
    gpio_set_value(LED_GPIO3_30, 0);		//START LED OFF
    gpio_set_value(LED_GPIO3_29, 0);		//HOLD LED OFF
#endif	//BSP_SUPPORT_HIRATA <tanaka20200610>
#endif	//<kuri0926>


    /* ------------------------------------ */
    /* Sets Boot inforation to common area. */
    /* ------------------------------------ */
    pBoootInfo = (PBSP_BOOT_INFO)(IMAGE_SHARE_ARGS_RA_CA_START + 
                                  IMAGE_SHARE_ARGS_RAM_SIZE - 
                                  sizeof(BSP_BOOT_INFO));   /* common PARAM area.   */

    if (WEIM_NOR_BOOT == bootDevice)
    {
        pBoootInfo->bootDevice = BOOT_DEVICE_NOR;
    }
    else if (SD_BOOT == bootDevice)
    {
        pBoootInfo->bootDevice = BOOT_DEVICE_SD;
    }
    pBoootInfo->strage     = bootMedia;
    pBoootInfo->errorCode  = errCode;
    pBoootInfo->osImageSize = FileSize;

    if(mode == 1) {
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_1;
    } else if(mode == 2) {
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_2;
    } else if(mode == 4) {
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_4;
    } else { /* if(mode == 5)   */
        pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_5;
    }
    printf ("INFO: Mode = %d, Device = %d, Error = %d.\n",
            pBoootInfo->bootMode, pBoootInfo->bootDevice, pBoootInfo->errorCode);

	printf ("osBoot_Mode_1245: (osImageSize=%x)\n", pBoootInfo->osImageSize);	//<kuri0924+>
#if 1	//<kuri1014>
	//PCBバージョン情報をCOPY
	pBoootInfo->boardVersion = Board_Version;
    printf ("INFO: Borad Version = %d.\n",pBoootInfo->boardVersion);
#endif	//<kuri1014>

#ifdef	BSP_SUPPORT_VERCHK	//<kuri0428>
	//NK.BINロード時の先頭情報(Bootバージョン比較用)をCOPY。
	//(NK.BINロードしていないときは0x00で初期化されている)
	memcpy(pBoootInfo->AllowBootVersion , Allow_Boot_Version , 8);
    printf ("INFO: Allow_Boot_Version = %s.\n",pBoootInfo->AllowBootVersion);
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

    return 0;
}

#if 1	//<ver0.09>
//PCB自己診断用起動キー追加
int osBoot_Mode_8(int mode)
{
    int     bootMedia = 0;          /* 1: USB, 2: SD/MMC    */
//<kuri0421>    unsigned char   *pRreadOff;
    PBSP_BOOT_INFO  pBoootInfo;
    char    FileName[128];
//<kuri0421>    int     ret, i;
    int     ret;	//<kuri0421>
//<kuri0421>    int     bootDevice = get_boot_device();
    int     errCode = ERROR_PP_BL_NO_ERROR;
    unsigned int    FileSize = 0;

    /* ------------------------- */
    /* MMC/SD memory File Check  */
    /* ------------------------- */
    printf ("osBoot_Mode_8: Media check\n");

    /* Read text file ext. "2016INST.TXT"   */
    memset(FileName, 0, sizeof(FileName));

    TimerInit(250);
    
    printf ("osBoot_Mode_8: Media search start.\n");

    /* Search on SD card */
    if ((errCode == ERROR_PP_BL_NO_ERROR) && (bootMedia == 0))
    {

        //bootMedia = BOOT_STORAGE_SD;	//起動デバイスSDとする

        printf ("INFO: Search File:MNT.KEY on SD card.\n");
        ret = osBoot_fat_fsload(STORAGE_DEV_NAME_MMC,
                                STORAGE_DEV_NUM_MMC,
                                (unsigned char *)FileName,
                                TEST_TEXT_FILENAME, NULL);
		//ファイルあり
        if(ret == 0)
        {
			//MNT.KEYファイルあり：自己診断起動モード
			//pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_8;
            printf ("osBoot_Mode_8: OK.\n");
        }
        else
        {
			//MNT.KEYファイルあり：通常起動モード
            printf ("osBoot_Mode_8: Error ret = %d.\n", ret);
			//pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_0;
        }

    }
    
    /* ------------------------------------ */
    /* Sets Boot inforation to common area. */
    /* ------------------------------------ */
    pBoootInfo = (PBSP_BOOT_INFO)(IMAGE_SHARE_ARGS_RA_CA_START + 
                                  IMAGE_SHARE_ARGS_RAM_SIZE - 
                                  sizeof(BSP_BOOT_INFO));   /* common PARAM area.   */

    pBoootInfo->bootDevice = BOOT_DEVICE_NOR;
    pBoootInfo->strage     = bootMedia;
    pBoootInfo->errorCode  = errCode;
    pBoootInfo->osImageSize = FileSize;
    
    //ポインタの指す起動モードの値をセットする
    if(ret == 0)
    {
			pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_8;
	}
	else
	{
			pBoootInfo->bootMode = BOOT_CFG_BOOTMODE_0;
	}
    printf ("INFO: Mode = %d, Device = %d, Error = %d.\n",
            pBoootInfo->bootMode, pBoootInfo->bootDevice, pBoootInfo->errorCode);

	printf ("osBoot_Mode_8: (osImageSize=%x)\n", pBoootInfo->osImageSize);	//<kuri0924+>
#if 1	//<kuri1014>
	//PCBバージョン情報をCOPY
	pBoootInfo->boardVersion = Board_Version;
    printf ("INFO: Borad Version = %d.\n",pBoootInfo->boardVersion);
#endif	//<kuri1014>
#ifdef	BSP_SUPPORT_VERCHK	//<kuri0428>
	//NK.BINロード時の先頭情報(Bootバージョン比較用)をCOPY。
	//(NK.BINロードしていないときは0x00で初期化されている)
	memcpy(pBoootInfo->AllowBootVersion , Allow_Boot_Version , 8);
    printf ("INFO: Allow_Boot_Version = %s.\n",pBoootInfo->AllowBootVersion);
#endif	//BSP_SUPPORT_VERCHK <kuri0428>

    return 0;
}

#endif	//<ver0.09>

/* calling from common/main.c - main_loop() */
int os_boot_submain(void)
{
    int  bootMode;
    int  ret = 0;
    void (*img_addr)(void);

    osBoot_init();      /* Re-Init. MMC/SD IOMUX    */

    bootMode = osBoot_KEYIn_Mode();
    printf ("INFO: Boot Mode is %d\n", bootMode);
    switch (bootMode) {
    case 0:
        osBoot_Mode_0367(bootMode);
        break;
    case 1:
        osBoot_Mode_1245(bootMode);
        break;
    case 2:
        osBoot_Mode_1245(bootMode);
        break;
    case 3:
        osBoot_Mode_0367(bootMode);
        break;
    case 4:
        osBoot_Mode_1245(bootMode);
        break;
    case 5:
        osBoot_Mode_1245(bootMode);
        break;
    case 6:
        osBoot_Mode_0367(bootMode);
        break;
    case 7:
        osBoot_Mode_0367(bootMode);
        break;
#if 1	//<ver0.09>
    case 8:
        osBoot_Mode_8(bootMode);
        break;
#endif	//<ver0.09>
    default:
        printf("WARN: Unknown boot mode.");
        printf ("os_boot_submain: (exit)\n");
        return 1;
    }

    ret = load_eboot();
    if (ret != 0)
    {
        printf ("ERROR: Failed to load Eboot!\n");
        return ret;
    }

    /* ------------- */
    /* JUMP to Image */
    /* ------------- */
    img_addr = (void *)IMAGE_EBOOT_JUMP_ADDR;
    printf ("osBoot_Mode_0367: (Jump to image at 0x%08X)\n", (unsigned int)img_addr);

    /* Disable MMU, clean cache before launch Eboot. */
    cleanup_before_linux();

    img_addr();

    printf ("os_boot_submain: (exit)\n");

    return 0;
}

#endif /* IMX6SOLO_PP */
/* -Modified for iMX6Solo_PP at 2015/08/18 by Akita */

/* <tanaka20200114> */
void print_pmu_reg(void)
{
	struct anatop_regs *anatop = (struct anatop_regs *)ANATOP_BASE_ADDR;
	u32 reg;

	reg = readl(&anatop->reg_core);
	printf ("PMU_REG_CORE: 0x%8X\n", reg);
	reg = readl(&anatop->reg_3p0);
	printf ("PMU_REG_3P0: 0x%8X\n", reg);
}

