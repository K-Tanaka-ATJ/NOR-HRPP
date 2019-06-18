/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Basic support for the pwm modul on imx6.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <pwm.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include "pwm-imx-util.h"

// +Modified for iMX6Solo_PP at 2015/08/20 by Akita
#ifdef IMX6SOLO_PP
#define MX_PWMCR_CLKSRC_IPG_HIGH (2 << 16)

#define MX_PWMCR_STOPEN     (1 << 25)
#define MX_PWMCR_DOZEEN     (1 << 24)
#define MX_PWMCR_WAITEN     (1 << 23)
#define MX_PWMCR_DBGEN      (1 << 22)

#define PWMPRESCALER_VAL                    3
#define PWM_PWMCR_CLKSRC_IPG_CLK            (1 << 16)
#define PWM_CLKSRC_VAL                      PWM_PWMCR_CLKSRC_IPG_CLK_32K
#define PWM_CLKSRC_VAL_66M                  PWM_PWMCR_CLKSRC_IPG_CLK        // for BUZZER PWM2
#endif
// -Modified for iMX6Solo_PP at 2015/08/20 by Akita

int pwm_init(int pwm_id, int div, int invert)
{
    struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);

    if (!pwm)
        return -1;

    writel(0, &pwm->ir);
    return 0;
}

int pwm_config(int pwm_id, int duty_ns, int period_ns)
{
    struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);
    unsigned long period_cycles, duty_cycles, prescale;
    u32 cr;

    if (!pwm)
        return -1;

    pwm_imx_get_parms(period_ns, duty_ns, &period_cycles, &duty_cycles,
              &prescale);

    cr = PWMCR_PRESCALER(prescale) |
        PWMCR_DOZEEN | PWMCR_WAITEN |
        PWMCR_DBGEN | PWMCR_CLKSRC_IPG_HIGH;

    writel(cr, &pwm->cr);
    /* set duty cycles */
    writel(duty_cycles, &pwm->sar);
    /* set period cycles */
    writel(period_cycles, &pwm->pr);
    return 0;
}

// +Modified for iMX6Solo_PP at 2015/08/20 by Akita
#ifdef IMX6SOLO_PP
int pwm_config_direct(int pwm_id, u32 sample, u32 period)
{
    struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);
    u32 cr;

    if (!pwm)
        return -1;

    writel(sample, &pwm->sar);
    writel(period, &pwm->pr);

    cr = PWMCR_PRESCALER(PWMPRESCALER_VAL) |
         MX_PWMCR_STOPEN | MX_PWMCR_DOZEEN |
         MX_PWMCR_WAITEN | MX_PWMCR_DBGEN;

    cr |= MX_PWMCR_CLKSRC_IPG_HIGH;

    writel(cr, &pwm->cr);

    return 0;
}
#endif
// -Modified for iMX6Solo_PP at 2015/08/20 by Akita

int pwm_enable(int pwm_id)
{
    struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);

    if (!pwm)
        return -1;

    setbits_le32(&pwm->cr, PWMCR_EN);
    return 0;
}

void pwm_disable(int pwm_id)
{
    struct pwm_regs *pwm = (struct pwm_regs *)pwm_id_to_reg(pwm_id);

    if (!pwm)
        return;

    clrbits_le32(&pwm->cr, PWMCR_EN);
}
