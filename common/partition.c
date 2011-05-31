/*
 * partition support
 * created by Ray
 * date: 11-05-24
 */

#include <common.h>
#include <command.h>
#include <partition.h>
#include <malloc.h>
#include <nand.h>


/*
 * Get mtd part table head infomation from nand flash 
 */
int mtdpart_get_head_info(part_head_t *parthead)
{
	struct mtd_info *nand = &nand_info[0];

	part_head_t *tmphead = NULL;
	char *tmpbuf = NULL;
	int ret = 1;

	uint partoff = CFG_PART_OFFSET;

	/* Malloc buffer for nand flash read/write */
	tmpbuf = malloc(nand_info[0].oobblock);
	if (tmpbuf == NULL)
        {
		printf("%s(%d) -- malloc buffer error\n", __FUNCTION__, __LINE__);
		return 1;
	}
	memset(tmpbuf, 0, nand_info[0].oobblock);

        if( nand_read( nand, partoff, &(nand_info[0].oobblock), tmpbuf) )
        {
            printf("%s(%d) -- read nand flash error\n", __FUNCTION__, __LINE__);
            ret = 1;
            goto EXIT;
        }

	tmphead = (part_head_t *)tmpbuf;
	if (tmphead->magic != MTD_PART_MAGIC) {
		printf("%s(%d) -- part table not exist\n", __FUNCTION__, __LINE__);
		ret = 2;
		goto EXIT;
	}
	ret = 0;
	/* Copy part table to user */
	if (parthead)
		memcpy((char *)parthead, (char *)tmphead, sizeof(part_head_t));

	free(tmpbuf);
	return 0;
EXIT:
	printf("%s(%d) -- get head info error\n", __FUNCTION__, __LINE__);
	if (tmpbuf)
		free(tmpbuf);
	return ret;
}

int mtdpart_get_mtd_info(part_head_t *parthead, part_entry_t *part_entry)
{

	struct mtd_info* nand = &nand_info[0];

	char *tmpbuf = NULL;
	int len = 0;

	uint partoff = CFG_PART_OFFSET;

	if (parthead->magic != MTD_PART_MAGIC) {
		printf("%s(%d) -- parthead error\n", __FUNCTION__, __LINE__);
		return 1;
	}

	len = ALIGN_SIZE(sizeof(part_entry_t)*MTD_MAX_NUMBER, nand_info[0].oobblock); 
	tmpbuf = malloc(len);
	if (tmpbuf == NULL)
        {
		printf("%s(%d) -- malloc buffer error\n", __FUNCTION__, __LINE__);
		return 1;
	}

	memset(tmpbuf, 0, len);

        if( nand_read(nand, partoff+parthead->parttab_offset, &len, tmpbuf) ) 
        {
		printf("%s(%d) -- read nand flash error\n", __FUNCTION__, __LINE__);
		free(tmpbuf);
		return 1;
	}
	/* Copy part info */
	if (part_entry)
		memcpy((char *)part_entry, (char *)tmpbuf, sizeof(part_entry_t)*parthead->part_num);

	free(tmpbuf);
	return 0;
}

int mtdpart_get_part_info(char *partname, int *size, int *offset)
{
	part_head_t parthead;

	part_entry_t *part_entry = NULL;
	part_entry_t *part_tmp = NULL;
	int ix = 0, found = 0;

	if (mtdpart_get_head_info(&parthead)) {
		printf("%s(%d) -- get mtdpart head info error\n", __FUNCTION__, __LINE__);
		return 1;
	}

	part_entry = (part_entry_t *)malloc(sizeof(part_entry_t)*parthead.part_num);
	if (mtdpart_get_mtd_info(&parthead, part_entry) != 0) {
		printf("%s(%d) -- Get mtdinfo error!\n", __FUNCTION__, __LINE__);
		free(part_entry);
		return 1;
	}

	part_tmp = part_entry;
	for (ix = 0; ix < parthead.part_num; ix ++) {
		if(!strcmp(part_tmp->partname, partname)) {
			*size = part_tmp->size;
			*offset = part_tmp->offset;
			found = 1;
			break;
		}
		part_tmp++;
	}

	if (!found) {
		printf("%s(%d) -- not found %s\n", __FUNCTION__, __LINE__, partname);
		free(part_entry);
		return 2;
	}
	free(part_entry);
	return 0;
}


