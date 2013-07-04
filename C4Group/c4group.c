/*
 * c4group.c
 *
 *  Created on: 28.05.2012
 *      Author: dromedar
 *
 *  Note: This isn't a very memory efficient implementation. It would probably be better, if streams were used, but this makes things more complicated. Just don't use really large files...
 */

#include <c4group.h>

char * C4GROUP_ErrorMessage = NULL;

// Little endian byte sequence to unsigned 32 bit integer. Saves some weird type casting and pointer magic and makes the code (hopefully) more readable. Internal.
unsigned int C4GROUP_char2int32(char * ptr)
{
	return((unsigned char) *ptr + ((unsigned char) (*(ptr+1)) << 8) + ((unsigned char) (*(ptr+2)) << 16) + ((unsigned char) (*(ptr+3)) << 24));
}

// Computes the CRC32 checksum for C4File_t structures. Just a wrapper around the crc32 function from zlib. Internal.
unsigned int C4GROUP_getCRC32(char * name, char * content, unsigned int contentSize)
{
	unsigned long crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (const Bytef *) content, contentSize);
	crc = crc32(crc, (const Bytef *) name, strlen(name));
	return (int) crc;
}

// MemScrambles a buffer. Internal.
void C4GROUP_MemScramble(unsigned char * buf, unsigned int length)
{
	unsigned char tmp;
	unsigned char * ptr;
	for(ptr = buf; (ptr+2) < buf+length; ptr += 3)
	{
		tmp = *ptr; *ptr = *(ptr+2); *(ptr + 2) = tmp;
	}
	for(ptr = buf; ptr < buf+length; ptr++)
	{
		*ptr ^= 0xED;
	}
}

// This would be much easier with exceptions in C++... Internal.
static void C4GROUP_setError(const char * error)
{
	free(C4GROUP_ErrorMessage);
	C4GROUP_ErrorMessage = (char *) malloc(strlen(error));
	if(!C4GROUP_ErrorMessage)
	{
		fputs("Memory allocation failed!\n", stderr);
	}
	if(error)
		strcpy(C4GROUP_ErrorMessage, error);
}

// Nomen est omen. (Also documented)
const char * C4GROUP_getError()
{
	return(C4GROUP_ErrorMessage);
}

