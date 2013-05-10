#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include "e2util.h"


// Switch all of the values in the superblock structure from ext2 little-endian
// to the host's byte order.
void byteswap_superblock(struct superblock *sb)
{
	SWAP_LE(sb->s_inodes_count, 32);
	SWAP_LE(sb->s_blocks_count, 32);
	SWAP_LE(sb->s_first_data_block, 32);
	SWAP_LE(sb->s_log_block_size, 32);
	SWAP_LE(sb->s_blocks_per_group, 32);
	SWAP_LE(sb->s_inodes_per_group, 32);
	SWAP_LE(sb->s_state, 16);
}

// Display a formatted output of the superblock parameters.
void print_superblock(struct superblock *sb)
{
	printf("Inodes: %u\n"
	       "Blocks: %u\n"
	       "First data block: %u\n"
	       "Block size: %u\n"
	       "Blocks/group: %u\n"
	       "Inodes/group: %u\n"
	       "State: %s\n",
			sb->s_inodes_count, sb->s_blocks_count,
			sb->s_first_data_block, blocksize(sb),
			sb->s_blocks_per_group, sb->s_inodes_per_group,
			sb->s_state == 1 ? "Clean" : "Dirty");
}

// Switch all of the values in the inode structure from ext2 little-endian to
// the host's byte order.
void byteswap_bgdesc(struct bgdesc *bg)
{
	SWAP_LE(bg->bg_block_bitmap, 32);
	SWAP_LE(bg->bg_inode_bitmap, 32);
	SWAP_LE(bg->bg_inode_table, 32);
}

// Switch all of the values in the inode structure from ext2 little-endian to
// the host's byte order.
void byteswap_inode(struct inode *i)
{
	int j;

	SWAP_LE(i->i_mode, 16);
	SWAP_LE(i->i_uid, 16);
	SWAP_LE(i->i_size, 32);
	SWAP_LE(i->i_atime, 32);
	SWAP_LE(i->i_ctime, 32);
	SWAP_LE(i->i_mtime, 32);
	SWAP_LE(i->i_dtime, 32);
	for (j = 0; j < 12; j++)
		SWAP_LE(i->i_block_d[j], 32);
	SWAP_LE(i->i_block_1i, 32);
	SWAP_LE(i->i_block_2i, 32);
	SWAP_LE(i->i_block_3i, 32);
}

// Display a formatted output of the inode parameters.
void print_inode(struct inode *i)
{
	time_t iatime = i->i_atime;
	time_t ictime = i->i_ctime;
	time_t imtime = i->i_mtime;
	time_t idtime = i->i_dtime;

	printf("Mode: %o\n"
	       "User ID: %u\n"
	       "Size: %u\n"
	       "Access time: %s"
	       "Change time: %s"
	       "Modification time: %s"
	       "Deletion time: %s"
	       "First direct block: %u\n",
			i->i_mode, i->i_uid, i->i_size,
			ctime(&iatime), ctime(&ictime),
			ctime(&imtime), ctime(&idtime),
			i->i_block_d[0]);
}

// Print out all the data in the file represented by a certain inode.
// Return 0 on success, 1 on error.
int print_inode_data(struct superblock *sb, struct inode *i)
{
	int fullblocks = i->i_size / blocksize(sb);
	int j;
	char *block;

	block = malloc(blocksize(sb));
	if (block == NULL)
		return 1;

	for (j = 0; j < fullblocks; j++) {
		if (get_inode_block(sb, i, j, block))
			return 1;
		if (fwrite(block, blocksize(sb), 1, stdout) != 1)
			return 1;
	}
	if (i->i_size % blocksize(sb)) {
		if (get_inode_block(sb, i, j, block))
			return 1;
		if (fwrite(block, i->i_size % blocksize(sb), 1, stdout) != 1)
			return 1;
	}

	free(block);
}

// Switch all of the values in an indirect block from ext2 little-endian to the
// host's byte order.
void byteswap_iblock(struct superblock *sb, char *block)
{
	int i;
	uint32_t *entry = (uint32_t *) block;
	for (i = 0; i < blocksize(sb) / 4; i++)
		SWAP_LE(entry[i], 32);
}

// Returns the block size of the filesystem
int blocksize(struct superblock *sb)
{
	return 1024 << sb->s_log_block_size;
}

// --- end provided code --- //