int do_get_partinfo( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int len = 0, offset = 0;
    part_head_t parthead;
    part_entry_t *part_entry = NULL;
    int ix = 0;

    if (argc > 1)
    {
        if (mtdpart_get_part_info(argv[1], &len, &offset) != 0)
            printf("Get partition %s info fail\n", argv[1]);
        else
            printf("partition %s info: offset 0x%x, size %d Byte\n", argv[1], offset, len);
    }
    else
    {
        if (mtdpart_get_head_info(&parthead))
        {
            printf("%s(%d) -- get mtdpart head info error\n", __FUNCTION__, __LINE__);
            return 1;
        }

        part_entry = (part_entry_t *)malloc(sizeof(part_entry_t)*parthead.part_num);
        if (mtdpart_get_mtd_info(&parthead, part_entry) != 0)
        {
            printf("%s(%d) -- Get mtdinfo error!\n", __FUNCTION__, __LINE__);
            free(part_entry);
            return 1;
        }

        for (ix = 0; ix < parthead.part_num; ix ++)
        {
            printf("Part%d: offset 0x%08x size 0x%08x,  %s\n", 
                ix, part_entry[ix].offset, part_entry[ix].size, part_entry[ix].partname);
        }

	free(part_entry);

    }
	
    return 0;

}


U_BOOT_CMD(
	partinfo,	2,	0,	do_get_partinfo,
	"partinfo  - show partition info\n",
	"partition name\n"
);


int mtdpart_init_part_table(void)
{

	struct mtd_info* nand = &nand_info[0];


	part_head_t *parthead = NULL;
	dev_part_t *device_part = NULL;
	char *tmpbuf = NULL;
	int len = 0, ix = 0;
        size_t retlen = 0;

	uint partoff = CFG_PART_OFFSET;

//        printf("nand_erase: %d\nnand_oobblock: %d\npart_off: %d\n",\
//             nand->erasesize, nand->oobblock, partoff);


	/* Malloc buffer for nand flash read/write */
	parthead = (part_head_t *)malloc(nand_info[0].oobblock);
	if (parthead == NULL)	{
		printf("%s(%d) -- malloc buffer error\n", __FUNCTION__, __LINE__);
		return 1;
	}
	memset(parthead, 0, nand_info[0].oobblock);
	/* MTD part table exist ? exist return */
	if (mtdpart_get_head_info(parthead) == 0)
        {
		free(parthead);
		return 0;
	}

	/* Get device mtd part */
	device_part = (dev_part_t *)malloc(sizeof(dev_part_t));
	if (device_part == NULL)
	{
		printf("%s(%d) -- malloc buffer error\n", __FUNCTION__, __LINE__);
		free(parthead);
		return 1;
	}
	memset(device_part, 0, sizeof(dev_part_t));
	set_device_part(device_part);

	len = ALIGN_SIZE(sizeof(part_entry_t)*MTD_MAX_NUMBER, nand->oobblock); 
//        printf("**** len1 == %d ***\n", len);
	tmpbuf = malloc(len);
	if (tmpbuf == NULL)
        {
		printf("%s(%d) -- malloc buffer error\n", __FUNCTION__, __LINE__);
		free(parthead);
		free(device_part);
		return 1;
	}
	memset(tmpbuf, 0, len);	
	/* Setup part table */
	memcpy(tmpbuf, device_part->part_entry, sizeof(part_entry_t)*MTD_MAX_NUMBER);

	/* Setup part table head */
	parthead->magic = MTD_PART_MAGIC;
	parthead->parttab_offset = ix*nand->erasesize + nand->oobblock;
	parthead->part_num = device_part->partnum;
//	parthead->spexe_flag = device_part->spexe_flag;
//	parthead->spexe_offset = (ix+1)*nand->erasesize;

	if (nand_erase(nand, partoff+ix*nand->erasesize, nand->erasesize)) {
		printf("%s(%d) -- erase nand flash error\n", __FUNCTION__, __LINE__);
		goto EXIT;
	}

        if( nand->write_ecc( nand, partoff + parthead->parttab_offset, len, &retlen, tmpbuf, NULL, NULL ) )
        {
            printf("%s(%d) -- write nand flash error\n", __FUNCTION__, __LINE__);
            goto EXIT;
        }

        if( nand->write_ecc( nand, partoff, nand->oobblock, &retlen, (char *)parthead, NULL, NULL ) )
        {
            printf("%s(%d) -- write nand flash error\n", __FUNCTION__, __LINE__);
            goto EXIT;
        }


	free(parthead);
	free(device_part);
	free(tmpbuf);
	return 0;
EXIT:
	printf("Init partition table error!\n");
	free(parthead);
	free(device_part);
	free(tmpbuf);
	return 1;
}