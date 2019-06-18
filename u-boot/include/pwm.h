/*
 * header file for pwm driver.
 *
 * Copyright (c) 2011 samsung electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _pwm_h_
#define _pwm_h_

int pwm_init        (int pwm_id, int div, int invert);
int pwm_config      (int pwm_id, int duty_ns, int period_ns);
// +Modified for iMX6Solo_PP at 2015/08/20 by Akita
int pwm_config_direct(int pwm_id, u32 sample, u32 period);
// -Modified for iMX6Solo_PP at 2015/08/20 by Akita
int pwm_enable      (int pwm_id);
void    pwm_disable     (int pwm_id);

#endif /* _pwm_h_ */