// Retrieve the interesting parts of the superblock and store it in the struct.
// Return 0 on success, 1 on error.
int get_superblock(FILE *f, struct superblock *out)
{
	// Save the file so other functions can use it
	out->file = f;
	
	fseek(f, 1024, SEEK_SET);
	
	uint32_t inodes_count;
	fread( &inodes_count, 4, 1, f);
	out->s_inodes_count = inodes_count;

	uint32_t blocks_count;
	fread( &blocks_count, 4, 1, f);
	out->s_blocks_count = blocks_count;
	
	fseek(f, 1108, SEEK_SET);
	
	uint32_t first_data_block;
	fread( &first_data_block, 4, 1, f);
	out->s_first_data_block = first_data_block;
	
	fseek(f, 1048, SEEK_SET);
	
	uint32_t block_size;
	fread( &block_size, 4, 1, f);
	out->s_log_block_size = block_size;
	
	fseek(f, 1056, SEEK_SET);
	
	uint32_t blocks_per_group;
	fread( &blocks_per_group, 4, 1, f);
	out->s_blocks_per_group = blocks_per_group;
	
	fseek(f, 1064, SEEK_SET);
	
	uint32_t inodes_per_group;
	fread( &inodes_per_group, 4, 1, f);
	out->s_inodes_per_group = inodes_per_group;
	
	fseek(f, 1082, SEEK_SET);
	
	uint16_t state;
	fread( &state, 2, 1, f);
	out->s_state = state;

	// Convert the superblock from little-endian format to whatever the
	// host system is.  Leave this at the end of get_superblock.
	byteswap_superblock(out);

	return 0;
}

// Fetch the data from the specified block into the provided buffer.
// Return 0 on success, 1 on error.
int get_block_data(struct superblock *sb, int blk, char *out)
{
	fseek(sb->file, blocksize(sb) * blk, SEEK_SET);
	return !(fread(out, blocksize(sb), 1, sb->file));
}

// Write the data from the specified block to standard output.
// Return 0 on success, 1 on error.
int print_block_data(struct superblock *sb, int blk)
{
	char *buffer = malloc(sizeof(char) * blocksize(sb));
	if(get_block_data(sb, blk, buffer) != 0)
	{
	  free(buffer);
	  return 1;
	}
	fwrite(buffer, blocksize(sb), 1, stdout);
	free(buffer);
	return 0;
}

// Return the number of the block group that a certain block belongs to.
int bg_from_blk(struct superblock *sb, int blk)
{
	return blk / sb->s_blocks_per_group;
}

// Return the index of a block within its block group.
int blk_within_bg(struct superblock *sb, int blk)
{
	return blk % sb->s_blocks_per_group;
}

// Return the number of the block group that a certain inode belongs to.
int bg_from_ino(struct superblock *sb, int ino)
{
	return (ino-1) / sb->s_inodes_per_group;
}

// Return the index of an inode within its block group
int ino_within_bg(struct superblock *sb, int ino)
{
	return (ino-1) % sb->s_inodes_per_group;
}

// Retrieve information from the block group descriptor table.
// Return 0 on success, 1 on error.
int get_bgdesc(struct superblock *sb, int bg, struct bgdesc *out)
{
	int offset;
	if(blocksize(sb) == 1024)
	  offset = 2048;
	else
	  offset = blocksize(sb);
	offset += 32*bg;

	fseek(sb->file, offset, SEEK_SET);

	uint32_t block_bitmap;
	fread( &block_bitmap, 4, 1, sb->file);
	out->bg_block_bitmap = block_bitmap;

	uint32_t inode_bitmap;
	fread( &inode_bitmap, 4, 1, sb->file);
	out->bg_inode_bitmap = inode_bitmap;

	uint32_t inode_table;
	fread( &inode_table, 4, 1, sb->file);
	out->bg_inode_table = inode_table;
	 
	// Convert the block info from little-endian format to whatever the
	// host system is.  Leave this at the end of get_bgdesc.
	byteswap_bgdesc(out);

	return 0;
}

// Retrieve information from an inode (file control block).
// Return 0 on success, 1 on error.
int get_inode(struct superblock *sb, int ino, struct inode *out)
{
	int bg;
	bg = bg_from_ino(sb, ino);
	
	int index = (ino - 1) % sb->s_inodes_per_group;
	
	struct bgdesc *blockdesc;
	get_bgdesc(sb, bg, blockdesc);
	
	int offset = blockdesc->bg_inode_table*blocksize(sb) + 128*index;
	fseek(sb->file, offset, SEEK_SET);
	
	uint16_t mode = 0;
	fread( &mode, 2, 1, sb->file);
	out->i_mode = mode;
		
	uint16_t uid;
	fread( &uid, 2, 1, sb->file);
	out->i_uid = uid;
	
	uint32_t size;
	fread( &size, 4, 1, sb->file);
	out->i_size = size;

	uint32_t atime;
	fread( &atime, 4, 1, sb->file);
	out->i_atime = atime;
	
	uint32_t ctime;
	fread( &ctime, 4, 1, sb->file);
	out->i_ctime = ctime;
	
	uint32_t mtime;
	fread( &mtime, 4, 1, sb->file);
	out->i_mtime = mtime;
	
	uint32_t dtime;
	fread( &dtime, 2, 1, sb->file);
	out->i_dtime = dtime;

	fseek(sb->file, offset+40, SEEK_SET);
	
	uint32_t block_d[12];
	int i;
	for(i=0; i<12; i++)
	{
	  fread( &block_d[i], 4, 1, sb->file);
	  out->i_block_d[i] = block_d[i];
	}
	
	uint32_t block_1i;
	fread( &block_1i, 4, 1, sb->file);
	out->i_block_1i = block_1i;

	uint32_t block_2i;
	fread( &block_2i, 4, 1, sb->file);
	out->i_block_2i = block_2i;

	uint32_t block_3i;
	fread( &block_3i, 4, 1, sb->file);
	out->i_block_3i = block_3i;

	// Convert the inode from little-endian format to whatever the host
	// system is.  Leave this at the end of get_inode.
	byteswap_inode(out);

	return 0;
}

