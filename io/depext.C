
#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

/* #################################################################################
 */

#ifdef	DEPEXT_ZLIB
#ifndef DEPEXT_ZLIB_INC
#define DEPEXT_ZLIB_INC

#include "zlib-1.2.3/zlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
     Compresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be at least the value returned
   by compressBound(sourceLen). Upon exit, destLen is the actual size of the
   compressed buffer.
     This function can be used to compress a whole file at once if the
   input file is mmap'ed.
     compress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer.
*/

ZEXTERN int ZEXPORT compress3 OF((Bytef *dest, uLongf *destLen,
				 const Bytef *source, uLong sourceLen,
				 int level, int method, int windowBits, int memLevel, int strategy));

ZEXTERN int ZEXPORT uncompress2 OF((Bytef *dest, uLongf *destLen,
				   const Bytef *source, uLong sourceLen,
				   int windowBits));

/* ===========================================================================
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/
int ZEXPORT compress3(Bytef *dest, uLongf *destLen,
		      const Bytef *source, uLong sourceLen,
		      int level, int method, int windowBits, int memLevel, int strategy)
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
#ifdef MAXSEG_64K
    /* Check for source > 64K on 16-bit machine: */
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;
#endif
    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit2(&stream, level, method, windowBits, memLevel, strategy);
    if (err != Z_OK) return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}

int ZEXPORT uncompress2(Bytef *dest, uLongf *destLen,
			const Bytef *source, uLong sourceLen,
			int windowBits)
{
  z_stream stream;
  int err;

  stream.next_in = (Bytef*)source;
  stream.avail_in = (uInt)sourceLen;
  /* Check for source > 64K on 16-bit machine: */
  if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

  stream.next_out = dest;
  stream.avail_out = (uInt)*destLen;
  if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;

  err = inflateInit2(&stream, windowBits);
  if (err != Z_OK) return err;

  err = inflate(&stream, Z_FINISH);
  if (err == Z_DATA_ERROR)
    *destLen = stream.total_out;
  if (err != Z_STREAM_END) {
    inflateEnd(&stream);
    if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
      return Z_DATA_ERROR;
    return err;
  }
  *destLen = stream.total_out;

  err = inflateEnd(&stream);
  return err;
}

#ifdef __cplusplus
}
#endif

#endif
#endif

/* #################################################################################
 */

#ifdef	DEPEXT_LIBZIP
#ifndef DEPEXT_LIBZIP_INC
#define DEPEXT_LIBZIP_INC

#include "libzip-0.10.1/lib/zip.h"
#endif
#endif

/* #################################################################################
 */

#ifdef	DEPEXT_7ZIP
#ifndef DEPEXT_7ZIP_INC
#define DEPEXT_7ZIP_INC

#define	MAXIMUM_COMPRESSION
#ifdef	MAXIMUM_COMPRESSION
#include "7z/InByte.cc"
#include "7z/OutByte.cc"
#include "7z/IInOutStreams.cc"
#include "7z/CRC.cc"
#include "7z/WindowIn.cc"
#include "7z/WindowOut.cc"
#include "7z/LSBFEncoder.cc"
#include "7z/LSBFDecoder.cc"
#include "7z/HuffmanEncoder.cc"
#include "7z/DeflateEncoder.cc"
#include "7z/DeflateDecoder.cc"
#include "7z/7zdeflate.cc"
#endif

#endif
#endif

/* #################################################################################
 */

#ifdef	DEPEXT_DIRENT
#ifndef DEPEXT_DIRENT_INC
#define DEPEXT_DIRENT_INC

// ######################## "libdirent\dirent.h"

#ifndef DIRENT_H
#define DIRENT_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dirent {
  unsigned char d_namlen;
  char d_name[260];
};

/* Definition of DIR requires many other headers; not included here to
   avoid namespace pollution. */
typedef struct {
  int num_read;
  char *name;

  WIN32_FIND_DATA ff;
  HANDLE ffhandle;

  struct dirent de;
} DIR;

/*
int closedir(DIR *dirp);
DIR *opendir(const char *_dirname);
struct dirent *readdir(DIR *_dirp);
void rewinddir(DIR *_dirp);

void seekdir(DIR *_dir, long _loc);
long telldir(DIR *_dir);
*/

#ifdef __cplusplus
}
#endif

#endif /* !DIRENT_H */

// ######################## "libdirent\rewinddir.c"

#include <dirent.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

void rewinddir(DIR *dir) {
  /* If we are using LFN-aware functions, close the handle used by
     Windows 9X during the search (readdir will open a new one).  */
  if (dir->ffhandle) {
    FindClose(dir->ffhandle);
    dir->ffhandle = 0;	/* 0 means it's closed */
  }

  dir->num_read = 0;
}

#ifdef __cplusplus
}
#endif

// ######################## "libdirent\closedir.c"

#include <stdlib.h>
#include <errno.h>
#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

int closedir(DIR *dir) {
  int retval = 0;
  int e = errno;

  errno = 0;
  rewinddir(dir);	/* under LFN this closes the search handle */

  if (errno == 0)
    errno = e;
  else
    retval = -1;

  free(dir->name);
  free(dir);

  return retval;
}

#ifdef __cplusplus
}
#endif

// ######################## "libdirent\fixpath.c"

#include <stdio.h>		/* For FILENAME_MAX */
#include <stdlib.h>
#include <errno.h>		/* For errno */
#include <string.h>		/* For strlen() */
#include <dirent.h>
#include <fcntl.h>		/* For LFN stuff */
#include <sys/stat.h>
#include <io.h>

