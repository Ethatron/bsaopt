/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is BSAopt.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#include <sys/stat.h>
#include <sys/utime.h>
#include <sys/types.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

#define	DEPEXT_DIRENT
#include "io/depext.C"

/* ------------------------------------------------------------ */

#define IOASYNC
#ifdef	IOASYNC
#if 1
CRITICAL_SECTION cs;
long int tmax, trunning;
HANDLE threads[256];
bool rethrow = false;
char rethrowing[256];
SYSTEM_INFO sysinfo;

void iorethrow(const char *what) {
  strcpy(rethrowing, what);
  rethrow = true;
}

void ioblock() {
  EnterCriticalSection(&cs);
}

void iorelease() {
  LeaveCriticalSection(&cs);
}

void ioinit() {
  GetSystemInfo(&sysinfo);

  tmax = max(1, sysinfo.dwNumberOfProcessors);
  trunning = 0;

  memset(threads, 0, sizeof(threads));
  InitializeCriticalSection(&cs);
}

void ioappend(HANDLE thread) {
  /* prevent the threads to completely saturate the machine */
  SetThreadPriority(thread, THREAD_PRIORITY_BELOW_NORMAL);

  /* register thread */
  threads[trunning] = thread;

  /* and raise number of running threads */
  trunning++;

  /* and lower number of allowed threads */
  DWORD wait = 0, signaled;
  if (--tmax == 0)
    wait = INFINITE;

  /* wait for thread termination */
//while (WaitForSingleObject(threads[0], wait) == WAIT_OBJECT_0) {
  if ((signaled = WaitForMultipleObjects(trunning, threads, FALSE, wait)) != WAIT_TIMEOUT) {
    wait = 0;
    signaled -= WAIT_OBJECT_0;
    while ((signed)signaled < trunning) {
      if (WaitForSingleObject(threads[signaled], wait) == WAIT_OBJECT_0) {
	/* slice of thread */
	CloseHandle(threads[signaled]);
	memmove(threads + signaled + 0, threads + signaled + 1, sizeof(threads) - (signaled + 1) * sizeof(HANDLE));

	/* and lower number of running threads */
	trunning--;

	/* and raise number of allowed threads */
	++tmax;
      }
      else
	signaled++;
    }
  }
}

void ioflush() {
  if (trunning > 0) {
    /* wait for all thread termination */
    WaitForMultipleObjects(trunning, threads, TRUE, INFINITE);

    /* slice of thread */
    while (trunning-- > 0) {
      CloseHandle(threads[trunning]);
      threads[trunning] = NULL;
    }
  }

  tmax = max(1, sysinfo.dwNumberOfProcessors);
  trunning = 0;
}

void ioexit() {
  ioflush();

  DeleteCriticalSection(&cs);
}

void iodispatch(DWORD (__stdcall * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct) {
  HANDLE th;

  /* halt on abort */
  if (rethrow) {
    ioflush(); rethrow = false;
    throw runtime_error(rethrowing);
  }

  /* async */
  if ((th = CreateThread(NULL, 0, _dispatchBase, _dispatchStruct, 0, NULL)) != INVALID_HANDLE_VALUE)
    ioappend(th);
  /* sync */
  else
    _dispatchBase(_dispatchStruct);
}
#else
TP_CALLBACK_ENVIRON te;
PTP_POOL tp;

void ioinit() {
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);

  InitializeThreadpoolEnvironment(&te);
  if ((tp = CreateThreadpool(NULL))) {
    SetThreadpoolThreadMaximum(tp, sysinfo.dwNumberOfProcessors);
    SetThreadpoolThreadMinimum(tp, 1);
    SetThreadpoolCallbackPriority(&te, TP_CALLBACK_PRIORITY_LOW);
  }
}

