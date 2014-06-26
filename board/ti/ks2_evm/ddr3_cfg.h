/*
 * Copyright (C) 2014 Texas Instruments
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef __DDR3_CFG_H
#define __DDR3_CFG_H

extern struct ddr3_phy_config ddr3phy_1600_8g;
extern struct ddr3_emif_config ddr3_1600_8g;

#ifdef CONFIG_SOC_K2HK
extern struct ddr3_phy_config ddr3phy_1333_2g;
extern struct ddr3_emif_config ddr3_1333_2g;
#endif

#ifdef CONFIG_SOC_K2E
extern struct ddr3_phy_config ddr3phy_1600_4g;
extern struct ddr3_emif_config ddr3_1600_4g;
#endif

#ifdef CONFIG_SOC_K2L
extern struct ddr3_phy_config ddr3phy_1600_2g;
extern struct ddr3_emif_config ddr3_1600_2g;
#endif

#endif /* __DDR3_CFG_H */
