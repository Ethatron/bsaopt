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

#include "globals.h"

/* ---------------------------------------------------- */

int processedfiles = 0;

/* ---------------------------------------------------- */

bool optimizequick = false;
bool dropextras = false;
bool skipbroken = false;
bool skiphashcheck = false;
bool skipexisting = false;
bool skipnewer = false;
bool skipprocessing = false;
bool skipoptimized = true;
bool processhidden = false;
bool passthrough = false;
bool simulation = false;
bool verbose = false;
bool critical = false;

char tempbuf[1024];
char nodebarchar = '-';
string nodebarref;

FILE *repfile = NULL;
FILE *logfile = NULL;

/*
 * Find the first occurrence of find in s, ignore case.
 */
char *stristr(const char *s, const char *find) {
  char c, sc;
  size_t len;

  if ((c = *find++) != 0) {
    c = tolower((unsigned char)c);
    len = strlen(find);
    do {
      do {
	if ((sc = *s++) == 0)
	  return (NULL);
      } while ((char)tolower((unsigned char)sc) != c);
    } while (strnicmp(s, find, len) != 0);

    s--;
  }

  return ((char *)s);
}

/* ---------------------------------------------------------------------------------------------- */

#include "io/io.h"
#include "io/zlib-1.2.3/zlib.h"

#ifdef _IO_SERVER
bool BSAioBegin(bool bsaioserver);
void BSAioEnd(bool bsaioserver);
#else
bool bsaioserver = false;
#endif

void process(const char *, const char *);

extern bool parse_gui();

void prolog(bool io) {
  if (io)
    io::init();

#ifdef _IO_SERVER
  if (!skipprocessing || bsaioserver) {
    if ((bsaioserver) && (BSAioBegin(bsaioserver) != true))
      exit(0);
  }
#endif
}

void epilog(bool io) {
#ifdef _IO_SERVER
  if (!skipprocessing) {
    if ((bsaioserver))
      BSAioEnd(bsaioserver);
  }
#endif

  if (io)
    io::exit();
  
#ifdef _IO_SERVER
  if (bsaioserver)
    exit(0);
#endif
}

/* ---------------------------------------------------------------------------------------------- */

char *infile = NULL;
char *outfile = NULL;

static char option_usage[] = "Options: \n\
-optimizequick    optimize faster but not so well, applies to level=10\n\
\n\
-dropextras       drop extra files from the hierarchy (thumbs.db, .psd, etc.)\n\
-skiphashcheck    don't verify BSA hashes, recover contents\n\
\n\
-compress <level> when writing BSA, compression-strength +- <0-9,10>\n\
-compressimages   any image will be converted to DXT-DDSs, tex-paths adjusted\n\
-skipexisting     skip if the destination file exist\n\
-skipnewer        skip if the destination file (or BSA) is newer\n\
-skipprocessing   skip any of the NIF i/o, just copy and report\n\
-processhidden    process hidden files and directories instead of skipping\n\
-passthrough      copy broken and every file which is not a NIF as well\n\
-game <mw|ob|sk|fo> indicate the gameversion being Morrowind (mw),\n\
                  Oblivion (ob) or Fallout (fo)/Skyrim (sk)\n\
-simulate         operate in read-only mode, nothing will be written\n\
-verbose          output more information\n\
-criticals	  output only critical error related information\n\
\n\
-copy             copy only preset, never change or compress anything\n\
-deployment       deployment preset, everything maxed out\n\
\n\
Some explanations:\n\
\n\
The tool runs on files, archives and directories, with and without outputs.\n\
\n\
When creating or modifying archives it will utilize the given strength of\n\
compression ranking from 0-9 as for the equivalent option in the zlib. You can\n\
also utilize 10 in which case a 7z-based deflater brute-force searches the\n\
parameters for best compression. The tool normally re-considers compressing\n\
files like mp3s and videos if they are not really well compressed. If you\n\
supply negative levels (-1 to -10) you can force it to compress always.\n\
";

static void usage(char *progname, bool isconsole) {
  RedirectIOToConsole();

  cerr << "Usage:" << endl;
  cerr << progname << " [options] in [out]" << endl;
  cerr << option_usage << endl;
  
  if (!isconsole) {
    char key;
    cerr << "\n";
    cerr << "Press enter to close the window ...";
    cin.get(key);
  }

  exit(1);
}

