/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

#include <common.h>
#include <s3c2410.h>
#include <partition.h>


DECLARE_GLOBAL_DATA_PTR;

#define FCLK_SPEED 1

#define S3C2410_MPLL_200MHZ     ((0x5C<<12)|(0x04<<4)|(0x00))
#define S3C2410_UPLL_48MHZ      ((0x28<<12)|(0x01<<4)|(0x02))
#define S3C2410_CLKDIV          0x03

#if FCLK_SPEED==0		/* Fout = 203MHz, Fin = 12MHz for Audio */
#define M_MDIV	0xC3
#define M_PDIV	0x4
#define M_SDIV	0x1
#elif FCLK_SPEED==1		/* Fout = 202.8MHz */
#define M_MDIV	0xA1
#define M_PDIV	0x3
#define M_SDIV	0x1
#endif

#define USB_CLOCK 1

#if USB_CLOCK==0
#define U_M_MDIV	0xA1
#define U_M_PDIV	0x3
#define U_M_SDIV	0x1
#elif USB_CLOCK==1
#define U_M_MDIV	0x48
#define U_M_PDIV	0x3
#define U_M_SDIV	0x2
#endif

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init (void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

        clk_power->CLKDIVN = S3C2410_CLKDIV;

        /* change to asynchronous bus mod */
        __asm__(    "mrc    p15, 0, r1, c1, c0, 0\n"    /* read ctrl register   */  
                    "orr    r1, r1, #0xc0000000\n"      /* Asynchronous         */  
                    "mcr    p15, 0, r1, c1, c0, 0\n"    /* write ctrl register  */  
                    :::"r1"
                    );

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	clk_power->MPLLCON = S3C2410_MPLL_200MHZ;

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->UPLLCON = S3C2410_UPLL_48MHZ;

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
	gpio->GPACON = 0x007FFFFF;
	gpio->GPBCON = 0x00045555;
        gpio->GPBDAT &= (~(1<<6));
	gpio->GPBUP = 0x000007FF;
	gpio->GPCCON = 0xAAAAAAAA;
	gpio->GPCUP = 0x0000FFFF;
	gpio->GPDCON = 0xAAAAAAAA;
	gpio->GPDUP = 0x0000FFFF;
	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;
	gpio->GPFCON = 0x000055AA;
	gpio->GPFUP = 0x000000FF;
	gpio->GPGCON = 0xFF95FFBA;
	gpio->GPGUP = 0x0000FFFF;
	gpio->GPHCON = 0x002AFAAA;
	gpio->GPHUP = 0x000007FF;

	/* arch number of SMDK2410-Board */
	gd->bd->bi_arch_number = MACH_TYPE_SMDK2410;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}


void set_device_part(dev_part_t *device_part)
{
	strcpy(device_part->part_entry[0].partname, "u-boot");
	//device_part->part_entry[0].purpose = CFG_MTD_BOOT_PUPOSE;
	//device_part->part_entry[0].flags = CFG_MTD_BOOT_FLAG;
	device_part->part_entry[0].offset = 0;
	device_part->part_entry[0].size = 1*1024*1024;
	
	strcpy(device_part->part_entry[1].partname, "boot_para");
	//device_part->part_entry[1].purpose = CFG_MTD_PART_PUPOSE;
	//device_part->part_entry[1].flags = CFG_MTD_PART_FLAG;
	device_part->part_entry[1].offset = 0x100000;
	device_part->part_entry[1].size = 64*1024;

	strcpy(device_part->part_entry[2].partname, "part_table");
	//device_part->part_entry[2].purpose = CFG_MTD_IOS0_PUPOSE;
	//device_part->part_entry[2].flags = CFG_MTD_IOS0_FLAG;
	device_part->part_entry[2].offset = 0x110000;
	device_part->part_entry[2].size = 512*1024;	

	strcpy(device_part->part_entry[3].partname, "kernel");
	//device_part->part_entry[3].purpose = CFG_MTD_IOS1_PUPOSE;
	//device_part->part_entry[3].flags = CFG_MTD_IOS1_FLAG;
	device_part->part_entry[3].offset = 0x190000;
	device_part->part_entry[3].size = 5*1024*1024;

	strcpy(device_part->part_entry[4].partname, "rootfs");
	//device_part->part_entry[4].purpose = CFG_MTD_APP0_PUPOSE;
	//device_part->part_entry[4].flags = CFG_MTD_APP0_FLAG;
	device_part->part_entry[4].offset = 0x690000;
	device_part->part_entry[4].size = 64*1024*1024 \
            - device_part->part_entry[3].size \
            - device_part->part_entry[2].size \
            - device_part->part_entry[1].size \
            - device_part->part_entry[0].size;


	device_part->partnum = 5;
//	device_part->spexe_flag = 0;
//	device_part->backup_flag = MTD_BACKUP_FLAG;	
}