// Retrieves the data from the nth data block of a certain inode.
// Return 0 on success, 1 on error.
int get_inode_block(struct superblock *sb, struct inode *i, uint32_t n, char *out)
{
	int directBlocks = 12;
	int indirectBlocks = blocksize(sb)/4;
	int doubleIndirectBlocks = (blocksize(sb)*blocksize(sb))/(4*4);
	int tripleIndirectBlocks = (blocksize(sb)*blocksize(sb)*blocksize(sb))/(4*4*4);
	if(n < directBlocks)
	{
	  get_block_data(sb, i->i_block_d[n], out);
	  return 0;
	}
	if(n < indirectBlocks + directBlocks)
	{
	  fseek(sb->file, (n-directBlocks)*4 + (i->i_block_1i*blocksize(sb)), SEEK_SET);
	  uint32_t addr;
	  fread( &addr, 4, 1, sb->file);
	  byteswap_iblock(sb, &addr);
	  get_block_data(sb, addr, out);
	  return 0;
	}
	if(n < doubleIndirectBlocks + indirectBlocks + directBlocks)
	{
	  int doubleIndirectOffset = (n-(indirectBlocks+directBlocks));
	  
	  //Seek to the correct position in the first indirect block
	  fseek(sb->file, doubleIndirectOffset/(blocksize(sb)/4) + (i->i_block_2i*blocksize(sb)), SEEK_SET);
	  uint32_t addr;
	  fread( &addr, 4, 1, sb->file);
	  byteswap_iblock(sb, &addr);
	  
	  //Seek to the correct position in the second indirect block -- which points to the final data block
	  fseek(sb->file, addr*blocksize(sb) + doubleIndirectOffset%(blocksize(sb)/4), SEEK_SET);
	  fread( &addr, 4, 1, sb->file);
	  byteswap_iblock(sb, &addr);
	  get_block_data(sb, addr, out);
	  return 0;
	}
	if(n < tripleIndirectBlocks + doubleIndirectBlocks + indirectBlocks + directBlocks)
	{
	  int tripleIndirectOffset = (n-(doubleIndirectBlocks+indirectBlocks+directBlocks));

	  //Seek to the correct position in the first indirect block
	  // Each 4-byte segment stores (blocksize^2)/16 block addresses
	  fseek(sb->file, tripleIndirectOffset/((blocksize(sb)*blocksize(sb))/16) + (i->i_block_3i*blocksize(sb)), SEEK_SET);
	  uint32_t addr;
	  fread( &addr, 4, 1, sb->file);
	  byteswap_iblock(sb, &addr);
	  
	  //Seek to the correct position in the second indirect block
	  int doubleIndirectOffset = tripleIndirectOffset%((blocksize(sb)*blocksize(sb))/16);
	  fseek(sb->file, doubleIndirectOffset + addr, SEEK_SET);
	  fread( &addr, 4, 1, sb->file);
	  byteswap_iblock(sb, &addr);
	  
	  //Seek to the correct position in the third indirect block -- which points to the final data block
	  fseek(sb->file, addr*blocksize(sb) + doubleIndirectOffset%(blocksize(sb)/4), SEEK_SET);
	  fread( &addr, 4, 1, sb->file);
	  byteswap_iblock(sb, &addr);
	  get_block_data(sb, addr, out);
	  return 0;
	}
	return 1;
}

// Return 1 if a block is free, 0 if it is not, and -1 on error
int is_block_free(struct superblock *sb, int blk)
{
	int bg = bg_from_blk(sb, blk);
	struct bgdesc *blockGroupDesc = malloc(sizeof(struct bgdesc));
	get_bgdesc(sb, bg, blockGroupDesc);

	//Offset of the block within the block group
	int bgOffset = blk - bg*sb->s_blocks_per_group;
	
	fseek(sb->file, (blockGroupDesc->bg_block_bitmap)*blocksize(sb) + (bgOffset/8), SEEK_SET);

	uint8_t tableByte;
	fread(&tableByte, 1, 1, sb->file);
	
	//Shift the read byte right to put the proper bit in the rightmost position
	tableByte >> (bgOffset % 8);
	
	free(blockGroupDesc);
	
	return tableByte & 1;
}

// Return 1 if a block appears to be an indirect block, 0 if it does not, and
// -1 on error.
int looks_indirect(struct superblock *sb, char *block)
{
	return 0;
}

// Return 1 if a block appears to be a doubly-indirect block, 0 if it does not,
// and -1 on error.
int looks_2indirect(struct superblock *sb, char *block)
{
	return 0;
}
