/*
 * partition support
 * created by Ray
 * date: 11-05-24
 */
#ifndef _PARTITION_H
#define _PARTITION_H




#define MTD_PART_MAGIC	0x20110524
#define MAX_NAME_LEN    32

/* Partition Max number */
#define MTD_MAX_NUMBER	0x10
#define ALIGN_SIZE(size, x)	((size + (x - 1)) & ~(x - 1))


typedef struct part_entry_struct {
	//unsigned int purpose;
	//unsigned int flags;
	unsigned int offset;
	unsigned int size;
	char partname[MAX_NAME_LEN];
} part_entry_t;


typedef struct dev_part_struct {
	int partnum;
//	int spexe_flag;
//	int backup_flag;
	part_entry_t part_entry[MTD_MAX_NUMBER];	
} dev_part_t;



/* Partition table head data structure */
typedef struct part_head_struct {
	unsigned int magic;
	unsigned int parttab_offset;
	unsigned int part_num;
} part_head_t;

#endif _PARTITION_H
