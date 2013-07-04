/*
	File: c4group.h

	Header to access the functionality of this C4Group implementation.
*/

#ifndef C4GROUP_H_
#define C4GROUP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#ifdef _WIN32
	#define ZLIB_WINAP
#endif
#include <zlib.h>


/* -- Constants -- */


/*
	Constant: C4GROUP_NO_CHECKSUM

	Indicates that no checksum of a C4File is available. Uncommon.
*/
#define C4GROUP_NO_CHECKSUM 0
/*
	Constant: C4GROUP_CONTENT_ONLY_CHECKSUM

	Indicates that the checksum of a C4File is computed only with the content of the file. Old.
*/
#define C4GROUP_CONTENT_ONLY_CHECKSUM 1
/*
	Constant: C4GROUP_CONTENT_AND_NAME_CHECKSUM

	Indicates that the checksum of a C4File is computer with the content and the name of the file. Recommended.
*/
#define C4GROUP_CONTENT_AND_NAME_CHECKSUM 2


/* -- Data types -- */


/*
	Type: C4GroupFile_t

	This is the representation of a C4GroupFile. It contains a header, files and (optionally) other C4GroupFiles.

	Members:
		>C4GroupHeader_t header
		The <C4GroupHeader_t> containing the meta information of the C4GroupFile.
		>C4File_t * files;
		An array of <C4File_t> in this C4GroupFile. The amount of files is saved as a meta information in the header.
		>C4GroupFile_t ** groupFiles
		An array of C4GroupFile_t in this C4GroupFile. The amount of C4GroupFiles is saved as a meta information in the header. This member is normally only set in recursively unpacked C4GroupFiles.
*/
/*
	Type: C4GroupHeader_t

	This type is a structure that represents the meta information of a C4GroupFile. The members of this structure are read-only and should only be manipulated via <C4GROUP_setHeaderProp>. You should not use this type directly, but in combination with <C4GroupFile_t>.

	Members:
		>unsigned char author[32]
		The author (=> last editor) of the C4GroupFile.
		>unsigned int creationTime
		The time of creation in seconds since 1.1.1970. (Probably unsigned)
		>bool original
		True, if an original C4GroupFile from RedWolfDesign, or false, if not.
		>unsigned int fileCount
		The amount of files in this C4GroupFile.
		>unsigned int groupCount
		The amount of subdirectories in this C4GroupFile. This is usually only set in recursively unpacked C4GroupFiles.
		>unsigned char name[257]
		(Optional for primary groups, but mandatory for subgroups) The name of the C4GroupFile. This name is not saved within the C4GroupHeader and is therefore optional for top-level C4Groups. However C4Groups in other C4Groups need this field in the packing process.

*/
/*
	Type: C4File_t

	This is the representation of a file in a C4GroupFile. It contains meta data and the content of the file. You should not use this type directly, but in combination with <C4GroupFile_t>.

	Members:
		>unsigned char name[257]
		Filename.
		>bool directory
		True if directory, false if file.
		>unsigned int size
		Filesize.
		>unsigned int lastModification
		Date of last modification in seconds since 1.1.1970. (Probably unsigned)
		>unsigned char checksumType
		Indicates the method of checksum was computed with. Must be one of:
		- C4GROUP_NO_CHECKSUM
		- C4GROUP_CONTENT_ONLY_CHECKSUM
		- C4GROUP_CONTENT_AND_NAME_CHECKSUM
		>unsigned int checksum
		CRC32 checksum. The method of computation is described in the checksumType filed.
		>unsigned char executable
		True if executable, false if not. (For file systems with an executable bit)
		>unsigned char * content
		The content of the file.
*/

typedef struct C4GroupHeader_s{
    char author[32];
	unsigned int creationTime;
	bool original;
	unsigned int fileCount;
	unsigned int groupCount;
    char name[257];
} C4GroupHeader_t;

typedef struct C4File_s{
    char name[257];
	bool directory;
	unsigned int size;
	unsigned int lastModification;
	unsigned char checksumType;
	unsigned int checksum;
	bool executable;
	unsigned char * content;
} C4File_t;

typedef struct C4GroupFile_s{
	C4GroupHeader_t header;
	C4File_t * files;
	struct C4GroupFile_s ** groupFiles;
} C4GroupFile_t;


/* -- Functions -- */

/*
	Function: C4GROUP_addFile

	Add a file entry to a C4GroupFile_t structure. The file is identified by its name (null terminated). For convenience the content of the file (not necessary null terminated) can be set, too. If a file with this name already exists, the operation will fail.

	Parameters:
		grp - A pointer to the <C4GroupFile_t> struct, in which the file should be added.
		name - The name of the new file.
		content - The content of the new file.
		fileSize - The size of the file.

	See Also:
 		- <C4GROUP_removeFile> to remove files.
*/
void C4GROUP_addFile(C4GroupFile_t * grp, char * name, char * content, unsigned int fileSize);

/*
	Function: C4GROUP_addGroup

	Adds a <C4GroupFile_t> structure to another C4Group.

	Parameters:
		grp - The group, to which the other group will be added.
		grp2 - The group that will be added.

	See Also:
 		- <C4GROUP_removeGroup> to remove groups.
*/
void C4GROUP_addGroup(C4GroupFile_t * grp, C4GroupFile_t * grp2);

/*
	Function: C4GROUP_cloneGroup

	Clones a <C4GroupFile_t> structure.

	Parameters:
		grp - The group to clone.

	Returns:
		A newly allocated clone of the group.
*/
C4GroupFile_t * C4GROUP_cloneGroup(C4GroupFile_t * grp);

