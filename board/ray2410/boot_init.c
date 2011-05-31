#include <common.h>
#include <s3c2410.h>

#define BUSY            1

void ray_turn_on_led(void)
{
// #define rGPBCON			(*(volatile unsigned *)0x56000010)
// #define rGPBDAT			(*(volatile unsigned *)0x56000014)
// #define rGPBUP			(*(volatile unsigned *)0x56000018)
    S3C24X0_GPIO *s3c2410_gpio = (S3C24X0_GPIO *)0x56000000;

    s3c2410_gpio->GPBCON &= ~(3<<12);
    s3c2410_gpio->GPBCON |= (1<<12);
    s3c2410_gpio->GPBDAT |= (1<<6);
}

static unsigned char s3c2410_read_data(void)
{
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

    volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFDATA;
    return *p;
}

static void s3c2410_nand_select_chip(void)
{
    int i;
    S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

    s3c2410nand->NFCONF &= ~(1<<11);
    for(i=0; i<10; i++);    
}


static void s3c2410_write_cmd(int cmd)
{
    S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

    volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFCMD;
    *p = cmd;
}

static void s3c2410_wait_idle(void)
{
    int i;
    S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;
	
    volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFSTAT;
    while(!(*p & BUSY))
        for(i=0; i<10; i++);
}

static void s3c2410_nand_deselect_chip(void)
{
    S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

    s3c2410nand->NFCONF |= (1<<11);
}

static void s3c2410_nand_reset(void)
{
    s3c2410_nand_select_chip();
    s3c2410_write_cmd(0xff);  // 复位命令
    s3c2410_wait_idle();
    s3c2410_nand_deselect_chip();
}

static void s3c2410_write_addr(unsigned int addr)
{
    int i;
    S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;
    volatile unsigned char *p = (volatile unsigned char *)&s3c2410nand->NFADDR;
    
    *p = addr & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 9) & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 17) & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 25) & 0xff;
    for(i=0; i<10; i++);
}

static void nand_reset(void)
{
    s3c2410_nand_reset();
}

static void nand_select_chip(void)
{
    int i;

    s3c2410_nand_select_chip();
	
    for(i=0; i<10; i++);
}


static void write_cmd(int cmd)
{
    s3c2410_write_cmd(cmd);
}

static void write_addr(unsigned int addr)
{
    s3c2410_write_addr(addr);
}

static void wait_idle(void)
{
    s3c2410_wait_idle();
}

static unsigned char read_data(void)
{
    return s3c2410_read_data();
}

static void nand_deselect_chip(void)
{
    s3c2410_nand_deselect_chip();
}

void nand_init_ll(void)
{
	S3C2410_NAND * s3c2410nand = (S3C2410_NAND *)0x4e000000;

#define TACLS   0
#define TWRPH0  3
#define TWRPH1  0

        /* 使能NAND Flash控制器, 初始化ECC, 禁止片选, 设置时序 */
        s3c2410nand->NFCONF = (1<<15)|(1<<12)|(1<<11)|(TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0);

	/* 复位NAND Flash */
	nand_reset();
}


#define NAND_SECTOR_SIZE    512
#define NAND_BLOCK_MASK     (NAND_SECTOR_SIZE - 1)

void nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;
    
    if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) {
        return ;    /* 地址或长度不对齐 */
    }

    /* 选中芯片 */
    nand_select_chip();

    for(i=start_addr; i < (start_addr + size);) {
      /* 发出READ0命令 */
      write_cmd(0);

      /* Write Address */
      write_addr(i);
      wait_idle();

      for(j=0; j < NAND_SECTOR_SIZE; j++, i++) {
          *buf = read_data();
          buf++;
      }
    }

    /* 取消片选信号 */
    nand_deselect_chip();
    
    return ;
}


int CopyCode2Ram(unsigned long start_addr, unsigned char *buf, int size)
{
    ray_turn_on_led();
    /* 初始化NAND Flash */
    nand_init_ll();
    /* 从 NAND Flash启动 */
    nand_read_ll(buf, start_addr, (size + NAND_BLOCK_MASK)&~(NAND_BLOCK_MASK));

    return 0;

}