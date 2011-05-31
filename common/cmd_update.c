/*
 * update command support
 * created by Ray
 * date: 11-05-24
 */


int do_boot_update( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

}


int do_kernel_update( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

}


int do_fs_update( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

}


U_BOOT_CMD(
	bootupdate,	2,	0,	do_boot_update,
	"bootupdate  - update u-boot image\n",
	"filename\n"
);

U_BOOT_CMD(
	kernelupdate,	2,	0,	do_kernel_update,
	"kernelupdate  - update linux kernel uImage\n",
	"filename\n"
);

U_BOOT_CMD(
	fsupdate,	2,	0,	do_fs_update,
	"fsupdate  - update root file system\n",
	"filename\n"
);