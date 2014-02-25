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

#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

#include "../globals.h"
#include <exception>

using namespace std;

#include "io.h"

#define	DEPEXT_DIRENT
#include "depext.C"

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

/* ############################################################ */
namespace io {

#define IOASYNC
#ifdef	IOASYNC
#if 1
CRITICAL_SECTION cs;
long int tmax, trunning;
HANDLE threads[256];
bool rethrown = false;
char rethrowing[256];
SYSTEM_INFO sysinfo;

void rethrow(const char *what) {
  strcpy(rethrowing, what);
  rethrown = true;
}

void block() {
  EnterCriticalSection(&cs);
}

void release() {
  LeaveCriticalSection(&cs);
}

void init() {
  GetSystemInfo(&sysinfo);

  tmax = max(1, sysinfo.dwNumberOfProcessors);
  trunning = 0;

  memset(threads, 0, sizeof(threads));
  InitializeCriticalSection(&cs);
}

void append(HANDLE thread) {
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

void flush() {
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

void exit() {
  flush();

  DeleteCriticalSection(&cs);
}

void dispatch(DWORD (__stdcall * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct) {
  HANDLE th;

  /* halt on abort */
  if (rethrown) {
    flush(); rethrown = false;
    throw runtime_error(rethrowing);
  }

  /* async */
  if ((th = CreateThread(NULL, 0, _dispatchBase, _dispatchStruct, 0, NULL)) != INVALID_HANDLE_VALUE)
    append(th);
  /* sync */
  else
    _dispatchBase(_dispatchStruct);
}
#else
TP_CALLBACK_ENVIRON te;
PTP_POOL tp;

void init() {
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);

  InitializeThreadpoolEnvironment(&te);
  if ((tp = CreateThreadpool(NULL))) {
    SetThreadpoolThreadMaximum(tp, sysinfo.dwNumberOfProcessors);
    SetThreadpoolThreadMinimum(tp, 1);
    SetThreadpoolCallbackPriority(&te, TP_CALLBACK_PRIORITY_LOW);
  }
}

void dispatch(DWORD (__stdcall * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct) {
  if (tp) {
    PTP_WORK wrk;

    wrk = CreateThreadpoolWork(_dispatchBase, _dispatchStruct, NULL);
    SubmitThreadpoolWork(wrk);
  }
  else {
    _dispatchBase(_dispatchStruct);
  }
}

void exit() {
  if (tp) CloseThreadpool(tp);
  DestroyThreadpoolEnvironment(&te);
}
#endif
#else
void init() {}
void exit() {}
void flush() {}
void block() {}
void release() {}

inline void dispatch(DWORD (__stdcall * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct) {
  _dispatchBase(_dispatchStruct);
}
#endif

} // namespace io
/* ############################################################ */

typedef size_t (__cdecl *reader)(void *block, size_t elements, size_t size, void *file);
typedef size_t (__cdecl *writer)(const void *block, size_t elements, size_t size, void *file);
typedef int (__cdecl *seeker)(void *file, long offset, int origin);
typedef long int (__cdecl *teller)(void *file);
typedef void (__cdecl *closer)(void *file);
typedef int (__cdecl *renamer)(const char *inname, const char *ouname);
typedef int (__cdecl *unlinker)(const char *pathname);

struct IOFL {
  void *fle;

  reader read; writer write;
  seeker seek; teller tell;
  closer close;
  
  renamer rename;
  unlinker unlink;
};
  
struct IOFL *iopen(const char *pathname, const char *mode = "rb");
struct IOFL *iwrap(FILE *file);

#include "bsa.C"
#include "zip.C"

struct IOFL *iwrap(FILE *file) {
  struct IOFL *fl = (struct IOFL *)malloc(sizeof(struct IOFL));

  {
    fl->fle = file;
    fl->seek = (seeker)&fseek;
    fl->tell = (teller)&ftell;
    fl->read = (reader)&fread;
    fl->write = (writer)&fwrite;
    fl->close = (closer)&fclose;
    fl->rename = &rename;
    fl->unlink = &unlink;
  }

  return fl;
}

struct IOFL *iopen(const char *pathname, const char *mode) {
  struct IOFL *fl = (struct IOFL *)malloc(sizeof(struct IOFL));
  char *walk = strdup(pathname);
  char *shrt;

  {
    if ((shrt = strrchr(walk, '\\')))
      *shrt = '\0';

    if (isarchive(walk)) {
      fl->fle = (mode ? (void *)::fopen_arc(pathname, mode) : NULL);
      fl->seek = &fseek_arc;
      fl->tell = &ftell_arc;
      fl->read = &fread_arc;
      fl->write = &fwrite_arc;
      fl->close = &fclose_arc;
      fl->rename = &rename_arc;
      fl->unlink = &unlink_arc;
    }
    else if (iszip(walk)) {
      fl->fle = (mode ? (void *)::fopen_zip(pathname, mode) : NULL);
      fl->seek = &fseek_zip;
      fl->tell = &ftell_zip;
      fl->read = &fread_zip;
      fl->write = &fwrite_zip;
      fl->close = &fclose_zip;
      fl->rename = &rename_zip;
      fl->unlink = &unlink_zip;
    }
    else {
      fl->fle = (mode ? (void *)::fopen(pathname, mode) : NULL);
      fl->seek = (seeker)&fseek;
      fl->tell = (teller)&ftell;
      fl->read = (reader)&fread;
      fl->write = (writer)&fwrite;
      fl->close = (closer)&fclose;
      fl->rename = &rename;
      fl->unlink = &unlink;
    }
  }

  free(walk);
  return fl;
}

/* ############################################################ */
namespace io {

int stat(const char *pathname, struct info *info) {
  struct stat sinfo;
  int ret;
  
  /* archives return ctime, as mtime is skewed by BASH */
  if (isarchive(pathname))
    ret = stat_arc(pathname, info);
  else if (iszip(pathname))
    ret = stat_zip(pathname, info);
  else {
    DWORD atr = GetFileAttributes(pathname);
    if (atr == INVALID_FILE_ATTRIBUTES)
      return -1;
    if ((atr & FILE_ATTRIBUTE_HIDDEN) && !processhidden)
      return -1;

    if (!(ret = ::stat(pathname, &sinfo))) {
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

  return ret;
}

int time(const char *pathname, struct info *info) {
  if (isarchive(pathname))
    return 0;
  if (iszip(pathname))
    return 0;

  {
    struct utimbuf otm;

    otm.actime = ::time(NULL);
    otm.modtime = info->io_time;

    return ::utime(pathname, &otm);
  }
}

int time(const char *takename, const char *pathname) {
  struct info info;

  if (!stat(takename, &info))
    return time(pathname, &info);

  return -1;
}

time_t time(const char *pathname) {
  struct info info;

  if (!stat(pathname, &info))
    return info.io_time;

  return -1;
}

size_t size(const char *pathname) {
  struct info info;

  if (!stat(pathname, &info))
    return info.io_size;

  return -1;
}

/* ------------------------------------------------------------ */

int rename(const char *inname, const char *ouname) {
  // requires arc/zip to handle inter-archive/zip movements
  if (isarchive(inname) && isarchive(ouname))
    return ::rename_arc(inname, ouname);
  else if (iszip(inname) && iszip(ouname))
    return ::rename_zip(inname, ouname);
  else if (!isarchive(inname) && !isarchive(ouname) && 
           !isarchive(ouname) && !isarchive(ouname))
    return ::rename(inname, ouname);
  else {
    // check if file or folder
    struct info sinfo, dinfo;

    if (stat(inname, &sinfo) == -1)
      return -1;
    if (stat(ouname, &dinfo) == -1)
      return -1;
    
    if ((sinfo.io_type == IO_DIRECTORY) &&
        (dinfo.io_type == IO_FILE))
      return -1;

    if ((sinfo.io_type == IO_DIRECTORY) &&
        (dinfo.io_type == IO_DIRECTORY)) {
      if ((mkdir(ouname) == -1) ||
	  (rmdir(inname) == -1))
	return -1;

      return 0;
    }

    if ((sinfo.io_type == IO_FILE) &&
        (dinfo.io_type == IO_FILE)) {
      if ((cp(inname, ouname) == -1) ||
	  (unlink(inname) == -1))
	return -1;

      return 0;
    }

    // append src-filename to dest-folder
    abort();
    return 0;
  }
}

int cp(const char *inname, const char *ouname) {
  void *infle; reader iread; closer iclse;
  void *oufle; writer owrte; closer oclse;
  
  if (isarchive(inname)) {
    infle = ::fopen_arc(inname, "rb");
    iread = &fread_arc;
    iclse = &fclose_arc;
  }
  else if (iszip(inname)) {
    infle = ::fopen_zip(inname, "rb");
    iread = &fread_zip;
    iclse = &fclose_zip;
  }
  else {
    infle = ::fopen(inname, "rb");
    iread = (reader)&fread;
    iclse = (closer)&fclose;
  }
  
  if (isarchive(ouname)) {
    oufle = ::fopen_arc(ouname, "wb");
    owrte = &fwrite_arc;
    oclse = &fclose_arc;
  }
  else if (iszip(ouname)) {
    oufle = ::fopen_zip(ouname, "wb");
    owrte = &fwrite_zip;
    oclse = &fclose_zip;
  }
  else {
    oufle = ::fopen(ouname, "wb");
    owrte = (writer)&fwrite;
    oclse = (closer)&fclose;
  }

  if (infle && oufle) {
    void *mem = malloc(1024 * 1024);
    size_t rd, sz = 0;

    if (!mem) {
      sprintf(rerror, "Unable to allocate memory to process \"%s\"\n", inname);
      throw runtime_error(rerror);
    }

    while ((rd = iread(mem, 1, 1024 * 1024, infle)) > 0) {
      sz += rd;
      if (owrte(mem, 1, rd, oufle) != rd) {
	sprintf(rerror, "Disk full\n"); free(mem);
	throw runtime_error(rerror);
      }
    }

    if (sz == 0) {
      struct info info;
      if (stat(inname, &info)) {
	sprintf(rerror, "Unable to access \"%s\"\n", inname); free(mem);
	throw runtime_error(rerror);
      }

      if (info.io_size != 0) {
	sprintf(rerror, "Unable to read from \"%s\"\n", inname); free(mem);
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

  if (infle) iclse(infle);
  if (oufle) oclse(oufle);

  /* clone time-stamp */
  return time(inname, ouname);
}

/* ------------------------------------------------------------ */

int mkdir(const char *pathname) {
  int ret;

  if (isarchive(pathname))
    ret = ::mkdir_arc(pathname);
  else if (iszip(pathname))
    ret = ::mkdir_zip(pathname);
  else
    ret = ::mkdir_rec(pathname);

  return ret;
}

int rmdir(const char *pathname) {
  int ret;

  if (isarchive(pathname))
    ret = ::rmdir_arc(pathname);
  else if (iszip(pathname))
    ret = ::rmdir_zip(pathname);
  else
    ret = ::rmdir(pathname);

  return ret;
}

struct dir *opendir(const char *pathname) {
  struct dir *dir = NULL;

  if (isarchive(pathname)) {
    void *ad = NULL;

    if ((ad = ::opendir_arc(pathname))) {
      dir = new struct dir;
      dir->od = NULL;
      dir->ad = ad;
      dir->zd = NULL;
    }
  }
  else if (iszip(pathname)) {
    void *zd = NULL;

    if ((zd = ::opendir_zip(pathname))) {
      dir = new struct dir;
      dir->od = NULL;
      dir->ad = NULL;
      dir->zd = zd;
    }
  }
  else {
    DIR *od = NULL;

    if ((od = ::opendir(pathname))) {
      dir = new struct dir;
      dir->od = od;
      dir->ad = NULL;
      dir->zd = NULL;
    }
  }

  return dir;
}

struct dirent *readdir(struct dir *dir) {
  if (isarchive(dir)) {
    struct ::dirent *de;
    if ((de = ::readdir_arc(dir->ad))) {
      strcpy(dir->oe.name, de->d_name);
      dir->oe.namelength = de->d_namlen;

      return &dir->oe;
    }

    return NULL;
  }
  else if (iszip(dir)) {
    struct ::dirent *de;
    if ((de = ::readdir_arc(dir->zd))) {
      strcpy(dir->oe.name, de->d_name);
      dir->oe.namelength = de->d_namlen;

      return &dir->oe;
    }

    return NULL;
  }
  else {
    struct ::dirent *de;
    if ((de = ::readdir(dir->od))) {
      strcpy(dir->oe.name, de->d_name);
      dir->oe.namelength = de->d_namlen;

      return &dir->oe;
    }

    return NULL;
  }
}

void closedir(struct dir *dir) {
  if (isarchive(dir)) {
    ::closedir_arc(dir->ad);
    delete dir;
  }
  else if (iszip(dir)) {
    ::closedir_zip(dir->zd);
    delete dir;
  }
  else {
    ::closedir(dir->od);
    delete dir;
  }
}

/* ------------------------------------------------------------ */

struct dir *openpattern(const char *pathname) {
  abort(); return NULL;
}

struct dirent *readpattern(struct ir *dir) {
  abort(); return NULL;
}

void closepattern(struct dir *dir) {
  abort();
}

/* ------------------------------------------------------------ */

int unlink(const char *pathname) {
  if (isarchive(pathname))
    return ::unlink_arc(pathname);
  else if (iszip(pathname))
    return ::unlink_zip(pathname);
  else
    return ::unlink(pathname);
}

struct file *openfile(const char *pathname, const char *mode) {
  struct file *file = NULL;

  if (isarchive(pathname)) {
    void *af = NULL;
    if ((af = ::fopen_arc(pathname, mode))) {
      file = new struct file;
      file->of = NULL;
      file->af = af;
      file->zf = NULL;
    }
  }
  else if (iszip(pathname)) {
    void *zf = NULL;
    if ((zf = ::fopen_arc(pathname, mode))) {
      file = new struct file;
      file->of = NULL;
      file->af = NULL;
      file->zf = zf;
    }
  }
  else {
    FILE *of = NULL;
    if ((of = ::fopen(pathname, mode))) {
      file = new struct file;
      file->of = of;
      file->af = NULL;
      file->zf = NULL;
    }
  }

  return file;
}

size_t readfile(void *block, size_t size, struct file *file) {
  if (isarchive(file))
    return ::fread_arc(block, 1, size, file->af);
  else if (iszip(file))
    return ::fread_zip(block, 1, size, file->zf);
  else
    return ::fread(block, 1, size, file->of);
}

char getc(struct file *file) {
  if (isarchive(file))
    return ::getc_arc(file->af);
  else if (iszip(file))
    return ::getc_zip(file->zf);
  else
    return ::getc(file->of);
}

size_t writefile(const void *block, size_t size, struct file *file) {
  if (isarchive(file))
    return ::fwrite_arc(block, 1, size, file->af);
  else if (iszip(file))
    return ::fwrite_zip(block, 1, size, file->zf);
  else
    return ::fwrite(block, 1, size, file->of);
}

int puts(const char *str, struct file *file) {
  if (isarchive(file))
    return ::fputs_arc(str, file->af);
  else if (iszip(file))
    return ::fputs_zip(str, file->zf);
  else
    return ::fputs(str, file->of);
}

void closefile(struct file *file) {
  if (isarchive(file)) {
    ::fclose_arc(file->af);
    delete file;
  }
  else if (iszip(file)) {
    ::fclose_zip(file->zf);
    delete file;
  }
  else {
    ::fclose(file->of);
    delete file;
  }
}

int eof(struct file *file) {
  if (isarchive(file))
    return ::feof_arc(file->af);
  else if (iszip(file))
    return ::feof_zip(file->zf);
  else
    return ::feof(file->of);
}

int seek(struct file *file, long offset, int origin) {
  if (isarchive(file))
    return ::fseek_arc(file->af, offset, origin);
  else if (iszip(file))
    return ::fseek_zip(file->zf, offset, origin);
  else
    return ::fseek(file->of, offset, origin);
}

int seek(struct file *file, long long offset, int origin) {
  assert(offset <= 0x000000007FFFFFFFLL);
  assert(offset >= 0xFFFFFFFF80000000LL);

  return seek(file, (long)offset, origin);
}

int rewind(struct file *file) {
  return seek(file, 0L, SEEK_SET);
}

int stat(struct file *file, struct info *info) {
  struct stat sinfo;
  int ret = 0;

  if (file->of) {
    if (!(ret = ::fstat(fileno(file->of), &sinfo))) {
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
  else if (file->af)
    ret = ::fstat_arc(file->af, info);
  else if (file->zf)
    ret = ::fstat_zip(file->zf, info);

  return ret;
}

int flush(struct file *file) {
  return 0;
}

long int tell(struct file *file) {
  if (isarchive(file))
    return ::ftell_arc(file->af);
  else if (iszip(file))
    return ::ftell_zip(file->zf);
  else
    return ::ftell(file->of);
}

int error(struct file *file) {
  return 0;
}

void clearerr(struct file *file) {
}

void flushtofile(const void *block, size_t size, struct file *file) {
  /* TODO: nowait asynchronous */
  if (isarchive(file)) {
//#pragma omp single nowait
    {
      size_t written =

      ::fwrite_arc(block, 1, size, file->af);
      ::fclose_arc(file->af);

      delete file;
    }
  }
  else if (iszip(file)) {
//#pragma omp single nowait
    {
      size_t written =

      ::fwrite_zip(block, 1, size, file->zf);
      ::fclose_zip(file->zf);

      delete file;
    }
  }
  else {
//#pragma omp single nowait
    {
      size_t written =

      ::fwrite(block, 1, size, file->of);
      ::fclose(file->of);

      delete file;
    }
  }
}

} // namespace io
/* ############################################################ */

#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <iostream>

using namespace std;

/* ############################################################ */
namespace io {

istream *openistream(const char *pathname) {
  if (isarchive(pathname))
    return openistream_arc(pathname);
  else if (iszip(pathname))
    return openistream_zip(pathname);
  else
    return new ifstream(pathname, ifstream::binary);
}

ostream *openostream(const char *pathname) {
  if (isarchive(pathname))
    return openostream_arc(pathname);
  else if (iszip(pathname))
    return openostream_zip(pathname);
  else
    return new ofstream(pathname, ofstream::binary);
}

void closeistream(istream *ist) {
  if (isarchive(ist))
    closeistream_arc(ist);
  else if (iszip(ist))
    closeistream_zip(ist);
  else {
    ifstream *ifs = (ifstream *)ist;

    ifs->close();
    delete ifs;
  }
}

void closeostream(ostream *ost) {
  if (isarchive(ost))
    closeostream_arc(ost);
  else if (iszip(ost))
    closeostream_zip(ost);
  else {
    ofstream *ofs = (ofstream *)ost;

    ofs->close();
    delete ofs;
  }
}

long tellistream(istream *ist) {
  if (isarchive(ist))
    return tellistream_arc(ist);
  else if (iszip(ist))
    return tellistream_zip(ist);
  else {
    ifstream *ifs = (ifstream *)ist;

//  return ifs->tellg();
//  return (long)ifs->tellp();
    return (long)ifs->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in);
  }
}

long tellostream(ostream *ost) {
  if (isarchive(ost))
    return tellostream_arc(ost);
  else if (iszip(ost))
    return tellostream_zip(ost);
  else {
    ofstream *ofs = (ofstream *)ost;

//  return ofs->tellg();
//  return (long)ofs->tellp();
    return (long)ofs->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
  }
}

} // namespace io