/*
	Function: C4GROUP_freeGroup

 	Deallocates the memory occupied by a <C4GroupFile_t> structure.

 	Parameters:
 		grpFile - A pointer to the C4GroupFile_t structure that should be freed.
 		keepSubgroups - If true, C4Groups inside grpFile are not freed. This can easily lead to memory leaks, so use with care.

 	See Also:
 		- <C4GROUP_newGroup> to create a new C4Group.
*/
void C4GROUP_freeGroup(C4GroupFile_t * grpFile, bool keepSubgroups);

/*
	Function: C4GROUP_getError

 	Retrieves the last error.

 	Returns:
 		The last error, duh.
*/
const char * C4GROUP_getError();

/*
	Function: C4GROUP_newGroup

	Creates and initializes a new <C4GroupFile_t> structure.

	Returns:
		A pointer to a new and empty <C4GroupFile_t> structure.

	See Also:
 		- <C4GROUP_freeGroup> to deallocate groups.
*/
C4GroupFile_t * C4GROUP_newGroup();

/*
	Function: C4GROUP_pack

	Packs a <C4GroupFile_t> structure into a non-null-terminated string.

	Parameters:
		grp - The <C4GroupFile_t> structure to pack.
		size - A pointer to an unsigned integer variable, that will contain the amount of packed data after calling this function.
		compress - True, if data should be compressed (with altered magic bytes).

	See Also:
 		- <C4GROUP_getError> for retrieving an error message, if return value is NULL.
 		- <C4GROUP_unpack> to unpack a C4Group.
*/
unsigned char * C4GROUP_pack(C4GroupFile_t * grp, unsigned int * size, bool compress);

/*
	Function: C4GROUP_removeFile

	Removes a <C4File_t> form a <C4GroupFile_t> structure. The file is identified by its name. The <C4File_t> structure is destroyed in this process.

	Parameters:
		grp - A pointer to the <C4GroupFile_t> struct, which contains the file.
		name - The name of the file.

	Returns:
		A string representing the C4Group or NULL in case of error. Call "free()" to deallocate this.

	See Also:
 		- <C4GROUP_addFile> to add files.
*/
void C4GROUP_removeFile(C4GroupFile_t * grp, char * name);

/*
	Function: C4GROUP_removeGroup

	Removes a <C4GroupFile_t> structure form another C4Group.

	Parameters:
		grp - The group, from which the other group will be removed.
		grp2 - The group that will be removed. This group is *NOT* freed in this process, but only removed from the other group. To free this, call <C4GROUP_freeGroup>.

	See Also:
 		- <C4GROUP_addGroup> to add groups.
*/

void C4GROUP_removeGroup(C4GroupFile_t * grp, C4GroupFile_t * grp2);

/*
	Function: C4GROUP_setFileContent

	(Re-)sets the content of a <C4File_t> in a <C4GroupFile_s> structure. The file is identified by its name.

	Parameters:
		grp - A pointer to the <C4GroupFile_t> struct, which contains the file.
		name - The name of the file.
		content - A pointer to the new content.
		fileSize - The size of the new content.

	See Also:
 		- <C4GROUP_setFileProp> for changing other properties of a <C4File_t> structure.
*/
void C4GROUP_setFileContent(C4GroupFile_t * grp, char * name, char * content, unsigned int fileSize);

/*
	Function: C4GROUP_setFileProp

	Sets a property of a <C4File_t> in a <C4GroupFile_t> structure. The file is identified by its name.

	Parameters:
		grp - A pointer to the <C4GroupFile_t> struct, which contains the file.
		name - The name of the file.
		prop - The name of the field to alter. May be one of: "name", "directory", "lastModification" or "executable".
		value - The new value of the field. Must be either a char pointer for "name", an int pointer for "lastModification" or a bool pointer for "directory" and "executable". Note: The string for the name is capped if it is too large.

	See Also:
 		- <C4GROUP_setFileContent> for changing the content of a <C4File_t> structure.
*/
void C4GROUP_setFileProp(C4GroupFile_t * grp, char * name, char * prop, void * value);

/*
	Function: C4GROUP_setHeaderProp

	Sets a value in the <C4GroupHeader_t> structure. The fields "fileCount" and "groupCount" are not accessible as they are automatically managed.

	Parameters:
		grp - A pointer to the <C4GroupFile_t> struct, whose header should be altered.
		prop - The name of the field to alter. May be one of: "author", "creationTime", "name" or "original".
		value - The new value of the field. Must be either a char pointer for "author" and "name", an int pointer for "creationTime" or a bool pointer for "original". Note: The strings for author and name are capped if they are too large.
*/
void C4GROUP_setHeaderProp(C4GroupFile_t * grp, char * prop, void * value);

/*
	Function: C4GROUP_unpack

 	Unpacks a string representing the content of a C4Group file into a readable structure.

	Parameters:
		groupFileBuf - The buffer containing the content of the file.
		buffsize - The size of the groupFileStr (=> the size of the file).
		explode - A flag to indicate that the unpacking should be executed recursively.
		compressed - A flag to indicate that groupFileStr is compressed. If the magic bytes indicate that the groupFileStr is already decompressed, this flag is ignored.
		name - If not NULL, this name is written in the result. Used for recursive unpacking.

 	Returns:
 		A pointer to said structure or NULL if an error occurred.

 	See Also:
 		- <C4GROUP_getError> for retrieving an error message, if return value is NULL.
 		- <C4GROUP_freeGroup> for freeing the memory of the return value (in the case of successful unpacking).
 		- <C4GROUP_pack> to pack the C4Group again.
*/
C4GroupFile_t * C4GROUP_unpack(unsigned char * groupFileBuf, unsigned int buffsize, bool explode, bool compressed, const char * name);

#endif /* C4GROUP_H_ */