void iodispatch(DWORD (__stdcall * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct) {
  if (tp) {
    PTP_WORK wrk;

    wrk = CreateThreadpoolWork(_dispatchBase, _dispatchStruct, NULL);
    SubmitThreadpoolWork(wrk);
  }
  else {
    _dispatchBase(_dispatchStruct);
  }
}

void ioexit() {
  if (tp) CloseThreadpool(tp);
  DestroyThreadpoolEnvironment(&te);
}
#endif
#else
void ioinit() {}
void ioexit() {}
void ioflush() {}
void ioblock() {}
void iorelease() {}

inline void iodispatch(DWORD (__stdcall * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct) {
  _dispatchBase(_dispatchStruct);
}
#endif

/* ------------------------------------------------------------ */

bool isext(const char *pathname, const char *ext) {
  const char *extp;

  if ((extp = strrchr(pathname, '.'))) {
    if (!strchr(extp + 1, '\\')) {
      return !stricmp(extp + 1, ext);
    }
  }

  return false;
}

char *getext(char *result, const char *pathname) {
  const char *extp;

  *result = '\0';
  if ((extp = strrchr(pathname, '.'))) {
    if (!strchr(extp + 1, '\\')) {
      strcpy(result, extp + 1);
    }
  }

  return result;
}

char *putext(char *result, const char *pathname, const char *ext) {
  char *extp;

  if (result != pathname)
    strcpy(result, pathname);
  if ((extp = strrchr(result, '.'))) {
    if (!strchr(extp, '\\')) {
      *extp = '\0';
    }
  }

  strcat(result, ".");
  strcat(result, ext);

  return result;
}

/* ------------------------------------------------------------ */

bool isext(const string &pathname, const char *ext) {
  const char *extp;

  if ((extp = strrchr(pathname.data(), '.'))) {
    if (!strchr(extp + 1, '\\')) {
      return !stricmp(extp + 1, ext);
    }
  }

  return false;
}

string &getext(string &result, const string &pathname) {
  const char *extp;

  result = "\0";
  if ((extp = strrchr(pathname.data(), '.'))) {
    if (!strchr(extp + 1, '\\')) {
      result = extp + 1;
    }
  }

  return result;
}

string &putext(string &result, const string &pathname, const char *ext) {
  const char *extp;

  if (result.data() != pathname.data())
    result = pathname;
  if ((extp = strrchr(result.data(), '.'))) {
    if (!strchr(extp, '\\')) {
      result = result.substr(0, extp - result.data());
    }
  }

  result += ".";
  result += ext;

  return result;
}

/* ------------------------------------------------------------ */

bool issuf(const char *pathname, const char *suffix) {
  char *extp;
  char check[1024];

  strcpy(check, pathname);
  if ((extp = strrchr(check, '.')))
    *extp = '\0';

  return !stricmp(check + strlen(check) - strlen(suffix), suffix);
}

char *setsuf(char *result, const char *pathname, const char *suffix) {
  char extbak[1024];
  char *extp;
  char *sufp;

  if (result != pathname)
    strcpy(result, pathname);
  if ((extp = strrchr(result, '.')))
    *extp = '\0';
  if ((sufp = strrchr(result, '_')))
    *sufp = '\0';
  /* backup ext, as it may be erased by the suffix */
  if (extp && extp[1])
    strcpy(extbak, extp + 1);
  strcat(result, suffix);
  if (extp && extp[1]) {
    strcat(result, ".");
    strcat(result, extbak);
  }

  return result;
}

char *putsuf(char *result, const char *pathname, const char *suffix) {
  char extbak[1024];
  char *extp;

  if (result != pathname)
    strcpy(result, pathname);
  if ((extp = strrchr(result, '.')))
    *extp = '\0';
  /* backup ext, as it may be erased by the suffix */
  if (extp && extp[1])
    strcpy(extbak, extp + 1);
  strcat(result, suffix);
  if (extp && extp[1]) {
    strcat(result, ".");
    strcat(result, extbak);
  }

  return result;
}

/* ------------------------------------------------------------ */

#define IO_DIRECTORY  1
#define IO_FILE	      2
#define IO_PATTERN    3	/* TODO */
#define IO_ARCHIVE    IO_DIRECTORY

struct ioinfo {
  unsigned short io_type;
  size_t io_size;
  size_t io_raws;
  time_t io_time;
};

struct iodirent {
  int namelength;
  char name[1024];
};

struct iodir {
  DIR  *od;
  void *ad;

  struct iodirent oe;
};

struct iofile {
  FILE *of;
  void *af;
};

#include "io/bsa.C"

/* ------------------------------------------------------------ */

int iostat(const char *pathname, struct ioinfo *info) {
  struct stat sinfo;
  int ret;

  if (!isarchive(pathname)) {
    DWORD atr = GetFileAttributes(pathname);
    if (atr == INVALID_FILE_ATTRIBUTES)
      return -1;
    if ((atr & FILE_ATTRIBUTE_HIDDEN) && !processhidden)
      return -1;

    if (!(ret = stat(pathname, &sinfo))) {
      if (sinfo.st_mode & S_IFDIR) {
        info->io_type = IO_DIRECTORY;
	info->io_size = sinfo.st_size;
	info->io_raws = (atr & FILE_ATTRIBUTE_COMPRESSED ? -1 : sinfo.st_size);
	info->io_time = sinfo.st_mtime;
      }
      else {
        info->io_type = IO_FILE;
	info->io_size = sinfo.st_size;
	info->io_raws = (atr & FILE_ATTRIBUTE_COMPRESSED ? -1 : sinfo.st_size);
	info->io_time = sinfo.st_mtime;
      }
    }
  }
  else
    /* archives return ctime, as mtime is skewed by BASH */
    ret = stat_arc(pathname, info);

  return ret;
}

int iotime(const char *pathname, struct ioinfo *info) {
  if (!isarchive(pathname)) {
    struct utimbuf otm;

    otm.actime = time(NULL);
    otm.modtime = info->io_time;

    return utime(pathname, &otm);
  }

  return 0;
}

int iotime(const char *takename, const char *pathname) {
  struct ioinfo info;

  if (!iostat(takename, &info))
    return iotime(pathname, &info);

  return -1;
}

time_t iotime(const char *pathname) {
  struct ioinfo info;

  if (!iostat(pathname, &info))
    return info.io_time;

  return -1;
}

size_t iosize(const char *pathname) {
  struct ioinfo info;

  if (!iostat(pathname, &info))
    return info.io_size;

  return -1;
}

/* ------------------------------------------------------------ */

void iocp(const char *inname, const char *ouname) {
  if (!isarchive(inname) && !isarchive(ouname)) {
    FILE *infle = fopen(inname, "rb");
    FILE *oufle = fopen(ouname, "wb");

    if (infle && oufle) {
      void *mem = malloc(1024 * 1024);
      size_t rd, sz = 0;

      while ((rd = fread(mem, 1, 1024 * 1024, infle)) > 0) {
	sz += rd;
	if (fwrite(mem, 1, rd, oufle) != rd) {
	  sprintf(rerror, "Disk full\n");
	  throw runtime_error(rerror);
        }
      }

      if (sz == 0) {
	struct ioinfo info;
	if (iostat(inname, &info)) {
	  sprintf(rerror, "Unable to access \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}

	if (info.io_size != 0) {
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}
      }

      free(mem);
    }
    else {
      if (!infle) {
	sprintf(rerror, "Unable to open input \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      if (!oufle) {
	sprintf(rerror, "Unable to open output \"%s\"\n", ouname);
	throw runtime_error(rerror);
      }
    }

    if (infle) fclose(infle);
    if (oufle) fclose(oufle);
  }
  else if (isarchive(inname) && isarchive(ouname)) {
    void *infle = fopen_arc(inname, "rb");
    void *oufle = fopen_arc(ouname, "wb");

    if (infle && oufle) {
      void *mem = malloc(1024 * 1024);
      size_t rd, sz = 0;

      if (!mem) {
	sprintf(rerror, "Unable to allocate memory to process \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      while ((rd = fread_arc(mem, 1, 1024 * 1024, infle)) > 0) {
	sz += rd;
	if (fwrite_arc(mem, 1, rd, oufle) != rd) {
	  sprintf(rerror, "Disk full\n"); free(mem);
	  sprintf(rerror, "Disk full\n");
	  throw runtime_error(rerror);
	}
      }

      if (sz == 0) {
	struct ioinfo info;
	if (iostat(inname, &info)) {
	  sprintf(rerror, "Unable to access \"%s\"\n", inname); free(mem);
	  sprintf(rerror, "Unable to access \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}

	if (info.io_size != 0) {
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname); free(mem);
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}
      }

      free(mem);
    }
    else {
      if (!infle) {
	sprintf(rerror, "Unable to open input \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      if (!oufle) {
	sprintf(rerror, "Unable to open output \"%s\"\n", ouname);
	throw runtime_error(rerror);
      }
    }

    if (infle) fclose_arc(infle);
    if (oufle) fclose_arc(oufle);
  }
  else if (isarchive(inname)) {
    void *infle = fopen_arc(inname, "rb");
    FILE *oufle = fopen    (ouname, "wb");

    if (infle && oufle) {
      void *mem = malloc(1024 * 1024);
      size_t rd, sz = 0;

      if (!mem) {
	sprintf(rerror, "Unable to allocate memory to process \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      while ((rd = fread_arc(mem, 1, 1024 * 1024, infle)) > 0) {
	sz += rd;
	if (fwrite(mem, 1, rd, oufle) != rd) {
	  sprintf(rerror, "Disk full\n"); free(mem);
	  sprintf(rerror, "Disk full\n");
	  throw runtime_error(rerror);
	}
      }

      if (sz == 0) {
	struct ioinfo info;
	if (iostat(inname, &info)) {
	  sprintf(rerror, "Unable to access \"%s\"\n", inname); free(mem);
	  sprintf(rerror, "Unable to access \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}

	if (info.io_size != 0) {
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname); free(mem);
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}
      }

      free(mem);
    }
    else {
      if (!infle) {
	sprintf(rerror, "Unable to open input \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      if (!oufle) {
	sprintf(rerror, "Unable to open output \"%s\"\n", ouname);
	throw runtime_error(rerror);
      }
    }

    if (infle) fclose_arc(infle);
    if (oufle) fclose    (oufle);
  }
  else if (isarchive(ouname)) {
    FILE *infle = fopen    (inname, "rb");
    void *oufle = fopen_arc(ouname, "wb");

    if (infle && oufle) {
      void *mem = malloc(1024 * 1024);
      size_t rd, sz = 0;

      if (!mem) {
	sprintf(rerror, "Unable to allocate memory to process \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      while ((rd = fread(mem, 1, 1024 * 1024, infle)) > 0) {
	sz += rd;
	if (fwrite_arc(mem, 1, rd, oufle) != rd) {
	  sprintf(rerror, "Disk full\n"); free(mem);
	  sprintf(rerror, "Disk full\n");
	  throw runtime_error(rerror);
	}
      }

      if (sz == 0) {
	struct ioinfo info;
	if (iostat(inname, &info)) {
	  sprintf(rerror, "Unable to access \"%s\"\n", inname); free(mem);
	  sprintf(rerror, "Unable to access \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}

	if (info.io_size != 0) {
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname); free(mem);
	  sprintf(rerror, "Unable to read from \"%s\"\n", inname);
	  throw runtime_error(rerror);
	}
      }

      free(mem);
    }
    else {
      if (!infle) {
	sprintf(rerror, "Unable to open input \"%s\"\n", inname);
	throw runtime_error(rerror);
      }

      if (!oufle) {
	sprintf(rerror, "Unable to open output \"%s\"\n", ouname);
	throw runtime_error(rerror);
      }
    }

    if (infle) fclose    (infle);
    if (oufle) fclose_arc(oufle);
  }

  /* clone time-stamp */
  iotime(inname, ouname);
}

/* ------------------------------------------------------------ */

int mkdir_rec(const char *pathname) {
  size_t len, l;
  char buf[1024];
  int ret = -1;

  len = strlen(pathname);
  strcpy(buf, pathname);

  /* iterate down */
  while ((ret = mkdir(buf))) {
    char *sep = strrchr(buf, '\\');
    if (!sep)
      break;
    *sep = '\0';
  }

  /* back up again */
  while ((l = strlen(buf)) != len) {
    char *sep = buf + l;
    *sep = '\\';
    if ((ret = mkdir(buf)))
      break;
  }

  return ret;
}

int iomkdir(const char *pathname) {
  int ret;

  if (!isarchive(pathname))
    ret = mkdir_rec(pathname);
  else
    ret = mkdir_arc(pathname);

  return ret;
}

int iormdir(const char *pathname) {
  int ret;

  if (!isarchive(pathname))
    ret = rmdir(pathname);
  else
    ret = rmdir_arc(pathname);

  return ret;
}

struct iodir *ioopendir(const char *pathname) {
  struct iodir *dir = NULL;

  if (!isarchive(pathname)) {
    DIR *od = NULL;

    if ((od = opendir(pathname))) {
      dir = new struct iodir;
      dir->od = od;
      dir->ad = NULL;
    }
  }
  else {
    void *ad = NULL;

    if ((ad = opendir_arc(pathname))) {
      dir = new struct iodir;
      dir->od = NULL;
      dir->ad = ad;
    }
  }

  return dir;
}

struct iodirent *ioreaddir(struct iodir *dir) {
  if (!isarchive(dir)) {
    struct dirent *de;

    if ((de = readdir(dir->od))) {
      strcpy(dir->oe.name, de->d_name);
      dir->oe.namelength = de->d_namlen;

      return &dir->oe;
    }

    return NULL;
  }
  else {
    struct dirent *de;

    if ((de = readdir_arc(dir->ad))) {
      strcpy(dir->oe.name, de->d_name);
      dir->oe.namelength = de->d_namlen;

      return &dir->oe;
    }

    return NULL;
  }
}

void ioclosedir(struct iodir *dir) {
  if (!isarchive(dir)) {
    closedir(dir->od);
    delete dir;
  }
  else {
    closedir_arc(dir->ad);
    delete dir;
  }
}

/* ------------------------------------------------------------ */

struct iodir *ioopenpattern(const char *pathname) {
  abort(); return NULL;
}

struct iodirent *ioreadpattern(struct iodir *dir) {
  abort(); return NULL;
}

void ioclosepattern(struct iodir *dir) {
  abort();
}

/* ------------------------------------------------------------ */

struct iofile *ioopenfile(const char *pathname, const char *mode) {
  struct iofile *file = NULL;

  if (!isarchive(pathname)) {
    FILE *of = NULL;

    if ((of = fopen(pathname, mode))) {
      file = new struct iofile;
      file->of = of;
      file->af = NULL;
    }
  }
  else {
    void *af = NULL;

    if ((af = fopen_arc(pathname, mode))) {
      file = new struct iofile;
      file->of = NULL;
      file->af = af;
    }
  }

  return file;
}

size_t ioreadfile(void *block, size_t size, struct iofile *file) {
  if (!isarchive(file))
    return fread(block, 1, size, file->of);
  else
    return fread_arc(block, 1, size, file->af);
}

char iogetc(struct iofile *file) {
  if (!isarchive(file))
    return getc(file->of);
  else
    return getc_arc(file->af);
}

size_t iowritefile(const void *block, size_t size, struct iofile *file) {
  if (!isarchive(file))
    return fwrite(block, 1, size, file->of);
  else
    return fwrite_arc(block, 1, size, file->af);
}

int ioputs(const char *str, struct iofile *file) {
  if (!isarchive(file))
    return fputs(str, file->of);
  else
    return fputs_arc(str, file->af);
}

void ioclosefile(struct iofile *file) {
  if (!isarchive(file)) {
    fclose(file->of);
    delete file;
  }
  else {
    fclose_arc(file->af);
    delete file;
  }
}

int ioeof(struct iofile *file) {
  if (!isarchive(file))
    return feof(file->of);
  else
    return feof_arc(file->af);
}

int ioseek(struct iofile *file, long offset, int origin) {
  if (!isarchive(file))
    return fseek(file->of, offset, origin);
  else
    return fseek_arc(file->af, offset, origin);
}

int ioseek(struct iofile *file, long long offset, int origin) {
  assert(offset <= 0x000000007FFFFFFFLL);
  assert(offset >= 0xFFFFFFFF80000000LL);

  return ioseek(file, (long)offset, origin);
}

int iorewind(struct iofile *file) {
  return ioseek(file, 0L, SEEK_SET);
}

int iostat(struct iofile *file, struct ioinfo *info) {
  struct stat sinfo;
  int ret;

  if (file->of) {
    if (!(ret = fstat(fileno(file->of), &sinfo))) {
      if (sinfo.st_mode & S_IFDIR) {
	info->io_type = IO_DIRECTORY;
	info->io_size = sinfo.st_size;
	info->io_time = sinfo.st_mtime;
      }
      else {
	info->io_type = IO_FILE;
	info->io_size = sinfo.st_size;
	info->io_time = sinfo.st_mtime;
      }
    }
  }
  else
    ret = fstat_arc(file->af, info);

  return ret;
}

int ioflush(struct iofile *file) {
  return 0;
}

long int iotell(struct iofile *file) {
  if (!isarchive(file))
    return ftell(file->of);
  else
    return ftell_arc(file->af);
}

int ioerror(struct iofile *file) {
  return 0;
}

void ioclearerr(struct iofile *file) {
}

void ioflushtofile(const void *block, size_t size, struct iofile *file) {
  /* TODO: nowait asynchronous */
  if (!isarchive(file)) {
//#pragma omp single nowait
    {
      size_t written =

      fwrite(block, 1, size, file->of);
      fclose(file->of);

      delete file;
    }
  }
  else {
//#pragma omp single nowait
    {
      size_t written =

      fwrite_arc(block, 1, size, file->af);
      fclose_arc(file->af);

      delete file;
    }
  }
}

/* ------------------------------------------------------------ */

#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <iostream>

using namespace std;

istream *ioopenistream(const char *pathname) {
  if (!isarchive(pathname))
    return new ifstream(pathname, ifstream::binary);
  else
    return openistream_arc(pathname);
}

ostream *ioopenostream(const char *pathname) {
  if (!isarchive(pathname))
    return new ofstream(pathname, ofstream::binary);
  else
    return openostream_arc(pathname);
}

void iocloseistream(istream *ist) {
  if (!isarchive(ist)) {
    ifstream *ifs = (ifstream *)ist;

    ifs->close();
    delete ifs;
  }
  else
    closeistream_arc(ist);
}

void iocloseostream(ostream *ost) {
  if (!isarchive(ost)) {
    ofstream *ofs = (ofstream *)ost;

    ofs->close();
    delete ofs;
  }
  else
    closeostream_arc(ost);
}

long iotellistream(istream *ist) {
  if (!isarchive(ist)) {
    ifstream *ifs = (ifstream *)ist;

//  return ifs->tellg();
//  return (long)ifs->tellp();
    return (long)ifs->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in);
  }
  else
    return tellistream_arc(ist);
}

long iotellostream(ostream *ost) {
  if (!isarchive(ost)) {
    ofstream *ofs = (ofstream *)ost;

//  return ofs->tellg();
//  return (long)ofs->tellp();
    return (long)ofs->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
  }
  else
    return tellostream_arc(ost);
}