#ifdef __cplusplus
extern "C" {
#endif

static int is_slash(int c) {
  return c == '/' || c == '\\';
}

static int is_term(int c) {
  return c == '/' || c == '\\' || c == '\0';
}

/* Takes as input an arbitrary path.  Fixes up the path by:
   1. Removing consecutive slashes
   2. Removing trailing slashes
   3. Making the path absolute if it wasn't already
   4. Removing "." in the path
   5. Removing ".." entries in the path (and the directory above them)
   6. Adding a drive specification if one wasn't there
   7. Converting all slashes to '/'
 */
void _fixpath(const char *in, char *out) {
  int		 drive_number;
  const char *ip;
  char		 *op = out;
  char		 *name_start;

  /* Perform the same magic conversions that _put_path does.  */
  ip = in;

  /* Add drive specification to output string */
  if (((*ip >= 'a' && *ip <= 'z') ||
       (*ip >= 'A' && *ip <= 'Z'))
      && (*(ip + 1) == ':')) {
    if (*ip >= 'a' && *ip <= 'z')
      drive_number = *ip - 'a';
    else
      drive_number = *ip - 'A';

    ip += 2;
  }
  else {
    char path[MAX_PATH];
    char fullpath[MAX_PATH];
    char *filepath;

    if (GetCurrentDirectory(MAX_PATH, path))
      if (GetFullPathName(path, MAX_PATH, fullpath, &filepath)) {
        if (fullpath[0] >= 'a' && fullpath[0] <= 'z')
    	  drive_number = fullpath[0] - 'a';
    	else
    	  drive_number = fullpath[0] - 'A';
      }
  }

  *op++ = drive_number + 'a';
  *op++ = ':';

  /* Convert relative path to absolute */
  if (!is_slash(*ip)) {
    char path[MAX_PATH];
    char fullpath[MAX_PATH];
    char *filepath;

    if (GetCurrentDirectory(MAX_PATH, path)) {
      if (GetFullPathName(path, MAX_PATH, fullpath, &filepath)) {
        if (fullpath[0] >= 'a' && fullpath[0] <= 'z')
    	  drive_number = fullpath[0] - 'a';
    	else
    	  drive_number = fullpath[0] - 'A';

    	strncpy(op, fullpath + 2, MAX_PATH - 2);
    	op = op + strlen(op);
      }
    }
  }

  /* Step through the input path */
  while (*ip) {
    /* Skip input slashes */
    if (is_slash(*ip)) {
      ip++;
      continue;
    }

    /* Skip "." and output nothing */
    if (*ip == '.' && is_term(*(ip + 1))) {
      ip++;
      continue;
    }

    /* Skip ".." and remove previous output directory */
    if (*ip == '.' && *(ip + 1) == '.' && is_term(*(ip + 2))) {
      ip += 2;
      /* Don't back up over drive spec */
      if (op > out + 2)
	/* This requires "/" to follow drive spec */
	while (!is_slash(*--op));
    	  continue;
    }

    /* Copy path component from in to out */
    *op++ = '\\';
    while (!is_term(*ip))
      *op++ = *ip++;
  }

  /* If root directory, insert trailing slash */
  if (op == out + 2)
    *op++ = '\\';

  /* Null terminate the output */
  *op = '\0';

  /* switch FOO\BAR to foo/bar, downcase where appropriate */
  for (op = out + 3, name_start = op - 1; *name_start; op++) {
    if (*op == '/')
      *op = '\\';
    if (*op == '\0')
      break;
  }
}

#ifdef __cplusplus
}
#endif

// ######################## "libdirent\opendir.c"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

#ifdef __cplusplus
extern "C" {
#endif

DIR *opendir(const char *name) {
  size_t length;
  DIR *dir = (DIR *)malloc(sizeof(DIR));

  if (dir == 0)
    return 0;

  dir->num_read = 0;
  dir->name = (char *)malloc(MAX_PATH);
  if (dir->name == 0) {
    free(dir);

    return 0;
  }

  /* Make absolute path */
  _fixpath(name, dir->name);

  /* Ensure that directory to be accessed exists */
  if ((dir->ffhandle = FindFirstFile(dir->name, &dir->ff)) == INVALID_HANDLE_VALUE) {
    dir->ffhandle = 0;

    free(dir->name);
    free(dir);

    return 0;
  }
  else {
    FindClose(dir->ffhandle);
    dir->ffhandle = 0;
  }

  /* Strip trailing slashes, so we can append "/ *.*" */
  length = strlen(dir->name);
  while (1) {
    if (length == 0)
      break;

    length--;

    if ((dir->name[length] == '/') || (dir->name[length] == '\\'))
      dir->name[length] = '\0';
    else {
      length++;

      break;
    }
  }

  dir->name[length++] = '\\';
  dir->name[length++] = '*';
  dir->name[length++] = '.';
  dir->name[length++] = '*';
  dir->name[length++] = 0;

  return dir;
}

#ifdef __cplusplus
}
#endif

// ######################## "libdirent\readdir.c"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dirent *readdir(DIR *dir) {
  int done;
  int oerrno = errno;

  if (dir->num_read)
    done = FindNextFile(dir->ffhandle, &dir->ff);
  else
    done = (dir->ffhandle = FindFirstFile(dir->name, &dir->ff)) ? 1 : 0;

  if (!done)
    return 0;

  dir->num_read++;
  strcpy(dir->de.d_name, dir->ff.cFileName);
  dir->de.d_namlen = (char)strlen(dir->de.d_name);

  return &dir->de;
}

#ifdef __cplusplus
}
#endif

#endif
#endif