bool parseCommandline(int argc, char *argv[], bool &isconsole) {
  int i;

  infile  = NULL;
  outfile = NULL;

  for (i = 1; i < argc; i++) {
    /**/ if (!strcmp(argv[i], "-gui"))
      return false;//parse_gui
    else if (!strcmp(argv[i], "-console"))
      isconsole = true;
    else if (!strcmp(argv[i], "-optimizequick"))
      optimizequick = true;
    else if (!strcmp(argv[i], "-dropextras"))
      dropextras = true;
    else if (!strcmp(argv[i], "-skiphashcheck"))
      skiphashcheck = true;
    else if (!strcmp(argv[i], "-skipexisting"))
      skipexisting = true;
    else if (!strcmp(argv[i], "-skipnewer"))
      skipnewer = true;
    else if (!strcmp(argv[i], "-skipprocessing"))
      skipprocessing = true;
    else if (!strcmp(argv[i], "-processhidden"))
      processhidden = true;
    else if (!strcmp(argv[i], "-passthrough"))
      passthrough = true;
    else if (!strcmp(argv[i], "-verbose"))
      verbose = true;
    else if (!strcmp(argv[i], "-criticals"))
      critical = true;
    else if (!strcmp(argv[i], "-compress"))
      compresslevel = atoi(argv[++i]);
    else if (!strcmp(argv[i], "-simulate"))
      simulation = true;
    else if (!strcmp(argv[i], "-game")) {
      ++i;
      /**/ if (!strcmp(argv[i], "mw"))
	gameversion = MW_BSAHEADER_VERSION;
      else if (!strcmp(argv[i], "ob"))
	gameversion = OB_BSAHEADER_VERSION;
      else if (!strcmp(argv[i], "fo"))
	gameversion = F3_BSAHEADER_VERSION;
      else if (!strcmp(argv[i], "sk"))
	gameversion = SK_BSAHEADER_VERSION;
    }
    else if (!strcmp(argv[i], "-deployment"))
      optimizequick = false,
      skiphashcheck = true,
      passthrough = true,
      verbose = true,
      skipprocessing = false,
      compresslevel = 10;
    else if (!strcmp(argv[i], "-copy"))
      optimizequick = false,
      skiphashcheck = false,
      passthrough = true,
      verbose = true,
      skipprocessing = true,
      compresslevel = 0;
    else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help"))
      usage(argv[0], isconsole);
#ifdef _IO_SERVER
    else if (!strcmp(argv[i], "-bsaserver"))
      bsaioserver = true;
#endif
    else if (!infile)
      infile = argv[i];
    else if (!outfile)
      outfile = argv[i];
    else
      usage(argv[0], isconsole);
  }

  if (compresslevel < 0)
    compresslevel = -compresslevel, thresholdbsa = false;
  if (compresslevel == 0)
    compressbsa = false;
  if (compresslevel == (Z_BEST_COMPRESSION + 1))
    compresslevel = 9, srchbestbsa = true;

  if (!infile && isconsole && !bsaioserver)
    usage(argv[0], isconsole);

  /* normalize */
  char *rpl;

  if (infile) {
    while ((rpl = strchr(infile, '/')))
      *rpl = '\\';
    rpl = infile + strlen(infile);
    while (*--rpl == '\\')
      *rpl = '\0';
  }

  if (outfile) {
    while ((rpl = strchr(outfile, '/')))
      *rpl = '\\';
    rpl = outfile + strlen(outfile);
    while (*--rpl == '\\')
      *rpl = '\0';
  }

  return !!outfile || bsaioserver;
}

void summary(FILE *out, const char *ou, int processedfiles) {
  if (out) {
    fprintf(out, "\n\nFinal report \"%s\":\n", ou);
    fprintf(out, " processed files: %d\n", processedfiles);

    if (processedoubytes || processedinbytes)
      fprintf(out, " i/o delta: %d bytes\n", processedoubytes - processedinbytes);
    if (convertedinbytes || convertedoubytes)
      fprintf(out, " tex delta: %d bytes\n", convertedoubytes - convertedinbytes);
    if (compressedinbytes || compressedoubytes)
      fprintf(out, " z delta: %d bytes\n", compressedoubytes - compressedinbytes);
    if (virtualbsabytes || virtualbsafiles)
      fprintf(out, " bsa virtuals: -%d bytes (%d files)\n", virtualbsabytes, virtualbsafiles);

    if (verbose) {
      if (zerobytes.size() > 0) {
	fprintf(out, "\nzero byte files:\n");
	for (size_t n = 0; n < zerobytes.size(); n++)
	  fprintf(out, "%s\n", zerobytes[n].data());
      }

      if (duplicates.size() > 0) {
	fprintf(out, "\nduplicate files:\n");
	map<string, string>::iterator n;
	for (n = duplicates.begin(); n != duplicates.end(); n++)
	  fprintf(out, "%s == %s\n", n->first.data(), n->second.data());
      }

    }

    fflush(out);
  }
}

int main(int argc,char **argv) {
  bool isc = true;

  repfile = stderr;
  logfile = NULL;

  parseCommandline(argc, argv, isc);

  repfile = stderr;
  logfile = (verbose ? stdout : NULL);

  prolog();

  if (infile) {
#ifdef	NDEBUG
    try {
#endif
      /* go ahead */
      process(infile, outfile);

      fflush(repfile);
      fflush(logfile);

      /* summary */
      summary(repfile, outfile, processedfiles);
      summary(logfile, outfile, processedfiles);
#ifdef	NDEBUG
    }
    catch(exception &e) {
      fprintf(repfile, "fatal error: %s\n", e.what());
    }
#endif
  }

  epilog();

  return 0;
}