// See documentation.
C4GroupFile_t * C4GROUP_unpack(unsigned char * groupFileBuf, unsigned int buffsize, bool explode, bool compressed, const char *name)
{
	unsigned char * buffer; // Buffer for uncompressed stuff
	if(buffsize > 0xffffffff)
	{
		C4GROUP_setError("C4GROUP: Group file too large (>= 4GB). Unpacking failed.\n");
		return NULL;
	}
	if((groupFileBuf[0] == 0x89) && (groupFileBuf[1] == 0x88)) // Failsafety thingy
		compressed = false;
	if(compressed){
		// Magic byte magic!
		if(((groupFileBuf[0] == 0x1e) && ( groupFileBuf[1] == 0x8c))){
			groupFileBuf[0] = 0x1f;
			groupFileBuf[1] = 0x8b;
		}
		else{
			C4GROUP_setError("C4GROUP: Corrupted magic bytes. Unpacking failed.\n");
			return NULL;
		}
		// zlib stuff
		z_stream strm;
		int ret;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;
		int size = C4GROUP_char2int32((char *) groupFileBuf+buffsize-4);
		buffer = malloc(size);
		if(!buffer)
		{
			C4GROUP_setError("C4GROUP: Memory allocation for uncompressed buffer failed. Unpacking failed, too.\n");
			return NULL;
		}
		if(inflateInit2(&strm,15+16) != Z_OK){
			if(strm.msg){
				char * zlibError = (char*) malloc(strlen("C4GROUP: zlib: ") + strlen(strm.msg) - 1);
				if(!zlibError)
				{
					C4GROUP_setError("C4Group: Memory allocation for appropriate zlib error message failed.\n");
					free(buffer);
					return NULL;
				}
				strcat(zlibError, "C4GROUP: zlib: ");
				strcat(zlibError, strm.msg);
				C4GROUP_setError(zlibError);	// I hate concatenating strings in C
				free(zlibError);
			}
			else
				C4GROUP_setError("C4GROUP: zlib: Unknown error. Unpacking failed.\n");
			free(buffer);
			return NULL;
		}
		strm.avail_in = buffsize;
		strm.next_in = groupFileBuf;
		strm.avail_out = size;
		strm.next_out = buffer;
		ret = inflate(&strm, Z_FINISH); // This could be way more memory efficient, if streams were used, but that's probably just ridiculous for the most uses of the C4Group format...
		if((ret != Z_OK) && (ret != Z_STREAM_END))
		{
			if(strm.msg){
				char * zlibError = (char*) malloc(strlen("C4GROUP: zlib: ") + strlen(strm.msg) - 1);
				if(!zlibError)
				{
					C4GROUP_setError("C4Group: Memory allocation for appropriate zlib error message failed.\n");
					free(buffer);
					return NULL;
				}
				strcat(zlibError, "C4GROUP: zlib: ");
				strcat(zlibError, strm.msg);
				C4GROUP_setError(zlibError);
				free(zlibError);
			}
			else
				C4GROUP_setError("C4GROUP: zlib: Unknown error. Unpacking failed.\n");
			free(buffer);
			return NULL;
		}
		(void)inflateEnd(&strm);
		buffsize = size;
	}
	else{
		buffer = groupFileBuf;
	}
	if(buffsize < 204){
		C4GROUP_setError("C4GROUP: Group file too short (< 204 Bytes). Unpacking failed.\n");
		if(compressed) free(buffer);
		return NULL;
	}
	// MemScramble time!
	C4GROUP_MemScramble(buffer, 204);
	// Assembling C4Header and validating C4Group
	if(strncmp((char *) buffer, "RedWolf Design GrpFolder", 25)){
		C4GROUP_setError("C4GROUP: Header validation failed. File may be corrupted.");
		if(compressed) free(buffer);
		return NULL;
	}
	if((C4GROUP_char2int32((char *) buffer+28) != 1) || (C4GROUP_char2int32((char *) buffer+32) != 2)){
		C4GROUP_setError("C4GROUP: Version check failed. File may be corrupted.");
		if(compressed) free(buffer);
		return NULL;
	}
	C4GroupHeader_t head;
	memcpy(head.author, buffer+40, 32);
	head.creationTime = C4GROUP_char2int32((char *) buffer+104);
	if(C4GROUP_char2int32((char *) buffer+108) == 1234567)
		head.original = true;
	else
		head.original = false;
	head.fileCount = C4GROUP_char2int32((char *) buffer+36);
	head.groupCount = 0;
	memset(head.name, 0, 257);
	if(name)
		strcpy((char *) head.name, (char *) name);

	// Assembling C4Group
	C4GroupFile_t * result = (C4GroupFile_t*) malloc(sizeof(C4GroupFile_t));
	if(!result)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for new C4Group structure failed. Unpacking failed, too.\n");
		if(compressed) free(buffer);
		return NULL;
	}
	result->header = head;
	if(head.fileCount)
	{
		result->files = (C4File_t *) calloc(head.fileCount, sizeof(C4File_t));
		if(!result->files)
		{
			C4GROUP_setError("C4GROUP: Memory allocation for file list failed. Unpacking failed, too.\n");
			if(compressed) free(buffer);
			free(result);
			return NULL;
		}
		result->groupFiles = (C4GroupFile_t **) calloc(head.fileCount, sizeof(C4GroupFile_t*)); // The exact number of directories is not saved in the header.
		if(!result->groupFiles)
		{
			C4GROUP_setError("C4GROUP: Memory allocation for list of subgroups failed. Unpacking failed, too.\n");
			if(compressed) free(buffer);
			free(result->files);
			free(result);
			return NULL;
		}
		int dirCount = 0, j;
        for(unsigned int i=0; i < head.fileCount; i++)
		{
			j = i - dirCount; // i => total counter, j => counter for files only
			if(explode && ((*(buffer + 204 + 316*i + 264)) == 0x01)) // For direcotries
			{
				result->groupFiles[dirCount] = C4GROUP_unpack(buffer + 204 + 316*result->header.fileCount + (*((int *) (buffer + 204 + 316*i + 276))), *((int *)(buffer + 204 + 316*i + 268)), true, false, (char *) (buffer + 204 + 316*i));
				dirCount++;
			}
			else // For files
			{
				memcpy(result->files[j].name, buffer + 204 + 316*i, 257);
				memcpy(&(result->files[j].directory), buffer + 204 + 316*i + 264, 4);
				memcpy(&(result->files[j].size), buffer + 204 + 316*i + 268, 4);
				memcpy(&(result->files[j].lastModification), buffer + 204 + 316*i + 280, 4);
				memcpy(&(result->files[j].checksumType), buffer + 204 + 316*i + 284, 1);
				memcpy(&(result->files[j].checksum), buffer + 204 + 316*i + 285, 4);
				memcpy(&(result->files[j].executable), buffer + 204 + 316*i + 289, 1);
				result->files[j].content = (unsigned char *) malloc(result->files[j].size);
				if(!result->files[j].content){
					C4GROUP_setError("C4Group: Could not allocate enough memory for file content. Filesize will be set to 0. Other meta information is saved.");
					result->files[j].size = 0;
				}
				memcpy(result->files[j].content, buffer + 204 + 316*result->header.fileCount + (*((int *) (buffer + 204 + 316*i + 276))), result->files[j].size);
			}
		}
		if(dirCount){
			result->header.fileCount -= dirCount;
			result->header.groupCount = dirCount;
			// Emulating realloc behavior as realloc is weird and causes memory leaks (according to valgrind)...
			C4File_t * tFileArray = (C4File_t *) calloc(result->header.fileCount, sizeof(C4File_t));
			if(!tFileArray)
			{
				C4GROUP_setError("C4GROUP: Memory allocation for file array failed. Unpacking failed, too.\n");
				if(compressed) free(buffer);
				return NULL;
			}
			memcpy(tFileArray, result->files, sizeof(C4File_t) * result->header.fileCount);
			free(result->files);
			result->files = tFileArray;
		}

	}
	else
	{
		result->files = NULL;
		result->groupFiles = NULL;
	}
	if(compressed) free(buffer);
	return result;
}

