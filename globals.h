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

#ifndef GLOBALS_H
#define GLOBALS_H

#define	_CRT_SECURE_NO_WARNINGS
#define	_CRT_NONSTDC_NO_DEPRECATE

#include <stdio.h>
#include <stdint.h>
#include <string>

using namespace std;

/* ---------------------------------------------------- */

#if 0
void barprintf(FILE *f, char fill, const char *format, ...) {
  va_list args;
  va_start(args, format);

  if (!critical) {
    char buf[1024];
    char fll[1024];
    size_t len;

    memset(fll, fill, 1024);
    sprintf(buf, format, args);
    len = strlen(buf);
    fll[(80 - 4) - len] = '\0';
    fprintf(f, "\n%c %s %s\n", fill, buf, fll);

    fflush(stdout);
  }
}

void nfoprintf(FILE *f, const char *format, ...) {
  va_list args;
  va_start(args, format);

  if (!critical) {
    char buf[1024];
    sprintf(buf, format, args);
    fprintf(f, format, args);
  }
}

void errprintf(FILE *f, const char *format, ...) {
  va_list args;
  va_start(args, format);

  if (1) {
    char buf[1024];
    sprintf(buf, format, args);
    fprintf(f, format, args);
  }
}
#elif 0
#define barprintf(f, fill, format, ...) {		\
  if (!critical) {					\
  char buf[1024];					\
  char fll[1024];					\
  size_t len;						\
  \
  memset(fll, fill, 1024);				\
  sprintf(buf, format, ## __VA_ARGS__);		\
  len = strlen(buf);					\
  fll[(80 - 4) - len] = '\0';				\
  fprintf(f, "\n%c %s %s\n", fill, buf, fll);		\
  \
  fflush(stdout);					\
  }							\
}

#define nfoprintf(f, format, ...) { if (!critical) { fprintf(f, format, ## __VA_ARGS__); } }
#define errprintf(f, format, ...) { if (1        ) { fprintf(f, format, ## __VA_ARGS__); } }
#endif

#define barprintf(f, fill, format, ...) {		\
  if (!critical && logfile) {				\
  char buf[1024];					\
  char fll[1024];					\
  size_t len;						\
  \
  memset(fll, fill, 1024);				\
  sprintf(buf, format, ## __VA_ARGS__);		\
  len = strlen(buf);					\
  fll[(80 - 4) - len] = '\0';				\
  \
  fprintf(logfile, "\n%c %s %s\n", fill, buf, fll);	\
  fflush(logfile);					\
  }							\
}

#define nfoprintf(f, format, ...) { if (!critical && logfile) { fprintf(logfile, format, ## __VA_ARGS__); fflush(logfile); } }
#define errprintf(f, format, ...) { if (1         && logfile) { fprintf(logfile, format, ## __VA_ARGS__); fflush(logfile); } }
#define     logrf(   format, ...) { if (0         && repfile) { fprintf(repfile, format, ## __VA_ARGS__); fflush(repfile); } }

extern FILE *repfile;
extern FILE *logfile;
extern char rerror[256];

#define addnote(format, ...) sprintf(tempbuf, format, ## __VA_ARGS__), notes.push_back(tempbuf)
#define adderrr(format, ...) sprintf(tempbuf, format, ## __VA_ARGS__), errrs.push_back(tempbuf)
#define remnote(           )                                           notes.pop_back()
#define remerrr(           )                                           errrs.pop_back()

/* ---------------------------------------------------- */

extern char tempbuf[1024];

extern char nodebarchar;
extern string nodebarref;

/* ---------------------------------------------------- */

extern int processedfiles;

/* ---------------------------------------------------- */

extern bool optimizequick;
extern bool dropextras;
extern bool skipbroken;
extern bool skiphashcheck;
extern bool skipexisting;
extern bool skipnewer;
extern bool skipprocessing;
extern bool skipoptimized;
extern bool processhidden;
extern bool passthrough;
extern bool simulation;
extern bool verbose;
extern bool critical;

#include "io/io.h"
#include "io/bsa.h"
#include "io/zip.h"
#include "process/process.h"

char *stristr(const char *s, const char *find);

/* ---------------------------------------------------- */

#define	DATAMINING

extern char *infile;
extern char *outfile;

void prolog(bool io = true);
void epilog(bool io = true);
bool parseCommandline(int argc, char *argv[], bool &isconsole);
void summary(FILE *out, const char *ou, int processedfiles);
bool RedirectIOToConsole();
int main(int argc,char **argv);

/* ---------------------------------------------------- */

void InitProgress(const char *patterna, const char *patternb, int dne, int rng);
void SetProgress(const char *str, int dne);
void SetProgress(size_t din, size_t dou);
void SetTopic(const char *topic);
void SetReport(const char *status, size_t din, size_t dou);
void SetReport(const char *status, size_t din, size_t dou, int dpl);
bool RequestFeedback(const char *question);

#endif
