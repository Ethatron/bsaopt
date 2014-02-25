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

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <sys/types.h>

#include "../globals.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

vector<string> notes;
vector<string> errrs;
vector<string> zerobytes;
map<string, string> duplicates;

/* ---------------------------------------------------- */

size_t processedinbytes = 0;
size_t processedoubytes = 0;
size_t compressedinbytes = 0;
size_t compresseddtbytes = 0;
size_t compressedoubytes = 0;
size_t convertedinbytes = 0;
size_t convertedoubytes = 0;
ptrdiff_t deltaiobytes = 0;
int virtualbsafiles = 0;
int virtualbsabytes = 0;

#include "../io/io.h"

/* ---------------------------------------------------------------------------------------------- */

void process(const char *inname, const char *ouname) {
  size_t ilen = (inname ? strlen(inname) : 0);
  size_t olen = (ouname ? strlen(ouname) : ilen);

  bool rm = false;
  struct io::info iinfo, oinfo;
  if (!io::stat(inname, &iinfo)) {
    /* input: directory */
    if (iinfo.io_type & IO_DIRECTORY) {
      /* don't create output in simulation-mode */
      if (ouname && !simulation && io::stat(ouname, &oinfo)) {
	rm = true;

	if (io::mkdir(ouname)) {
	  fprintf(stderr, "can't create the directory \"%s\"\n", ouname);
	  return;
	}

	if (io::stat(ouname, &oinfo)) {
	  fprintf(stderr, "can't find the directory \"%s\"\n", ouname);
	  return;
	}
      }

      if (simulation || (oinfo.io_type & IO_DIRECTORY)) {
	struct io::dir *dir;

	if ((dir = io::opendir(inname))) {
	  struct io::dirent *et;

	  while ((et = io::readdir(dir))) {
	    if (!strcmp(et->name, ".") ||
		!strcmp(et->name, ".."))
	      continue;

	    char *ninname = (char *)malloc(ilen + 1 + et->namelength + 1);
	    char *nouname = (char *)malloc(olen + 1 + et->namelength + 1);

	    strcpy(ninname, inname);
	    strcpy(nouname, ouname ? ouname : inname);

	    strcat(ninname, "\\");
	    strcat(nouname, "\\");

	    strcat(ninname, et->name);
	    strcat(nouname, et->name);

	    process(ninname, nouname);

	    free(ninname);
	    free(nouname);
	  }

	  io::closedir(dir);

	  /* don't create output in simulation-mode */
	  if (ouname && !simulation && (dir = io::opendir(ouname))) {
	    struct io::dirent *et;
	    int num = 0;

	    while ((et = io::readdir(dir))) {
	      if (!strcmp(et->name, ".") ||
		  !strcmp(et->name, ".."))
		continue;

	      /* ioopen/fopen is synchronous, only close is
	       * asynchronous, which means even non-yet-written
	       * files exist in the folder and we don't nee to wait for
	       * completition to check if a folder is empty/non-empty
	       *
	       * no lock/synchronization necessary
	       */
	      num++;
	    }

	    io::closedir(dir);

	    if (!num && rm)
	      io::rmdir(ouname);
	  }
	}
	else
	  fprintf(stderr, "can't open the directory \"%s\"\n", inname);
      }
      else
	fprintf(stderr, "destination isn't a directory \"%s\"\n", ouname);
    }
    /* input: file */
    else {
      const char *fle;
      struct io::dir *dir;
      bool dealloc = false;
      bool docopy = true;
      bool iszero = !iinfo.io_size;

      /* strip trailing slash */
      if (!(fle = strrchr(inname, '/')))
	if (!(fle = strrchr(inname, '\\')))
	  fle = inname - 1;

      fle += 1;
      if (stristr(fle, "thumbs.db"))
	return;
      if (dropextras) {
	if (isext(fle, "psd"))
	  docopy = false;

	if (!docopy)
	  fprintf(stderr, "filtered \"%s\"\n", fle);
      }

      /* if there is no destination, clone the source */
      if (ouname && !io::stat(ouname, &oinfo)) {
	if (oinfo.io_type & IO_DIRECTORY) {
	  char *nouname = (char *)malloc(olen + 1 + strlen(fle) + 1);

	  /* if we push a single specific file into a BSA
	   * we have to open/close it to flush the BSA to disk
	   */
	  dir = io::opendir(ouname);

	  strcpy(nouname, ouname);
	  strcat(nouname, "\\");
	  strcat(nouname, fle);

	  ouname = nouname;
	  dealloc = true;
	}
	else if (skipexisting)
	  return;
	else if (skipnewer)
	  if (iinfo.io_time <= oinfo.io_time)
	    return;
      }

      /* global statistics (even for multiple files in one pass) */
      deltaiobytes = 0;

      /* passthrough-mode, don't create output in simulation-mode */
      if (docopy && passthrough && ouname && !simulation) {
	/* action required */
	if (stricmp(inname, ouname)) {
	  fprintf(stderr, "copying \"%s\"\n", fle);

	  /* TODO: nowait asynchronous */
	  io::cp(inname, ouname);
	}

	if (iszero)
	  zerobytes.push_back(inname);
      }
      else {
	if (deltaiobytes)
	  nfoprintf(stdout, "delta:            %d bytes %s\n", (deltaiobytes > 0) ? deltaiobytes : -deltaiobytes, (deltaiobytes > 0) ? "less" : "more");
/*	else
	  nfoprintf(stdout, "delta:            unchanged\n");	*/
      }

      if (dealloc) {
	io::closedir(dir);
	free((void *)ouname);
      }
    }
  }
}