// See documentation.
void C4GROUP_freeGroup(C4GroupFile_t * grpFile, bool keepSubgroups)
{
    for(unsigned int i=0; i<(grpFile->header.fileCount); i++)
	{
		free(grpFile->files[i].content);
	}
	free(grpFile->files);
	if(!keepSubgroups){
        for(unsigned int i=0; i<(grpFile->header.groupCount); i++)
		{
			C4GROUP_freeGroup(grpFile->groupFiles[i], false);
		}
	}
	free(grpFile->groupFiles);
	free(grpFile);
}

// See documentation
C4GroupFile_t * C4GROUP_newGroup()
{
	C4GroupFile_t * grp = (C4GroupFile_t *) malloc(sizeof(C4GroupFile_t));
	grp->files = NULL; // Init stuff
	grp->groupFiles = NULL;
	memset(grp->header.author, 0, 32);
	grp->header.creationTime = 0;
	grp->header.fileCount = 0;
	grp->header.groupCount = 0;
	memset(grp->header.name, 0, 257);
	grp->header.original = 0;
	return grp;
}

// See documentation
void C4GROUP_setHeaderProp(C4GroupFile_t * grp, char * prop, void * value)
{
	if(!strcmp(prop, "author"))
	{
		memset(grp->header.author, 0, 32); // Clean up. We don't want random bytes in our C4GroupFiles, do we?
		strncpy((char *) grp->header.author, (char *) value, 31);
		grp->header.author[31] = 0; // Make sure, the string is null terminated
	}
	else if(!strcmp(prop, "creationTime"))
	{
		grp->header.creationTime = *((int *) value);
	}
	else if(!strcmp(prop, "name"))
	{
		memset(grp->header.name, 0, 257);
		strncpy((char *) grp->header.name, (char *) value, 256);
		grp->header.name[256] = 0;
	}
	else if(!strcmp(prop, "original"))
	{
		grp->header.original = *((bool *) value);
	}
	else
	{
		C4GROUP_setError("C4GROUP: Invalid property given to C4GROUP_setHeaderProp.");
	}
}

