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

#ifndef IO_H
#define IO_H

#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

#include <string>

using namespace std;

#include <sys/stat.h>
#include <sys/utime.h>
#include <sys/types.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "depext.h"

/* ------------------------------------------------------------ */

bool isext(const char *pathname, const char *ext);
char *getext(char *result, const char *pathname);
char *putext(char *result, const char *pathname, const char *ext);
bool isext(const string &pathname, const char *ext);

string &getext(string &result, const string &pathname);
string &putext(string &result, const string &pathname, const char *ext);

/* ------------------------------------------------------------ */

bool issuf(const char *pathname, const char *suffix);
char *setsuf(char *result, const char *pathname, const char *suffix);
char *putsuf(char *result, const char *pathname, const char *suffix);

/* ------------------------------------------------------------ */

int mkdir_rec(const char *pathname);

/* ############################################################ */
namespace io {

#define DELIMITER_OS  '\\'
#define DELIMITES_OS  "\\"

#define IO_DIRECTORY  1
#define IO_FILE	      2
#define IO_PATTERN    3	/* TODO */
#define IO_ARCHIVE    IO_DIRECTORY
#define IO_ZIP	      IO_DIRECTORY

struct info {
  unsigned short io_type;
  size_t io_size;
  size_t io_raws;
  time_t io_time;
};

struct dirent {
  int namelength;
  char name[1024];
};

struct dir {
  DIR  *od;
  void *ad;
  void *zd;

  struct dirent oe;
};

struct file {
  FILE *of;
  void *af;
  void *zf;
};

/* ---------------------------------------------------- */

void rethrow(const char *what);
void block();
void release();
void init();
void append(HANDLE thread);
void flush();
void exit();
void dispatch(DWORD (__cdecl * _dispatchBase)(LPVOID lp), LPVOID _dispatchStruct);

/* ------------------------------------------------------------ */

int stat(const char *pathname, struct info *info);
int time(const char *pathname, struct info *info);
int time(const char *takename, const char *pathname);
time_t time(const char *pathname);
size_t size(const char *pathname);

/* ------------------------------------------------------------ */

int rename(const char *inname, const char *ouname);
int cp(const char *inname, const char *ouname);

/* ------------------------------------------------------------ */

int mkdir(const char *pathname);
int rmdir(const char *pathname);
struct dir *opendir(const char *pathname);
struct dirent *readdir(struct dir *dir);
void closedir(struct dir *dir);

/* ------------------------------------------------------------ */

struct dir *openpattern(const char *pathname);
struct dirent *readpattern(struct dir *dir);
void closepattern(struct dir *dir);

/* ------------------------------------------------------------ */

int unlink(const char *pathname);
struct file *openfile(const char *pathname, const char *mode);
size_t readfile(void *block, size_t size, struct file *file);
char getc(struct file *file);
size_t writefile(const void *block, size_t size, struct file *file);
int puts(const char *str, struct file *file);
void closefile(struct file *file);
int eof(struct file *file);
int seek(struct file *file, long offset, int origin);
int seek(struct file *file, long long offset, int origin);
int rewind(struct file *file);
int stat(struct file *file, struct info *info);
int flush(struct file *file);
long int tell(struct file *file);
int error(struct file *file);
void clearerr(struct file *file);
void flushtofile(const void *block, size_t size, struct file *file);

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

istream *openistream(const char *pathname);
ostream *openostream(const char *pathname);
void closeistream(istream *ist);
void closeostream(ostream *ost);
long tellistream(istream *ist);
long tellostream(ostream *ost);

} // namespace io

#endif