// See documentation
void C4GROUP_addFile(C4GroupFile_t * grp, char * name, char * content, unsigned int fileSize)
{
	// Check if file already exists
    for(unsigned int i=0; i<grp->header.fileCount; i++)
	{
		if(!strcmp( (char *) grp->files[i].name,name))
		{
			C4GROUP_setError("C4GROUP: File already exists. Could not add a new file to this C4GroupFile.");
			return;
		}
	}
	// Expand file array by 1
	C4File_t * tFileArray = (C4File_t *) calloc(grp->header.fileCount + 1, sizeof(C4File_t));
	if(!tFileArray)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for new file array failed. Could not add a new file to this C4GroupFile.\n");
		return;
	}
	memcpy(tFileArray, grp->files, sizeof(C4File_t) * grp->header.fileCount);
	free(grp->files);
	grp->files = tFileArray;
	grp->header.fileCount++;
	// Init file
	memset(grp->files[grp->header.fileCount-1].name, 0, 257);
	grp->files[grp->header.fileCount-1].directory = false;
	//grp->files[grp->header.fileCount-1].size = 0;
	grp->files[grp->header.fileCount-1].lastModification = 0;
	grp->files[grp->header.fileCount-1].checksumType = 0;
	grp->files[grp->header.fileCount-1].checksum = 0;
	grp->files[grp->header.fileCount-1].executable = 0;
	grp->files[grp->header.fileCount-1].content = NULL;
	// Add new data
	strncpy((char *) grp->files[grp->header.fileCount-1].name, name, 257);
	grp->files[grp->header.fileCount-1].size =  fileSize;
	grp->files[grp->header.fileCount-1].content = (unsigned char *) malloc(fileSize);
	if(!grp->files[grp->header.fileCount-1].content)
	{
		C4GROUP_setError("C4Group: Could not allocate enough memory for file content. Filesize will be set to 0.");
		grp->files[grp->header.fileCount-1].size = 0;
	}
	else
	{
		memcpy((char *)grp->files[grp->header.fileCount-1].content, content, fileSize);
	}
	grp->files[grp->header.fileCount-1].checksum = C4GROUP_getCRC32(name, content, fileSize);
	grp->files[grp->header.fileCount-1].checksumType = C4GROUP_CONTENT_AND_NAME_CHECKSUM;
}

// See documentation
void C4GROUP_setFileProp(C4GroupFile_t * grp, char * name, char * prop, void * value)
{
	// Find file with <name>
	long int found = -1;
    for(unsigned int i=0; i<grp->header.fileCount; i++)
	{
		if(!strcmp((char *) grp->files[i].name, name))
			found = i;
	}
	if(found < 0)
	{
		C4GROUP_setError("C4GROUP: File with this name does not exist.");
		return;
	}
	// Set prop
	if(!strcmp(prop, "name"))
	{
		memset(grp->files[found].name, 0, 257);
		strncpy((char *) grp->files[found].name, (char *) value, 256);
		grp->files[found].name[256] = 0;
	}
	else if(!strcmp(prop, "directory"))
	{
		grp->files[found].directory = *((bool *) value);
	}
	else if(!strcmp(prop, "lastModification"))
	{
		grp->files[found].lastModification = *((unsigned int *) value);
	}
	else if(!strcmp(prop, "executable"))
	{
		grp->files[found].executable = *((bool *) value);
	}
	else
	{
		C4GROUP_setError("C4GROUP: Invalid property given to C4GROUP_setFileProp.");
	}
}

// See documentation
void C4GROUP_setFileContent(C4GroupFile_t * grp, char * name, char * content, unsigned int fileSize)
{
	// Find file
	long int found = -1;
    for(unsigned int i=0; i<grp->header.fileCount; i++)
	{
		if(!strcmp((char *) grp->files[i].name, name))
			found = i;
	}
	if(found < 0)
	{
		C4GROUP_setError("C4GROUP: File with this name does not exist.");
		return;
	}
	// Reset content
	free(grp->files[found].content);
	grp->files[found].content = (unsigned char *) malloc(fileSize);
	memcpy(grp->files[found].content, content, fileSize);
	grp->files[found].size = fileSize;
	// Recalc checksum
	grp->files[found].checksum = C4GROUP_getCRC32(name, content, fileSize);
	grp->files[found].checksumType = C4GROUP_CONTENT_AND_NAME_CHECKSUM;
}

// See documentation
void C4GROUP_removeFile(C4GroupFile_t * grp, char * name)
{
	// Find file
	long int found = -1;
    for(unsigned int i=0; i<grp->header.fileCount; i++)
	{
		if(!strcmp((char *) grp->files[i].name, name))
			found = i;
	}
	if(found < 0)
	{
		C4GROUP_setError("C4GROUP: File with this name does not exist.");
		return;
	}
	// Remove file
	C4File_t * tFileArray = (C4File_t *) calloc(grp->header.fileCount - 1, sizeof(C4File_t));
	if(!tFileArray)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for new file array failed. Could not remove file from this C4GroupFile.\n");
		return;
	}
	free(grp->files[found].content);
	int j = 0;
    for(unsigned int i=0; i<grp->header.fileCount; i++)
	{
		if(i != found)
		{
			memcpy(tFileArray + j * sizeof(C4File_t), grp->files + i * sizeof(C4File_t), sizeof(C4File_t));
			j++;
		}
	}
	free(grp->files);
	grp->files = tFileArray;
	grp->header.fileCount--;
}

// See documentation
void C4GROUP_addGroup(C4GroupFile_t * grp, C4GroupFile_t * grp2)
{
	C4GroupFile_t ** tGrpArray = (C4GroupFile_t **) calloc(grp->header.groupCount + 1 ,sizeof(C4GroupFile_t *));
	if(!tGrpArray)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for new group array failed. Could not add group to this C4GroupFile.\n");
		return;
	}
	memcpy(tGrpArray, grp->groupFiles, sizeof(C4GroupFile_t *) * grp->header.groupCount);
	free(grp->groupFiles);
	grp->groupFiles = tGrpArray;
	grp->header.groupCount++;
	grp->groupFiles[grp->header.groupCount - 1] = grp2;
}

// See documentation
void C4GROUP_removeGroup(C4GroupFile_t * grp, C4GroupFile_t * grp2)
{
	// Find group
	long int found = -1;
    for(unsigned int i=0; i<grp->header.groupCount; i++)
	{
		if(grp->groupFiles[i] == grp2)
			found = i;
	}
	if(found < 0)
	{
		C4GROUP_setError("C4GROUP: Group not found.");
		return;
	}
	// Remove group
	C4GroupFile_t ** tGrpArray = (C4GroupFile_t **) calloc(grp->header.groupCount - 1, sizeof(C4GroupFile_t *));
	if(!tGrpArray)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for new group array failed. Could not remove group from this C4GroupFile.\n");
		return;
	}
	int j = 0;
    for(unsigned int i=0; i<grp->header.groupCount; i++)
	{
		if(i != found)
		{
			tGrpArray[j] = grp->groupFiles[i];
			j++;
		}
	}
	free(grp->groupFiles);
	grp->groupFiles = tGrpArray;
	grp->header.groupCount--;
}

// See documentation
unsigned char * C4GROUP_pack(C4GroupFile_t * grp, unsigned int * size, bool compress)
{
	// Security
    for(unsigned int i=0; i<grp->header.groupCount; i++)
	{
		if(!grp->groupFiles[i]->header.name[0])
		{
			C4GROUP_setError("C4GROUP: Missing name in subgroup. Packing failed.");
			return NULL;
		}
	}
	// Packing
	unsigned int uncSize = 204 + 316 * (grp->header.fileCount + grp->header.groupCount);
	unsigned char * packedGrps[grp->header.groupCount];
	unsigned int packedGrpsSizes[grp->header.groupCount];
    for(unsigned int i=0; i<grp->header.groupCount; i++)
	{
		packedGrps[i] = C4GROUP_pack(grp->groupFiles[i], &(packedGrpsSizes[i]), false); // Weee, recursion!
		uncSize += packedGrpsSizes[i];
	}
    for(unsigned int i=0; i<grp->header.fileCount; i++)
	{
		uncSize += grp->files[i].size;
	}
	unsigned char * uncResult = malloc(uncSize); // Uncompressed buffer
	if(!uncResult)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for uncompressed buffer failed. Packing failed, too.\n");
        for(unsigned int i=0; i<grp->header.groupCount; i++)
			free(packedGrps[i]);
		free(uncResult);
		return NULL;
	}
	memset(uncResult, 0, uncSize);
	// C4Group-Header
	memcpy(uncResult, "RedWolf Design GrpFolder", 25);
	memset(uncResult + 28, 1, 1);
	memset(uncResult + 32, 2, 1);
	unsigned int count = grp->header.fileCount + grp->header.groupCount;
	memcpy(uncResult + 36, &count, 4);
	memcpy(uncResult + 40, grp->header.author, 32);
	memcpy(uncResult + 104, &grp->header.creationTime, 4);
	if(grp->header.original)
		memcpy(uncResult + 108, "\x87\xd6\x12\x00", 4); // 1234567 = '\x87\xd6\x12\x00'
	C4GROUP_MemScramble(uncResult, 204);
	// Table of contents and contents
	unsigned int contentOffset = 0;
    for(unsigned int i=0; i<grp->header.groupCount; i++) // Groups first
	{
		memcpy(uncResult + 204 + i*316, grp->groupFiles[i]->header.name, 257);
		memset(uncResult + 204 + i*316 + 260, 42, 1);
		memset(uncResult + 204 + i*316 + 264, 1, 1);
		memcpy(uncResult + 204 + i*316 + 268, &(packedGrpsSizes[i]), 4);
		memcpy(uncResult + 204 + i*316 + 276, &contentOffset, 4);
		memcpy(uncResult + 204 + i*316 + 280, &grp->groupFiles[i]->header.creationTime, 4); // Not really the last modification, but who reads this information anyway?
		memset(uncResult + 204 + i*316 + 284, C4GROUP_CONTENT_AND_NAME_CHECKSUM, 1);
		int crc = C4GROUP_getCRC32((char *) grp->groupFiles[i]->header.name, (char *) packedGrps[i], packedGrpsSizes[i]);
		memcpy(uncResult + 204 + i*316 + 285, &crc, 4);
		memcpy(uncResult + 204 + 316 * count + contentOffset, packedGrps[i], packedGrpsSizes[i]);
		contentOffset += packedGrpsSizes[i];
	}
    for(unsigned int i=0; i<grp->header.fileCount; i++) // Files second
	{
		int j = grp->header.groupCount + i;
		memcpy(uncResult + 204 + j*316, grp->files[i].name, 257);
		memset(uncResult + 204 + j*316 + 260, 42, 1);
		memcpy(uncResult + 204 + j*316 + 268, &grp->files[i].size, 4);
		memcpy(uncResult + 204 + j*316 + 276, &contentOffset, 4);
		memcpy(uncResult + 204 + j*316 + 280, &grp->files[i].lastModification, 4);
		memset(uncResult + 204 + j*316 + 284, C4GROUP_CONTENT_AND_NAME_CHECKSUM, 1);
		int crc = C4GROUP_getCRC32((char *) grp->files[i].name, (char *) grp->files[i].content, grp->files[i].size);
		memcpy(uncResult + 204 + j*316 + 285, &crc, 4);
		memset(uncResult + 204 + j*316 + 289, grp->files[i].executable, 1);
		memcpy(uncResult + 204 + 316 * count + contentOffset, grp->files[i].content, grp->files[i].size);
		contentOffset += grp->files[i].size;
	}
	// Clean up
    for(unsigned int i=0; i<grp->header.groupCount; i++)
		free(packedGrps[i]);
	// Return
	if(!compress)
	{
		*size = uncSize;
		return uncResult;
	}
	// Compression (optional)
	z_stream strm;
	int ret;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	if(deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		if(strm.msg){
			char * zlibError = (char*) malloc(strlen("C4GROUP: zlib: ") + strlen(strm.msg) - 1);
			if(!zlibError)
			{
				C4GROUP_setError("C4Group: Memory allocation for appropriate zlib error message failed.\n");
				free(uncResult);
				return NULL;
			}
			strcat(zlibError, "C4GROUP: zlib: ");
			strcat(zlibError, strm.msg);
			C4GROUP_setError(zlibError);
			free(zlibError);
		}
		else
			C4GROUP_setError("C4GROUP: zlib: Unknown error. Unpacking failed.\n");
		free(uncResult);
		return NULL;
	}
	unsigned long estCompSize = deflateBound(&strm, uncSize);
	unsigned char * buffer = malloc(estCompSize);
	if(!buffer)
	{
		C4GROUP_setError("C4GROUP: Memory allocation for compressed buffer failed. Packing failed, too.\n");
		free(uncResult);
		return NULL;
	}
	strm.avail_in = uncSize;
	strm.next_in = uncResult;
	strm.avail_out = estCompSize;
	strm.next_out = buffer;
	ret = deflate(&strm, Z_FINISH);
	if((ret != Z_OK) && (ret != Z_STREAM_END))
	{
		if(strm.msg){
			char * zlibError = (char*) malloc(strlen("C4GROUP: zlib: ") + strlen(strm.msg) - 1);
			if(!zlibError)
			{
				C4GROUP_setError("C4Group: Memory allocation for appropriate zlib error message failed.\n");
				free(uncResult);
				free(buffer);
				return NULL;
			}
			strcat(zlibError, "C4GROUP: zlib: ");
			strcat(zlibError, strm.msg);
			C4GROUP_setError(zlibError);
			free(zlibError);
		}
		else
			C4GROUP_setError("C4GROUP: zlib: Unknown error. Unpacking failed.\n");
		free(uncResult);
		free(buffer);
		return NULL;
	}
	buffer = realloc(buffer, estCompSize - strm.avail_out);
	*size = estCompSize - strm.avail_out;
	(void)deflateEnd(&strm);
	// Clean up and return
	free(uncResult);
	return buffer;
}

C4GroupFile_t * C4GROUP_cloneGroup(C4GroupFile_t * grp)
{
	C4GroupFile_t * res = C4GROUP_newGroup();
	// Copy header
	memcpy(&(res->header), &(grp->header), sizeof(C4GroupHeader_t));
	// Copy files
	res->files = calloc(res->header.fileCount, sizeof(C4File_t));
    for(unsigned int i=0; i<res->header.fileCount; i++)
	{
		memcpy(&(res->files[i]), &(grp->files[i]), sizeof(C4File_t));
		res->files[i].content = malloc(res->files[i].size);
		memcpy(res->files[i].content, grp->files[i].content, grp->files[i].size);
	}
	// Copy groups
	res->groupFiles = calloc(res->header.groupCount, sizeof(C4GroupFile_t *));
    for(unsigned int i=0; i<res->header.groupCount; i++)
	{
		res->groupFiles[i] = C4GROUP_cloneGroup(grp->groupFiles[i]);
	}
	return res;
}
