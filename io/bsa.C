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

#define EXTRA_VERIFICATION
#define REMOVE_DOUBLES
#define	DEPEXT_ZLIB
#define	DEPEXT_7ZIP
#include "depext.C"

int compresslevel = Z_BEST_SPEED;
bool compressbsa = true;
bool srchbestbsa = false;
bool thresholdbsa = true;
int gameversion = -1;
char rerror[256];

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

#include <assert.h>

/* Default header data */
#define MW_BSAHEADER_FILEID  0x00000100 //!< Magic for Morrowind BSA
#define OB_BSAHEADER_FILEID  0x00415342 //!< Magic for Oblivion BSA, the literal string "BSA\0".
#define MW_BSAHEADER_VERSION 0x00 //!< Version number of an Morrowind BSA
#define OB_BSAHEADER_VERSION 0x67 //!< Version number of an Oblivion BSA
#define F3_BSAHEADER_VERSION 0x68 //!< Version number of a Fallout 3 BSA
#define SK_BSAHEADER_VERSION 0x68 //!< Version number of a Skyrim BSA
#define OX_BSAHEADER_VERSION -OB_BSAHEADER_VERSION //!< Version number of an Oblivion BSA (XBox)
#define FX_BSAHEADER_VERSION -F3_BSAHEADER_VERSION //!< Version number of a Fallout 3 BSA (XBox)
#define SX_BSAHEADER_VERSION -SK_BSAHEADER_VERSION //!< Version number of a Skyrim BSA (XBox)

/* Archive flags */
#define OB_BSAARCHIVE_PATHNAMES           0x0001 //!< Whether the BSA has names for paths
#define OB_BSAARCHIVE_FILENAMES           0x0002 //!< Whether the BSA has names for files
#define OB_BSAARCHIVE_COMPRESSFILES       0x0004 //!< Whether the files are compressed
#define SK_BSAARCHIVE_BIGENDIAN		  0x0040 //!< Whether the archive is in big endian
#define F3_BSAARCHIVE_PREFIXFULLFILENAMES 0x0100 //!< Whether the name is prefixed to the data?
#define SK_BSAARCHIVE_PREFIXFULLFILENAMES 0x0100 //!< Whether the name is prefixed to the data?
#define SK_BSAARCHIVE_XMEMCODEC		  0x0204 //!< Whether the archive is for XMem

/* Content flags */
#define OB_BSACONTENT_MESHES	0x0001 //!< Set when the BSA contains meshes
#define OB_BSACONTENT_TEXTURES  0x0002 //!< Set when the BSA contains textures
#define OB_BSACONTENT_MENUS	0x0004 //!< Set when the BSA contains menus
#define OB_BSACONTENT_SOUNDS	0x0008 //!< Set when the BSA contains sounds
#define OB_BSACONTENT_VOICES	0x0010 //!< Set when the BSA contains voices
#define OB_BSACONTENT_SHADERS	0x0020 //!< Set when the BSA contains shaders
#define OB_BSACONTENT_TREES	0x0040 //!< Set when the BSA contains trees
#define OB_BSACONTENT_FONTS     0x0080 //!< Set when the BSA contains fonts
#define OB_BSACONTENT_MISC	0x0100 //!< Set when the BSA contains misc

/* File flags */
#define OB_BSAFILE_NIF  0x0001 //!< Set when the BSA contains NIF files
#define OB_BSAFILE_DDS  0x0002 //!< Set when the BSA contains DDS files
#define OB_BSAFILE_XML  0x0004 //!< Set when the BSA contains XML files
#define OB_BSAFILE_WAV  0x0008 //!< Set when the BSA contains WAV files
#define OB_BSAFILE_MP3  0x0010 //!< Set when the BSA contains MP3 files
#define OB_BSAFILE_XWM  0x0010 //!< Set when the BSA contains XWM files
#define OB_BSAFILE_TXT  0x0020 //!< Set when the BSA contains TXT files
#define OB_BSAFILE_HTML 0x0020 //!< Set when the BSA contains HTML files
#define OB_BSAFILE_BAT  0x0020 //!< Set when the BSA contains BAT files
#define OB_BSAFILE_SCC  0x0020 //!< Set when the BSA contains SCC files
#define OB_BSAFILE_SPT  0x0040 //!< Set when the BSA contains SPT files
#define OB_BSAFILE_TEX  0x0080 //!< Set when the BSA contains TEX files
#define OB_BSAFILE_FNT  0x0080 //!< Set when the BSA contains FNT files
#define OB_BSAFILE_CTL  0x0100 //!< Set when the BSA contains CTL files

#define OB_BSAFILE_LIP  0x0200 //!< Set when the BSA contains LIP files
#define OB_BSAFILE_FUZ  0x0200 //!< Set when the BSA contains FUZ files
#define OB_BSAFILE_BIK  0x0400 //!< Set when the BSA contains BIK files
#define OB_BSAFILE_JPG  0x0800 //!< Set when the BSA contains JPG files
#define OB_BSAFILE_OGG	0x1000 //!< Set when the BSA contains OGG files
#define OB_BSAFILE_GID	0x2000 //!< Set when the BSA contains GID files
#define OB_BSAFILE_PEX	0x2000 //!< Set when the BSA contains PEX files

#define OB_BSAFILE_MASK 0x01FF //!< native types
#define OB_BSAFILE_THRESH (OB_BSAFILE_OGG | OB_BSAFILE_MP3 | OB_BSAFILE_XWM | OB_BSAFILE_LIP | OB_BSAFILE_FUZ | OB_BSAFILE_BIK | OB_BSAFILE_JPG)
#define OB_BSAFILE_EXLUDE (OB_BSAFILE_OGG)

/* Bitmasks for the size field in the header */
#define OB_BSAFILE_SIZEMASK 0x3fffffff //!< Bit mask with OBBSAFileInfo::sizeFlags to get the size of the file

/* Record flags */
#define OB_BSAFILE_FLAG_ALLFLAGS  0xC0000000 //!< Bit mask with OBBSAFileInfo::sizeFlags to get the compression status
#define OB_BSAFILE_FLAG_COMPRESS  0x40000000 //!< Bit mask with OBBSAFileInfo::sizeFlags to get the compression status

//! The header of a Morrowind BSA.
/*!
 * Follows MW_BSAHEADER_FILEID.
 */
struct MWBSAHeader
{
	unsigned int HashEntryOffset; //!< Offset of beginning of hash entries
	unsigned int FileCount; //!< Total number of file records (MWBSAFileInfo)
};

//! Info for a hash inside an Morrowind BSA
struct MWBSAHashEntry
{
	unsigned __int64 hash; //!< Hash of the filename
};

//! Info for a filename inside an Morrowind BSA
struct MWBSAFilenameOffset
{
	unsigned int offset; //!< Offset to filename data
};

//! Info for a file inside an Morrowind BSA
struct MWBSAFileInfo
{
	unsigned int size; //!< Size of the data
	unsigned int offset; //!< Offset to raw file data
};

//! The header of an Oblivion BSA.
/*!
 * Follows OB_BSAHEADER_FILEID and OB_BSAHEADER_VERSION.
 */
struct OBBSAHeader
{
	unsigned int FolderRecordOffset; //!< Offset of beginning of folder records
	unsigned int ArchiveFlags; //!< Archive flags
	unsigned int FolderCount; //!< Total number of folder records (OBBSAFolderInfo)
	unsigned int FileCount; //!< Total number of file records (OBBSAFileInfo)
	unsigned int FolderNameLength; //!< Total length of folder names
	unsigned int FileNameLength; //!< Total length of file names
	unsigned int FileFlags; //!< File flags
};

//! Info for a folder inside an Oblivion BSA
struct OBBSAFolderInfo
{
	unsigned __int64 hash; //!< Hash of the folder name
	unsigned int fileCount; //!< Number of files in folder
	unsigned int offset; //!< Offset to name of this folder
};

//! Info for a file inside an Oblivion BSA
struct OBBSAFileInfo
{
	unsigned __int64 hash; //!< Hash of the filename
	unsigned int sizeFlags; //!< Size of the data, possibly with OB_BSAFILE_FLAG_COMPRESS set
	unsigned int offset; //!< Offset to raw file data
};

/* ------------------------------------------------------------ */

unsigned __int64 GenMWHashPair(string pth) {
  unsigned __int64 hash = 0;

  unsigned l = (unsigned)(pth.length() >> 1);
  unsigned suml, sumu, off, temp, i, n;
  
  for (suml = off = i = 0; i < l; i++) {
    suml ^= (((unsigned)(pth[i])) << (off & 0x1F));
    off += 8;
  }

  for (sumu = off = 0; i < pth.length(); i++) {
    temp = (((unsigned)(pth[i])) << (off & 0x1F));
    sumu ^= temp;
    n = temp & 0x1F;
    sumu = (sumu << (32 - n)) | (sumu >> n);  // binary "rotate right"
    off += 8;
  }

  hash += sumu;
  hash <<= 32;
  hash += suml;

  return hash;
}

unsigned __int64 GenMWHash(string path, string file) {
  std::transform(file.begin(), file.end(), file.begin(), ::tolower);
  std::replace(file.begin(), file.end(), '/', '\\');

  if (path.length() && file.length())
    return GenMWHashPair(path + "\\" + file);
  else
    return GenMWHashPair(path +        file);
}

/* ------------------------------------------------------------ */

unsigned int GenOBHashStr(string s) {
  unsigned int hash = 0;

  for (size_t i = 0; i < s.length(); i++) {
    hash *= 0x1003F;
    hash += (unsigned char)s[i];
  }

  return hash;
}

unsigned __int64 GenOBHashPair(string fle, string ext) {
  unsigned __int64 hash = 0;

  if (fle.length() > 0) {
    hash = (unsigned __int64)(
      (((unsigned char)fle[fle.length() - 1]) * 0x1) +
      ((fle.length() > 2 ? (unsigned char)fle[fle.length() - 2] : (unsigned char)0) * 0x100) +
      (fle.length() * 0x10000) +
      (((unsigned char)fle[0]) * 0x1000000)
    );

    if (fle.length() > 3) {
      hash += (unsigned __int64)(GenOBHashStr(fle.substr(1, fle.length() - 3)) * 0x100000000);
    }
  }

  if (ext.length() > 0) {
    hash += (unsigned __int64)(GenOBHashStr(ext) * 0x100000000LL);

    unsigned char i = 0;
    if (ext == ".nif") i = 1;
    if (ext == ".kf" ) i = 2;
    if (ext == ".dds") i = 3;
    if (ext == ".wav") i = 4;

    if (i != 0) {
      unsigned char a = (unsigned char)(((i & 0xfc ) << 5) + (unsigned char)((hash & 0xff000000) >> 24));
      unsigned char b = (unsigned char)(((i & 0xfe ) << 6) + (unsigned char)( hash & 0x000000ff)       );
      unsigned char c = (unsigned char)(( i          << 7) + (unsigned char)((hash & 0x0000ff00) >>  8));

      hash -= hash & 0xFF00FFFF;
      hash += (unsigned int)((a << 24) + b + (c << 8));
    }
  }

  return hash;
}

unsigned __int64 GenOBHash(string path, string file) {
  std::transform(file.begin(), file.end(), file.begin(), ::tolower);
  std::replace(file.begin(), file.end(), '/', '\\');

  string fle;
  string ext;

  const char *_fle = file.data();
  const char *_ext = strrchr(_fle, '.');
  if (_ext) {
    ext = file.substr((0 + _ext) - _fle);
    fle = file.substr(0, ( _ext) - _fle);
  }
  else {
    ext = "";
    fle = file;
  }

  if (path.length() && fle.length())
    return GenOBHashPair(path + "\\" + fle, ext);
  else
    return GenOBHashPair(path +        fle, ext);
}

/* ------------------------------------------------------------ */

typedef struct {
  bool operator()(const string &s1, const string &s2) const {
    return stricmp(s1.data(), s2.data()) < 0;
  }
} stringi;

class bsfile  ; typedef	set<bsfile  , stringi> bsfileset;
class bsfolder; typedef	set<bsfolder, stringi> bsfolderset;

class bsfile : public string {

public:
  bsfile() {
    inp = NULL;
    oup = NULL;
  }

  ~bsfile() {
    if (inp) free(inp);
    if (oup) free(oup);
  }

  void clear() {
    memset(&iinfo, 0, sizeof(iinfo));
    memset(&oinfo, 0, sizeof(oinfo));

    inp = NULL; ics = inc = ins = ina = 0;
    oup = NULL; ocs = ouc = ous = oua = 0;
  }

public:
  mutable struct OBBSAFileInfo iinfo;
  mutable struct OBBSAFileInfo oinfo;

  void GenHash(unsigned int version, unsigned int flags = 0) const {
    if (version == MW_BSAHEADER_VERSION) {
//    iinfo.hash =
      oinfo.hash = ::GenMWHash("", *this);
    }
    else {
//    iinfo.hash =
      oinfo.hash = ::GenOBHash("", *this);

      // swap top half
      if (flags & SK_BSAARCHIVE_BIGENDIAN)
//	iinfo.hash =
	oinfo.hash = (oinfo.hash & 0xFFFFFFFF) + ((unsigned __int64)_byteswap_ulong((unsigned long)(oinfo.hash >> 32)) << 32);
    }
  }

public:
  /* ... */
  mutable unsigned int filetype;
  mutable unsigned int inc; mutable unsigned int ics;
  mutable unsigned int ouc; mutable unsigned int ocs;

//mutable unsigned int ia32; mutable unsigned int ic32; mutable unsigned int isha;
//mutable unsigned int oa32; mutable unsigned int oc32; mutable unsigned int osha;

  /* in/out to make read-write to the same file
   * in the same archive possible
   */
  mutable void *inp; mutable unsigned int ins; mutable unsigned int ina;
  mutable void *oup; mutable unsigned int ous; mutable unsigned int oua;
};

class bsfolder : public string {

public:
  void clear() {
    memset(&iinfo, 0, sizeof(iinfo));
    memset(&oinfo, 0, sizeof(oinfo));
  }

public:
  mutable struct OBBSAFolderInfo iinfo;
  mutable struct OBBSAFolderInfo oinfo;

  void GenHash(unsigned int version, unsigned int flags = 0) const {
    if (version == MW_BSAHEADER_VERSION) {
//    iinfo.hash =
      oinfo.hash = ::GenMWHash(*this, "");
    }
    else {
//    iinfo.hash =
      oinfo.hash = ::GenOBHash(*this, "");

      // swap top half
      if (flags & SK_BSAARCHIVE_BIGENDIAN)
//	iinfo.hash =
	oinfo.hash = (oinfo.hash & 0xFFFFFFFF) + ((unsigned __int64)_byteswap_ulong((unsigned long)(oinfo.hash >> 32)) << 32);
    }
  }

  mutable bsfileset files;

public:
  /* ... */
};

unsigned __int64 swp(unsigned __int64 h) { return _byteswap_uint64(h); }
unsigned __int64 rev(unsigned __int64 h) { return (h << 32) + (h >> 32); }

bool   bsfile_comp (const bsfile   *i, const bsfile   *j) { return (    i->oinfo.hash  <     j->oinfo.hash ); }
bool bsfolder_comp (const bsfolder *i, const bsfolder *j) { return (    i->oinfo.hash  <     j->oinfo.hash ); }
bool   bsfile_compr(const bsfile   *i, const bsfile   *j) { return (rev(i->oinfo.hash) < rev(j->oinfo.hash)); }
bool bsfolder_compr(const bsfolder *i, const bsfolder *j) { return (rev(i->oinfo.hash) < rev(j->oinfo.hash)); }
bool   bsfile_comps(const bsfile   *i, const bsfile   *j) { return (swp(i->oinfo.hash) < swp(j->oinfo.hash)); }
bool bsfolder_comps(const bsfolder *i, const bsfolder *j) { return (swp(i->oinfo.hash) < swp(j->oinfo.hash)); }

class bsarchive {

public:
  bsarchive() :
    loaded(false),
    changedhead(false),
    changedbody(false),
    ibsa(NULL),
    obsa(NULL),
    fbsa(NULL),
    mem(NULL),
    cmp(NULL) {}
  ~bsarchive() {
    if (ibsa) { if (ibsa->fle) ibsa->close(ibsa->fle); free(ibsa); }
    if (obsa) { if (obsa->fle) obsa->close(obsa->fle); free(obsa); }
    if (fbsa) { if (fbsa->fle) fbsa->close(fbsa->fle); free(fbsa); }
    if (mem) free(mem);
    if (cmp) free(cmp);
  }

  bool shutdown(const char *message) {
    loaded = false;
    folders.clear();

    if (ibsa) { if (ibsa->fle) ibsa->close(ibsa->fle);                                            free(ibsa); ibsa = NULL; }
    if (obsa) { if (obsa->fle) obsa->close(obsa->fle);                                            free(obsa); obsa = NULL; }
    if (fbsa) { if (fbsa->fle) fbsa->close(fbsa->fle); fbsa->unlink((arcname + ".final").data()); free(fbsa); fbsa = NULL; }

    throw runtime_error(message);
    return true;
  }

public:
  bool loaded, changedhead, changedbody;
  unsigned int magic, version;
//union {
    struct MWBSAHeader mwheader;
    struct OBBSAHeader obheader;
//};
    
  size_t sizehead, sizebody, sizefile;
  IOFL *ibsa;
  IOFL *obsa;
  IOFL *fbsa;

  time_t arctime;
  string arcname;
  bsfolderset folders;

protected:
  void *mem, *cmp;
  size_t prg, bdy;
  int    fls, flc;

  void smallcopy(const unsigned int sze, IOFL *src, IOFL *dst) {
    size_t cnt = sze, blk, rsr, wds;

    /* copy with small buffer */
    do {
      blk = min(cnt, 1024 * 1024);

      /* progress */
 //   fprintf(stderr, "Consolidating BSA-fragments: %d/%d files (%d/%d bytes)\r", fls, flc, prg, bdy);

      if ((rsr = src->read (mem, 1, blk, src->fle)) != blk)
	shutdown("Reading BSA failed!");
      if ((wds = dst->write(mem, 1, blk, dst->fle)) != blk)
	shutdown("Writing BSA failed!");

      prg += blk;
      cnt -= blk;
    } while (cnt != 0);

    /* progress */
    fls++;
  }

  bool smallcompare(const unsigned int sze, IOFL *src, IOFL *dst) {
    size_t cnt = sze, blk, rsr, rds;
    bool diff = false;

    /* compare with small buffer */
    do {
      blk = min(cnt, 1024 * 1024);

      if ((rsr = src->read(mem, 1, blk, src->fle)) != blk)
	shutdown("Reading BSA failed!");
      if ((rds = dst->read(cmp, 1, blk, dst->fle)) != blk)
	shutdown("Reading BSA failed!");

      diff = diff || !!memcmp(mem, cmp, blk);

      cnt -= blk;
    } while (cnt != 0);

    /* rewind to source-location */
    if (src->seek(src->fle, -((long)sze), SEEK_CUR))
      shutdown("Seeking BSA failed!");
    /* rewind to destination-location */
    if (dst->seek(dst->fle, -((long)sze), SEEK_CUR))
      shutdown("Seeking BSA failed!");

    return diff;
  }

  unsigned int smalladler(const unsigned int sze, IOFL *src) {
    size_t cnt = sze, blk, rsr;
    unsigned int adler = adler32(0,0,0);

    /* compare with small buffer */
    do {
      blk = min(cnt, 1024 * 1024);

      if ((rsr = src->read(mem, 1, blk, src->fle)) != blk)
	shutdown("Reading BSA failed!");

      adler = adler32(adler, (const Bytef *)mem, (uInt)blk);

      cnt -= blk;
    } while (cnt != 0);

    /* rewind to source-location */
    if (src->seek(src->fle, -((long)sze), SEEK_CUR))
      shutdown("Seeking BSA failed!");

    return adler;
  }

public:
  bool open(const char *pathname) {
    arcname.assign(pathname);
    folders.clear();

    /* set default header */
    memset(&mwheader, 0, sizeof(mwheader));
    memset(&obheader, 0, sizeof(obheader));

    arctime = 0; struct stat sinfo;
    if (!::stat(pathname, &sinfo))
      arctime = sinfo.st_ctime;

    /* open for reading */
    if ((ibsa = iopen(pathname, "rb"))->fle) {
      if (ibsa->read(&magic, 1, sizeof(magic), ibsa->fle) != sizeof(magic))
	return shutdown("Can't read from the BSA!");

      if (magic == MW_BSAHEADER_FILEID) {
	version = MW_BSAHEADER_VERSION; {
	  if (ibsa->read(&mwheader, 1, sizeof(mwheader), ibsa->fle) != sizeof(mwheader))
	    return shutdown("Can't read from the BSA!");
	  if (ibsa->seek(ibsa->fle, 0L, SEEK_END))
	    return shutdown("Can't read from the BSA!");
	  
	  if (gameversion == -1)
	    gameversion = (int)version;

	  sizefile = ibsa->tell(ibsa->fle);

	  size_t FolderFileBlob =
		mwheader.FileCount * sizeof(MWBSAFileInfo);
	  size_t FileNameDirectoryBlob =
		mwheader.FileCount * sizeof(MWBSAFilenameOffset);
	  size_t FileNameBlob =
		mwheader.HashEntryOffset -
		FileNameDirectoryBlob -
		FolderFileBlob;
	  size_t EndOfDirectory =
		mwheader.FileCount * sizeof(MWBSAHashEntry);

	  /* premature end-of-file */
	  if ((mwheader.HashEntryOffset + EndOfDirectory) > sizefile)
	    return (loaded = true);

	  ibsa->seek(ibsa->fle, sizeof(magic) + sizeof(mwheader), SEEK_SET);

	  // file records
	  vector<struct MWBSAFileInfo> finfos(mwheader.FileCount);
//	  ibsa->seek(bsa,
//		sizeof(mwheader),
//	        SEEK_SET);
	  ibsa->read(&finfos[0], sizeof(struct MWBSAFileInfo),
	        mwheader.FileCount, ibsa->fle);

	  // filename offsets
	  vector<struct MWBSAFilenameOffset> foffs(mwheader.FileCount);
//	  ibsa->seek(bsa,
//		sizeof(mwheader) + FolderFileBlob,
//	        SEEK_SET);
	  ibsa->read(&foffs[0], sizeof(struct MWBSAFilenameOffset),
	        mwheader.FileCount, ibsa->fle);

	  // file names
	  vector<char> dfblob(FileNameBlob);
	  char *dfmix = &dfblob[0];
//	  ibsa->seek(ibsa->fle,
//		header.FolderRecordOffset +
//		header.FolderCount * sizeof(OBBSAFolderInfo) +
//	  	header.FolderCount * sizeof(char) +
//	  	header.FolderNameLength +
//	  	header.FileCount   * sizeof(OBBSAFileInfo),
//	  	SEEK_SET);
	  ibsa->read(&dfblob[0], sizeof(char),
	        FileNameBlob, ibsa->fle);

	  // hash records
	  vector<struct MWBSAHashEntry> fhashes(mwheader.FileCount);
//	  ibsa->seek(bsa,
//		sizeof(mwheader),
//	        SEEK_SET);
	  ibsa->read(&fhashes[0], sizeof(struct MWBSAHashEntry),
	        mwheader.FileCount, ibsa->fle);

	  /* streaming folder/files in */
	  vector<struct MWBSAFileInfo>::iterator df = finfos.begin();
	  vector<struct MWBSAFilenameOffset>::iterator fo = foffs.begin();
	  vector<struct MWBSAHashEntry>::iterator fh = fhashes.begin();
	  for (df = finfos.begin(); df != finfos.end(); ++df, ++fo, ++fh) {
	    bsfolder folder;

	    /* assign known data */
	    folder.assign(dfmix + fo->offset);
	    folder.iinfo.hash = fh->hash;
	    folder.iinfo.fileCount = 1;
	    folder.iinfo.offset = 0;

	    /* clear the rest */
	    memset(&folder.oinfo, 0, sizeof(folder.oinfo));

	    /* man, all lowercase ... */
	    std::transform(folder.begin(), folder.end(), folder.begin(), ::tolower);
	    std::replace(folder.begin(), folder.end(), '/', '\\');
	    folder.GenHash(version);

	    /* verify integrity */
	    if (!skiphashcheck || verbose)
	      if (folder.iinfo.hash != folder.oinfo.hash) {
		sprintf(rerror, "BSA corrupt: Hash for file \"%s\" in \"%s\" is different!\n", dfmix + fo->offset, pathname);
		nfoprintf(stderr, rerror);

		if (!skiphashcheck && !RequestFeedback(rerror))
		  return shutdown("ExitThread");
	      }

	    // split directory/file in two parts
	    const char *_fle = folder.data();
	    const char *_dir = strrchr(_fle, '\\');
	    string npth = "";
	    string nfle = folder;
	    if (_dir) {
	      nfle = folder.substr((1 + _dir) - _fle);
	      npth = folder.substr(0, ( _dir) - _fle);
	    }

	    folder.assign(npth);

	    /* search case-sensitive duplicate */
	    bsfolderset::iterator folderref = folders.end();
	    for (bsfolderset::iterator it = folders.begin(); it != folders.end(); ++it) {
	      if (!stricmp(folder.data(), (*it).data())) {
		folderref = it;
		break;
	      }
	    }

	    /* streaming files in */
	    {
	      bsfile file;

	      /* assign known data */
	      file.assign(nfle);
	      file.iinfo.hash = 0;
	      file.iinfo.sizeFlags = df->size;
	      file.iinfo.offset = df->offset;
	      file.filetype = filetype(&file);

	      /* clear the rest */
	      memset(&file.oinfo, 0, sizeof(file.oinfo));

	      file.ics = file.ocs = 0;
	      file.inc = file.ouc = 0;
	      file.inp = file.oup = 0;
	      file.ins = file.ous = 0;
	      file.ina = file.oua = 0;

	      /* man, all lowercase ... */
	      std::transform(file.begin(), file.end(), file.begin(), ::tolower);
	      std::replace(file.begin(), file.end(), '/', '\\');

	      if (folderref == folders.end())
		folder.files.insert(file);
	      else
		folderref->files.insert(file);
	    }

	    if (folderref == folders.end())
	      folders.insert(folder);
	    else
	      folderref->iinfo.fileCount = (unsigned int)folderref->files.size();
	  }

	  sizehead = ibsa->tell(ibsa->fle);
	  sizebody = sizefile - sizehead;
	}
      }
      else if (magic == OB_BSAHEADER_FILEID) {
	if (ibsa->read(&version, 1, sizeof(version), ibsa->fle) != sizeof(version))
	  return shutdown("Can't read from the BSA!");

	if ((version == OB_BSAHEADER_VERSION) ||
	    (version == SK_BSAHEADER_VERSION)) {
	  if (ibsa->read(&obheader, 1, sizeof(obheader), ibsa->fle) != sizeof(obheader))
	    return shutdown("Can't read from the BSA!");
	  if (ibsa->seek(ibsa->fle, 0L, SEEK_END))
	    return shutdown("Can't read from the BSA!");
	  if ((version == SK_BSAHEADER_VERSION) &&
	     ((obheader.ArchiveFlags & SK_BSAARCHIVE_XMEMCODEC) == SK_BSAARCHIVE_XMEMCODEC))
	    return shutdown("Unsupported BSA! It uses the XMem compression algorithm from the XBox.");
	  
	  if (gameversion == -1) {
	    gameversion = (int)version;

	    /* make it litte/big endian */
	    if ((version == SK_BSAHEADER_VERSION) &&
	        (obheader.ArchiveFlags & SK_BSAARCHIVE_BIGENDIAN))
	      gameversion = -gameversion;
	  }

	  sizefile = ibsa->tell(ibsa->fle);

	  size_t FolderFilePos =
		obheader.FolderRecordOffset +
		obheader.FolderCount * sizeof(OBBSAFolderInfo);
	  size_t FolderFileBlob =
	  	obheader.FolderCount * sizeof(char) +
	  	obheader.FolderNameLength +
		obheader.FileCount   * sizeof(OBBSAFileInfo);
	  size_t EndOfDirectory =
		obheader.FolderCount * sizeof(OBBSAFolderInfo) +
		FolderFileBlob +
		obheader.FileNameLength;
	  size_t FileNamePos =
		EndOfDirectory -
		obheader.FileNameLength;

	  /* premature end-of-file */
	  if ((obheader.FolderRecordOffset + EndOfDirectory) > sizefile)
	    return (loaded = true);

	  ibsa->seek(ibsa->fle, obheader.FolderRecordOffset, SEEK_SET);

	  // folder records
	  vector<struct OBBSAFolderInfo> dinfos(obheader.FolderCount);
//	  ibsa->seek(bsa,
//		header.FolderRecordOffset,
//	        SEEK_SET);
	  ibsa->read(&dinfos[0], sizeof(struct OBBSAFolderInfo),
	        obheader.FolderCount, ibsa->fle);

	  // folder names + file records
	  vector<char> dfblob(FolderFileBlob);
	  char *dfmix = &dfblob[0];
//	  ibsa->seek(bsa,
//		header.FolderRecordOffset +
//	  	header.FolderCount * (1 + sizeof(OBBSAFolderInfo)),
//	        SEEK_SET);
	  ibsa->read(&dfmix[0], sizeof(char),
	        FolderFileBlob, ibsa->fle);

#ifdef	EXTRA_VERIFICATION
	  /* verify the header-offsets */
	  int FolderNameLength = 0;
	  int FolderNameCount = 0;
	  vector<struct OBBSAFolderInfo>::iterator dd = dinfos.begin();
	  for (dd = dinfos.begin(); dd != dinfos.end(); ++dd) {
	    int len = *dfmix++;

	    while (*dfmix++)
	    FolderNameCount += 1;
	    FolderNameCount += 1;
	    FolderNameLength += len;

	    dfmix += dd->fileCount * sizeof(OBBSAFileInfo);
	  }

	  /* in this case the directory isn't reliable, don't do anything and let it try to do what's possible */
	  if (FolderNameLength != FolderNameCount) {
	    if (skipbroken || !RequestFeedback("Sanity-check of the directory-names results negative! You likely identify names in bad shape in the listing and have to unselect them.\nContinue to salvage parts?"))
	      return shutdown("ExitThread");
	  }
	  /* in this case the directory appears quite all right and we can correct the header-position */
	  else if (FolderNameLength != obheader.FolderNameLength) {
	    if (!skipbroken && !RequestFeedback("Sanity-check of the header results negative! You likely identify names in bad shape in the listing and have to unselect them.\nContinue to salvage parts?"))
	      return shutdown("ExitThread");

	    /* make the correction */
	    ibsa->seek(ibsa->fle, (long)FolderNameLength - (long)obheader.FolderNameLength, SEEK_CUR);
	  }

	  dfmix = &dfblob[0];
#endif

	  // file names
	  vector<char> fnames(obheader.FileNameLength);
	  char *fname = &fnames[0];
//	  ibsa->seek(ibsa->fle,
//		header.FolderRecordOffset +
//		header.FolderCount * sizeof(OBBSAFolderInfo) +
//	  	header.FolderCount * sizeof(char) +
//	  	header.FolderNameLength +
//	  	header.FileCount   * sizeof(OBBSAFileInfo),
//	  	SEEK_SET);
	  ibsa->read(&fnames[0], sizeof(char),
	        obheader.FileNameLength, ibsa->fle);

	  /* streaming folder in */
	  vector<struct OBBSAFolderInfo>::iterator d = dinfos.begin();
	  for (d = dinfos.begin(); d != dinfos.end(); ++d) {
	    bsfolder folder;

	    /* assign known data */
	    int len = *dfmix++;
	    folder.assign(dfmix);
	    while (*dfmix++);
	    folder.iinfo = *d;

	    /* clear the rest */
	    memset(&folder.oinfo, 0, sizeof(folder.oinfo));

	    /* man, all lowercase ... */
	    std::transform(folder.begin(), folder.end(), folder.begin(), ::tolower);
	    std::replace(folder.begin(), folder.end(), '/', '\\');
	    folder.GenHash(version, obheader.ArchiveFlags);

	    /* verify integrity */
	    if (!skiphashcheck || verbose)
	      if (folder.iinfo.hash != folder.oinfo.hash) {
		sprintf(rerror, "BSA corrupt: Hash for folder \"%s\" in \"%s\" is different!\n", folder.data(), pathname);
		nfoprintf(stderr, rerror);
		if (!skiphashcheck && !RequestFeedback(rerror))
		  return shutdown("ExitThread");
	      }

#if 0
	    /* search case-sensitive duplicate */
	    for (bsfolderset::iterator it = folders.begin(); it != folders.end(); ++it) {
	      if (!stricmp(folder.data(), (*it).data())) {
		folder = *it;
	      }
	    }
#endif

	    /* streaming files in */
	    struct OBBSAFileInfo *f = (struct OBBSAFileInfo *)dfmix;
	    for (unsigned int n = 0; n < d->fileCount; n++, f++) {
	      bsfile file;

	      /* assign known data */
	      file.assign(fname);
	      while (*fname++);
	      file.iinfo = *f;
	      file.filetype = filetype(&file);

	      /* flip the compressed flag (easier to handle on each file individually) */
	      file.iinfo.sizeFlags ^= (obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES ? OB_BSAFILE_FLAG_COMPRESS : 0);

	      /* clear the rest */
	      memset(&file.oinfo, 0, sizeof(file.oinfo));

	      file.ics = file.ocs = 0;
	      file.inc = file.ouc = 0;
	      file.inp = file.oup = 0;
	      file.ins = file.ous = 0;
	      file.ina = file.oua = 0;

	      /* man, all lowercase ... */
	      std::transform(file.begin(), file.end(), file.begin(), ::tolower);
	      std::replace(file.begin(), file.end(), '/', '\\');
	      file.GenHash(version, obheader.ArchiveFlags);

	      /* verify integrity */
	      if (!skiphashcheck || verbose)
		if (file.iinfo.hash != file.oinfo.hash) {
		  sprintf(rerror, "BSA corrupt: Hash for file \"%s\" in \"%s\" is different!\n", file.data(), pathname);
		  nfoprintf(stderr, rerror);
		  if (!skiphashcheck && !RequestFeedback(rerror))
		    return shutdown("ExitThread");
		}

#if 0
	      /* search case-sensitive duplicate */
	      for (bsfileset::iterator it = folder.files.begin(); it != folder.files.end(); ++it) {
		if (!stricmp(file.data(), (*it).data())) {
		  file = *it;
		}
	      }
#endif

	      folder.files.insert(file);
	    }

	    dfmix = (char *)f;

	    folders.insert(folder);
	  }

	  sizehead = ibsa->tell(ibsa->fle);
	  sizebody = sizefile - sizehead;

	  /* remove compressed flag (it's on each individual file now) */
	  obheader.ArchiveFlags &= ~OB_BSAARCHIVE_COMPRESSFILES;
	}
	else
	  return shutdown("File has unsupported version!");
      }
      else
	return shutdown("File is not a supported BSA!");
    }
    else if (gameversion != -1) {
      version = std::abs(gameversion);
    }

    return (loaded = true);
  }

  bool changed() {
    return (changedhead || changedbody);
  }

  bool close() {
//#pragma omp barrier
    if (changed()) {
      if ((fbsa = iopen((arcname + ".final").data(), "wb+"))->fle) {
	SetTopic("Assembling BSA-directory:");

	if (((gameversion == -1) && (version == MW_BSAHEADER_VERSION)) ||
	    ((gameversion == MW_BSAHEADER_VERSION))) {
	  unsigned int _magic, _version;
	  struct MWBSAHeader _header;
	  size_t FileNameBlob = 0;

	  fls = flc = 0;
	  prg = bdy = 0;
	  mem = malloc(1024 * 1024);
	  cmp = malloc(1024 * 1024);
	  
	  _magic   = MW_BSAHEADER_FILEID;
	  _version = MW_BSAHEADER_VERSION;
	  if (gameversion != -1)
	    _version = (unsigned int)std::abs(gameversion);

	  memset(&_header, 0, sizeof(_header));

	  /* walk (for summary) */
	  {
	    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
	      size_t num = (*dt).files.size();
	      if (num > 0) {
		for (bsfileset::iterator ft = (*dt).files.begin(); ft != (*dt).files.end(); ++ft) {
		  /* calculate the required directory-size on the fly */
		  _header.FileCount += 1;

		  /* calculate the required directory-size on the fly */
		  FileNameBlob += (unsigned int)(*dt).length() + 1;
		  FileNameBlob += (unsigned int)(*ft).length() + 1;

		  /* this is in obsa */
		  if ((*ft).oinfo.offset) {
		    bdy += (*ft).oinfo.sizeFlags;
		    flc += 1;
		  }
		  /* this is in ibsa */
		  else if ((*ft).iinfo.offset) {
		    bdy += (*ft).iinfo.sizeFlags;
		    flc += 1;
		  }
		  /* this is a non-zero byte file! */
		  else if ((*ft).oinfo.sizeFlags || (*ft).iinfo.sizeFlags)
		    return shutdown("Lost BSA-File reference!");
		  if (bdy > 0x7FFFFFFF)
		    return shutdown("The BSA would exceed 2GiB!");
		}
	      }
	    }
	  }

	  size_t FolderFileBlob =
		_header.FileCount * sizeof(MWBSAFileInfo);
	  size_t FileNameDirectoryBlob =
		_header.FileCount * sizeof(MWBSAFilenameOffset);
	  size_t EndOfDirectory =
		_header.FileCount * sizeof(MWBSAHashEntry);
	  
	  size_t HashEntryOffset =
		FileNameBlob +
		FileNameDirectoryBlob +
		FolderFileBlob;

	  if ((sizeof(_header) + _header.HashEntryOffset + EndOfDirectory + bdy) > 0x7FFFFFFF)
	    return shutdown("The BSA would exceed 2GiB!");

	  /* set new size */
	  _header.HashEntryOffset = (unsigned int)HashEntryOffset;
	  fbsa->seek(fbsa->fle, sizeof(magic) + sizeof(_header) + _header.HashEntryOffset + EndOfDirectory, SEEK_SET);

	  /* record directory while transfering */
	  vector<struct MWBSAFileInfo> finfos(_header.FileCount);
	  vector<struct MWBSAFilenameOffset> foffs(_header.FileCount);
	  vector<struct MWBSAHashEntry> fhashes(_header.FileCount);
	  vector<char> fnames(FileNameBlob);
	  map<unsigned int, MWBSAFileInfo *> written;
	  map<unsigned int, char *> wrnamed;

	  unsigned int fld = 0;
	  char *fname = &fnames[0], *fend = fname +         FileNameBlob;

	  /* progress */
//	  fprintf(stderr, "Consolidating BSA-fragments: %d/%d files (%d/%d bytes)\r", fls, flc, prg, bdy);

	  SetTopic("Consolidating BSA-fragments:");
	  SetReport("Efficiency: %s to %s bytes (%d duplicates)",
	    processedinbytes,
	    processedinbytes - compresseddtbytes - virtualbsabytes, 0
	  );
	  SetProgress(
	    processedinbytes,
	    processedinbytes - compresseddtbytes - virtualbsabytes
	  );

	  /* walk (for transfer contents) */
	  {
	    vector<bsfile> tf;
	    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
	      for (bsfileset::iterator ft = (*dt).files.begin(); ft != (*dt).files.end(); ++ft) {
		bsfile temp = (*ft);

		if ((*dt).length() && (*ft).length())
		  temp.assign((*dt) + "\\" + (*ft));
		else
		  temp.assign((*dt) +        (*ft));

		temp.GenHash(version);
		tf.push_back(temp);
	      }
	    }
	    
	    vector<const bsfile *> fs;
	    for (vector<bsfile>::iterator ft = tf.begin(); ft != tf.end(); ++ft) 
	      fs.push_back(&(*ft));

	    /* sort files by hash */
	    std::sort(fs.begin(), fs.end(), bsfile_compr);

	    /* transfer the files from source-locations */
	    for (vector<const bsfile *>::iterator ft = fs.begin(); ft != fs.end(); ++ft) {
	      struct MWBSAFileInfo fle, *ref;
	      struct MWBSAFilenameOffset flo;
	      struct MWBSAHashEntry flh;
	      unsigned int sze, pos;
	      unsigned int adler;
	      
	      /* this is in obsa */
	      if ((*ft)->oinfo.offset) {
		sze = (*ft)->oinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS);
		pos = (*ft)->oinfo.offset;

		/* fill file structure */
		flh.hash = (*ft)->oinfo.hash;
		flo.offset = (unsigned int)(fname - &fnames[0]);
		fle.size = (*ft)->oinfo.sizeFlags;
		fle.offset = fbsa->tell(fbsa->fle);

		/* goto source-location */
		if (obsa->seek(obsa->fle, pos, SEEK_SET))
		  return shutdown("Seeking BSA failed!");

		/* revoke individual-compressed flag */
		if ((*ft)->oinfo.sizeFlags & OB_BSAFILE_FLAG_COMPRESS) {
		  // should be impossible!
		  abort();
		}

		/* possible duplicate */
		bool diff = true;
#ifdef	REMOVE_DOUBLES
		if (!(adler = (*ft)->ocs))
		  adler = (*ft)->ocs = smalladler(sze, obsa);
		if ((adler = (*ft)->ocs) && (ref = written[adler])) {
		  /* goto source-location */
		  if (fbsa->seek(fbsa->fle, ref->offset, SEEK_SET))
		    return shutdown("Seeking BSA failed!");

		  /* compare with small buffer */
		  diff = smallcompare(sze, obsa, fbsa);

		  /* goto destination-location */
		  if (fbsa->seek(fbsa->fle, fle.offset, SEEK_SET))
		    return shutdown("Seeking BSA failed!");

		  /* they are equal */
		  if (!diff) {
		    fle.offset = ref->offset;

		    /* it just got smaller */
		    bdy -= sze;

		    /* record what happened */
		    virtualbsabytes += sze;
		    virtualbsafiles += 1;

		    duplicates[(*ft)->data()] = wrnamed[(*ft)->ocs];
		  }
		}
#endif

		/* copy with small buffer */
		if (diff)
		  smallcopy(sze, obsa, fbsa);
	      }
	      /* this is in ibsa */
	      else if ((*ft)->iinfo.offset) {
		sze = (*ft)->iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS);
		pos = (*ft)->iinfo.offset;

		/* fill file structure */
		flh.hash = (*ft)->iinfo.hash;
		flo.offset = (unsigned int)(fname - &fnames[0]);
		fle.size = (*ft)->iinfo.sizeFlags;
		fle.offset = fbsa->tell(fbsa->fle);

		/* goto source-location */
		if (ibsa->seek(ibsa->fle, pos, SEEK_SET))
		  return shutdown("Seeking BSA failed!");
		
		/* revoke individual-compressed flag */
		if ((*ft)->iinfo.sizeFlags & OB_BSAFILE_FLAG_COMPRESS) {
		  // should be impossible!
		  abort();
		}

		/* possible duplicate */
		bool diff = true;
#ifdef	REMOVE_DOUBLES
		if (!(adler = (*ft)->ics))
		  adler = (*ft)->ics = smalladler(sze, ibsa);
		if ((adler = (*ft)->ics) && (ref = written[adler])) {
		  /* goto source-location */
		  if (fbsa->seek(fbsa->fle, ref->offset, SEEK_SET))
		    return shutdown("Seeking BSA failed!");

		  /* compare with small buffer */
		  diff = smallcompare(sze, ibsa, fbsa);

		  /* goto destination-location */
		  if (fbsa->seek(fbsa->fle, fle.offset, SEEK_SET))
		    return shutdown("Seeking BSA failed!");

		  /* they are equal */
		  if (!diff) {
		    fle.offset = ref->offset;

		    /* it just got smaller */
		    bdy -= sze;

		    /* record what happened */
		    virtualbsabytes += sze;
		    virtualbsafiles += 1;

		    duplicates[(*ft)->data()] = wrnamed[(*ft)->ics];
		  }
		}
#endif

		/* copy with small buffer */
		if (diff)
		  smallcopy(sze, ibsa, fbsa);
	      }
	      /* zero-byte files (no in, no out) */
	      else {
		/* fill file structure */
		flh.hash = (*ft)->oinfo.hash;
		flo.offset = (unsigned int)(fname - &fnames[0]);
		fle.size = (*ft)->oinfo.sizeFlags;
		fle.offset = fbsa->tell(fbsa->fle);
	      }

	      /* remember that file? */
	      written[adler] = (MWBSAFileInfo *)&finfos[fld];
	      wrnamed[adler] = (char *)fname;

	      /* assign known data */
	      finfos [fld] = fle;
	      foffs  [fld] = flo;
	      fhashes[fld] = flh;
	      fld += 1;

	      assert(fld <= _header.FileCount);

	      /* assign known file (255+\0) */
	      const char *lname = (*ft)->data();
	      unsigned char nlen = (unsigned char)min((*ft)->length(), 255);
	      memcpy(fname, lname, nlen);
		fname  += nlen;
	      *fname++ = '\0';

	      assert(fname <= fend);
	      SetReport("Efficiency: %s to %s bytes (%d duplicates)",
		processedinbytes,
		processedinbytes - compresseddtbytes - virtualbsabytes, (int)duplicates.size()
	      );
	      SetProgress(
		processedinbytes,
		processedinbytes - compresseddtbytes - virtualbsabytes
	      );
	    }
	  }

	  assert(fname == fend);

	  free(mem); mem = NULL;
	  free(cmp); cmp = NULL;

	  /* progress */
//	  fprintf(stderr, "Finalizing BSA-directory                                                  \r");

	  /* set new size */
	  fbsa->seek(fbsa->fle, 0, SEEK_SET);

	  fbsa->write(&_magic  , 1, sizeof(_magic  ), fbsa->fle);
//	  fbsa->write(&_version, 1, sizeof(_version), fbsa->fle);
	  fbsa->write(&_header , 1, sizeof(_header ), fbsa->fle);
	  
	  fbsa->write(&finfos[0], sizeof(struct MWBSAFileInfo),
	    _header.FileCount   , fbsa->fle);
	  fbsa->write(&foffs[0], sizeof(struct MWBSAFilenameOffset),
	    _header.FileCount   , fbsa->fle);
	  fbsa->write(&fnames[0], sizeof(char),
	    FileNameBlob        , fbsa->fle);
	  fbsa->write(&fhashes[0], sizeof(struct MWBSAHashEntry),
	    _header.FileCount   , fbsa->fle);
	}
	else {
	  unsigned int _magic, _version;
	  struct OBBSAHeader _header;

	  fls = flc = 0;
	  prg = bdy = 0;
	  mem = malloc(1024 * 1024);
	  cmp = malloc(1024 * 1024);

	  _magic   = OB_BSAHEADER_FILEID;
	  _version = OB_BSAHEADER_VERSION;
	  if (gameversion != -1)
	    _version = (unsigned int)std::abs(gameversion);

	  memset(&_header, 0, sizeof(_header));

	  /* take some stuff over */
	  _header.FolderRecordOffset = sizeof(_magic) + sizeof(_version) + sizeof(_header);
	  _header.FileFlags = (obheader.FileFlags & 0xFFFF0000);
	  _header.ArchiveFlags |= OB_BSAARCHIVE_PATHNAMES | OB_BSAARCHIVE_FILENAMES | (compressbsa ? OB_BSAARCHIVE_COMPRESSFILES : 0);
	  _header.ArchiveFlags |= (obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
	  _header.ArchiveFlags |= (obheader.ArchiveFlags & SK_BSAARCHIVE_BIGENDIAN);

	  /* unknown */
	  if (_version == OB_BSAHEADER_VERSION) {
	    _header.ArchiveFlags |= 0x0700;
	    _header.ArchiveFlags &= (~SK_BSAARCHIVE_PREFIXFULLFILENAMES);
	  }
	  
	  /* make it litte/big endian */
	  /**/ if (gameversion == SX_BSAHEADER_VERSION)
	    _header.ArchiveFlags |= ( SK_BSAARCHIVE_BIGENDIAN);
	  else
	    _header.ArchiveFlags &= (~SK_BSAARCHIVE_BIGENDIAN);

	  /* walk (for summary) */
	  {
	    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
	      size_t num = (*dt).files.size();
	      if (num > 0) {
		/* calculate the required directory-size on the fly */
		_header.FolderCount += 1;
		_header.FolderNameLength += (unsigned int)(*dt).length() + 1;

		for (bsfileset::iterator ft = (*dt).files.begin(); ft != (*dt).files.end(); ++ft) {
		  /* calculate the required directory-size on the fly */
		  _header.FileCount += 1;
		  _header.FileNameLength += (unsigned int)(*ft).length() + 1;

		  /* recalculate the filecontent-flags */
		  _header.FileFlags |= ((*ft).filetype & OB_BSAFILE_MASK);

		  /* make a few corrections */
		  if (_version == OB_BSAHEADER_VERSION) {
		    if ((*ft).filetype & (OB_BSAFILE_LIP | OB_BSAFILE_MP3))
		      _header.FileFlags |= (OB_BSAFILE_MP3 | OB_BSAFILE_MP3);
		    if (!(*ft).filetype)
		      _header.FileFlags |= (0);
		  }
		  if (_version == SK_BSAHEADER_VERSION) {
		    if ((*ft).filetype & (OB_BSAFILE_FUZ | OB_BSAFILE_XWM))
		      _header.FileFlags |= (OB_BSAFILE_WAV | OB_BSAFILE_XWM);
		    if (!(*ft).filetype)
		      _header.FileFlags |= (OB_BSACONTENT_MISC);
		  }
		  if (_version == F3_BSAHEADER_VERSION) {
		    if ((*ft).filetype & (OB_BSAFILE_OGG | OB_BSAFILE_MP3))
		      _header.FileFlags |= (OB_BSAFILE_WAV | OB_BSAFILE_MP3);
		    if (!(*ft).filetype)
		      _header.FileFlags |= (OB_BSACONTENT_MISC);
		  }

		  /* check the state-of-the compression */
		  unsigned int zlb = 0;

		  /* this is in obsa */
		  /**/ if ((*ft).oinfo.offset) {
		    zlb  = (*ft).oinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS);
		    bdy += (*ft).oinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS);
		    flc += 1;
		  }
		  /* this is in ibsa */
		  else if ((*ft).iinfo.offset) {
		    zlb  = (*ft).iinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS);
		    bdy += (*ft).iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS);
		    flc += 1;
		  }
		  /* this is a non-zero byte file! */
		  else if ((*ft).oinfo.sizeFlags || (*ft).iinfo.sizeFlags)
		    return shutdown("Lost BSA-File reference!");
		  if (bdy > 0x7FFFFFFF)
		    return shutdown("The BSA would exceed 2GiB!");

		  /* revoke all-compressed flag */
		  _header.ArchiveFlags &= ~(zlb ? 0 : OB_BSAARCHIVE_COMPRESSFILES);
		}
	      }
	    }
	  }

	  size_t FolderFileBlob =
	    _header.FolderCount * sizeof(char) +
	    _header.FolderNameLength +
	    _header.FileCount   * sizeof(OBBSAFileInfo);
	  size_t EndOfDirectory =
	    _header.FolderCount * sizeof(OBBSAFolderInfo) +
		    FolderFileBlob +
	    _header.FileNameLength;

	  if ((_header.FolderRecordOffset + EndOfDirectory + bdy) > 0x7FFFFFFF)
	    return shutdown("The BSA would exceed 2GiB!");

	  /* set new size */
	  fbsa->seek(fbsa->fle, _header.FolderRecordOffset + EndOfDirectory, SEEK_SET);

	  /* record directory while transfering */
	  vector<struct OBBSAFolderInfo> dinfos(_header.FolderCount);
	  vector<char> dfblob(FolderFileBlob);
	  vector<char> fnames(_header.FileNameLength);
	  map<unsigned int, OBBSAFileInfo *> written;
	  map<unsigned int, char *> wrnamed;

	  unsigned int fld = 0;
	  char *dfmix = &dfblob[0], *dend = dfmix +         FolderFileBlob;
	  char *fname = &fnames[0], *fend = fname + _header.FileNameLength;

	  /* progress */
//	  fprintf(stderr, "Consolidating BSA-fragments: %d/%d files (%d/%d bytes)\r", fls, flc, prg, bdy);

	  SetTopic("Consolidating BSA-fragments:");
	  SetReport("Efficiency: %s to %s bytes (%d duplicates)",
	    processedinbytes,
	    processedinbytes - compresseddtbytes - virtualbsabytes, 0
	  );
	  SetProgress(
	    processedinbytes,
	    processedinbytes - compresseddtbytes - virtualbsabytes
	  );

	  /* walk (for transfer contents) */
	  {
	    vector<const bsfolder *> ds;
	    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
	      (*dt).GenHash(version, _header.ArchiveFlags);

	      ds.push_back(&(*dt));
	    }

	    /* sort folders by hash */
	    if (_header.ArchiveFlags & SK_BSAARCHIVE_BIGENDIAN)
	      std::sort(ds.begin(), ds.end(), bsfolder_comps);
	    else
	      std::sort(ds.begin(), ds.end(), bsfolder_comp);

	    for (vector<const bsfolder *>::iterator dt = ds.begin(); dt != ds.end(); ++dt) {
	      vector<const bsfile *> fs;
	      for (bsfileset::iterator ft = (*dt)->files.begin(); ft != (*dt)->files.end(); ++ft) {
		(*ft).GenHash(version, _header.ArchiveFlags);

		fs.push_back(&(*ft));
	      }

	      /* sort files by hash */
	      if (_header.ArchiveFlags & SK_BSAARCHIVE_BIGENDIAN)
		std::sort(fs.begin(), fs.end(), bsfile_comps);
	      else
		std::sort(fs.begin(), fs.end(), bsfile_comp);

	      size_t num = (*dt)->files.size();
	      if (num > 0) {
		struct OBBSAFolderInfo dir;

		/* fill folder structure */
		dir.hash = (*dt)->oinfo.hash;
		dir.fileCount = (unsigned int)num;
		dir.offset =
		  _header.FolderRecordOffset +
		  _header.FolderCount * sizeof(OBBSAFolderInfo) +
		  _header.FileNameLength +
		  (unsigned int)(dfmix - &dfblob[0]);

		/* assign known data */
//		memcpy(dinfos, &dir, sizeof(dir));
//		dinfos += sizeof(dir);
		assert(fld < _header.FolderCount);
		dinfos[fld++] = dir;

		/* assign known data */
		strcpy(dfmix + 1, (*dt)->data());
		unsigned char len = (unsigned char)(*dt)->length();
		*dfmix++ = len + 1;
		 dfmix  += len;
		*dfmix++ = '\0';

		assert(dfmix <= dend);

		/* transfer the files from source-locations */
		for (vector<const bsfile *>::iterator ft = fs.begin(); ft != fs.end(); ++ft) {
		  struct OBBSAFileInfo fle, *ref;
		  unsigned int sze, pos;
		  unsigned int adler;

		  /* this is in obsa */
		  if ((*ft)->oinfo.offset) {
		    sze = (*ft)->oinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS);
		    pos = (*ft)->oinfo.offset;

		    /* fill file structure */
		    fle.hash = (*ft)->oinfo.hash;
		    fle.sizeFlags = (*ft)->oinfo.sizeFlags;
		    fle.offset = fbsa->tell(fbsa->fle);

		    /* goto source-location */
		    if (obsa->seek(obsa->fle, pos, SEEK_SET))
		      return shutdown("Seeking BSA failed!");

		    /* possible duplicate */
		    bool diff = true;
#ifdef	REMOVE_DOUBLES
		    if (!(adler = (*ft)->ocs))
		      adler = (*ft)->ocs = smalladler(sze, obsa);
		    if ((adler = (*ft)->ocs) && (ref = written[adler])) {
		      /* goto source-location */
		      if (fbsa->seek(fbsa->fle, ref->offset, SEEK_SET))
			return shutdown("Seeking BSA failed!");

		      /* compare with small buffer */
		      diff = smallcompare(sze, obsa, fbsa);

		      /* goto destination-location */
		      if (fbsa->seek(fbsa->fle, fle.offset, SEEK_SET))
			return shutdown("Seeking BSA failed!");

		      /* they are equal */
		      if (!diff) {
			fle.offset = ref->offset;

			/* it just got smaller */
			bdy -= sze;

			/* record what happened */
			virtualbsabytes += sze;
			virtualbsafiles += 1;

			duplicates[(*ft)->data()] = wrnamed[(*ft)->ocs];
		      }
		    }
#endif

		    /* copy with small buffer */
		    if (diff)
		      smallcopy(sze, obsa, fbsa);
		  }
		  /* this is in ibsa */
		  else if ((*ft)->iinfo.offset) {
		    sze = (*ft)->iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS);
		    pos = (*ft)->iinfo.offset;

		    /* fill file structure */
		    fle.hash = (*ft)->iinfo.hash;
		    fle.sizeFlags = (*ft)->iinfo.sizeFlags;
		    fle.offset = fbsa->tell(fbsa->fle);

		    /* goto source-location */
		    if (ibsa->seek(ibsa->fle, pos, SEEK_SET))
		      return shutdown("Seeking BSA failed!");

		    /* possible duplicate */
		    bool diff = true;
#ifdef	REMOVE_DOUBLES
		    if (!(adler = (*ft)->ics))
		      adler = (*ft)->ics = smalladler(sze, ibsa);
		    if ((adler = (*ft)->ics) && (ref = written[adler])) {
		      /* goto source-location */
		      if (fbsa->seek(fbsa->fle, ref->offset, SEEK_SET))
			return shutdown("Seeking BSA failed!");

		      /* compare with small buffer */
		      diff = smallcompare(sze, ibsa, fbsa);

		      /* goto destination-location */
		      if (fbsa->seek(fbsa->fle, fle.offset, SEEK_SET))
			return shutdown("Seeking BSA failed!");

		      /* they are equal */
		      if (!diff) {
			fle.offset = ref->offset;

			/* it just got smaller */
			bdy -= sze;

			/* record what happened */
			virtualbsabytes += sze;
			virtualbsafiles += 1;

			duplicates[(*ft)->data()] = wrnamed[(*ft)->ics];
		      }
		    }
#endif

		    /* copy with small buffer */
		    if (diff)
		      smallcopy(sze, ibsa, fbsa);
		  }
		  /* zero-byte files (no in, no out) */
		  else {
		    /* fill file structure */
		    fle.hash = (*ft)->oinfo.hash;
		    fle.sizeFlags = (*ft)->oinfo.sizeFlags;
		    fle.offset = fbsa->tell(fbsa->fle);
		  }

		  /* revoke individual-compressed flag */
		  fle.sizeFlags &= ~(_header.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES ? OB_BSAFILE_FLAG_COMPRESS : 0);

		  /* remember that file? */
		  written[adler] = (OBBSAFileInfo *)dfmix;
		  wrnamed[adler] = (char *)fname;

		  /* assign known data */
		  memcpy(dfmix, &fle, sizeof(fle));
		  dfmix += sizeof(fle);

		  assert(dfmix <= dend);

		  /* assign known file (255+\0) */
		  const char *lname = (*ft)->data();
		  unsigned char nlen = (unsigned char)min((*ft)->length(), 255);
		  memcpy(fname, lname, nlen);
		   fname  += nlen;
		  *fname++ = '\0';

		  assert(fname <= fend);
		  SetReport("Efficiency: %s to %s bytes (%d duplicates)",
		    processedinbytes,
		    processedinbytes - compresseddtbytes - virtualbsabytes, (int)duplicates.size()
		  );
		  SetProgress(
		    processedinbytes,
		    processedinbytes - compresseddtbytes - virtualbsabytes
		  );
		}
	      }
	    }
	  }

	  assert(dfmix == dend);
	  assert(fname == fend);

	  free(mem); mem = NULL;
	  free(cmp); cmp = NULL;

	  /* progress */
//	  fprintf(stderr, "Finalizing BSA-directory                                                  \r");

	  /* set new size */
	  fbsa->seek(fbsa->fle, 0, SEEK_SET);

	  fbsa->write(&_magic  , 1, sizeof(_magic  ), fbsa->fle);
	  fbsa->write(&_version, 1, sizeof(_version), fbsa->fle);
	  fbsa->write(&_header , 1, sizeof(_header ), fbsa->fle);

	  fbsa->write(&dinfos[0], sizeof(struct OBBSAFolderInfo),
	    _header.FolderCount   , fbsa->fle);
	  fbsa->write(&dfblob[0], sizeof(char),
		    FolderFileBlob, fbsa->fle);
	  fbsa->write(&fnames[0], sizeof(char),
	    _header.FileNameLength, fbsa->fle);
	}
	
		       fbsa->close(fbsa->fle);
	if (ibsa->fle) ibsa->close(ibsa->fle);
	if (obsa->fle) obsa->close(obsa->fle);

	/* put the file in the right position */
	fbsa->unlink((arcname).data());
	fbsa->rename((arcname + ".final").data(), (arcname).data());
	
	            free(fbsa); fbsa = NULL;
	if (ibsa) { free(ibsa); ibsa = NULL; }
	if (obsa) { free(obsa); obsa = NULL; }
      }
    }

    if (ibsa) { if (ibsa->fle) ibsa->close(ibsa->fle); free(ibsa); ibsa = NULL; }
    if (obsa) { if (obsa->fle) obsa->close(obsa->fle); free(obsa); obsa = NULL; }

    /* remove remnants */
//  unlink((arcname + ".tmp").data());

    return true;
  }

  int stat(const char *pathname, struct io::info *info) const {
    const char *drewind = pathname;
    int ret = -1;

    /* cache-hit? */
    for (bsfolderset::const_iterator it = folders.begin(); it != folders.end(); ++it) {
      const char *fldname = (*it).data();

      /* if foldername is a subset of pathname */
      pathname = drewind;
      if (pathname == stristr(pathname, fldname)) {
	/* remove folder-prefix */
	pathname += strlen(fldname);
	if ((*pathname) == '\0') {
	  info->io_type = IO_DIRECTORY;
	  info->io_size = 0;
	  info->io_raws = 0;
	  info->io_time = arctime;

	  ret = 0;
	  break;
	}
	else if ((*pathname) == '\\')
	  pathname++;

	/* mask sub-folders */
	if (!strchr(pathname, '\\')) {
	  bsfile sch; sch.assign(pathname);
	  std::transform(sch.begin(), sch.end(), sch.begin(), ::tolower);
	  std::replace(sch.begin(), sch.end(), '/', '\\');

	  bsfileset::iterator ft = (*it).files.find(sch);
	  if (ft != (*it).files.end()) {
	    info->io_type = IO_FILE;
	    info->io_size = size(&(*ft));
	    info->io_raws = raws(&(*ft));
	    info->io_time = arctime;

	    ret = 0;
	    break;
	  }
	}
      }

      /* if pathname is a subset of foldername */
      pathname = drewind;
      if (fldname == stristr(fldname, pathname)) {
	/* remove folder-prefix */
	fldname += strlen(pathname);

	if ((*fldname) == '\\') {
	  info->io_type = IO_DIRECTORY;
	  info->io_size = 0;
	  info->io_raws = 0;
	  info->io_time = arctime;

	  ret = 0;
	  break;
	}
      }
    }

    return ret;
  }

  int stat(const bsfolder *file, struct io::info *info) const {
    info->io_type = IO_DIRECTORY;
    info->io_size = 0;
    info->io_raws = 0;
    info->io_time = arctime;

    return 0;
  }

  int stat(const bsfile *file, struct io::info *info) const {
    info->io_type = IO_FILE;
    info->io_size = size(file);
    info->io_raws = raws(file);
    info->io_time = arctime;

    return 0;
  }

  int mkdir(const char *pathname) {
    int ret = 0;

    /* length is limited to 255 characters (because of the
     * BSTR), minus the \0 character
     */
    if (strlen(pathname) > (255 - 1))
      return -1;

    /* man, all lowercase ... */
    bsfolder sch; sch.assign(pathname);
    std::transform(sch.begin(), sch.end(), sch.begin(), ::tolower);
    std::replace(sch.begin(), sch.end(), '/', '\\');

    /* go find it, or skip it */
    bsfolderset::iterator dt = folders.find(sch);
    if (dt == folders.end()) {
      /* mark the head & possibly body to be changed */
      changedhead = changedhead || true;
      changedbody = changedbody || false;

      folders.insert(sch);
      ret = 0;
    }

    return ret;
  }

  int rmdir(const char *pathname) {
    int ret = -1;

    /* man, all lowercase ... */
    bsfolder sch; sch.assign(pathname);
    std::transform(sch.begin(), sch.end(), sch.begin(), ::tolower);
    std::replace(sch.begin(), sch.end(), '/', '\\');

    /* go find it, or skip it */
    bsfolderset::iterator dt = folders.find(sch);
    if (dt != folders.end()) {
      /* mark the head & possibly body to be changed */
      changedhead = changedhead || true;
      changedbody = changedbody || !!(*dt).files.size();

      folders.erase(*dt);
      ret = 0;
    }

    return ret;
  }
  
  set<string> find(const char *pathname) const {
    set<string> contents;

    /* folder-hit? */
    for (bsfolderset::const_iterator dt = folders.begin(); dt != folders.end(); ++dt) {
      const char *subname = (*dt).data(), *ssn;

      if (subname == stristr(subname, pathname)) {
	/* remove folder-prefix */
	subname += strlen(pathname);

	if ((*subname) == '\0') {
	  /* this folder, contains files */
	  for (bsfileset::const_iterator ft = (*dt).files.begin(); ft != (*dt).files.end(); ++ft)
	    contents.insert((*ft).data());

	  contents.insert(".");
	  contents.insert("..");
	  continue;
	}
	else if ((*subname) == '\\')
	  /* this folder, contains folders */
	  subname++;

	/* mask sub-folders */
	if ((ssn = strchr(subname, '\\'))) {
	  string ss(subname, ssn - subname);
	  contents.insert(ss);
	  continue;
	}

	if ((*subname) != '\0') {
	  contents.insert(subname);
	  continue;
	}
      }
    }

    /* implicit folders */
//  if (contents.size()) {
      contents.insert(".");
      contents.insert("..");
//  }

    return contents;
  }
  
  int rename(const char *srcname, const char *dstname) {
    abort();
  }

  int unlink(const char *pathname) {
    const char *drewind = pathname;

    /* folder-hit? */
    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
      const char *subname = (*dt).data();

      pathname = drewind;
      if (pathname == stristr(pathname, subname)) {
	/* remove folder-prefix */
	pathname += strlen(subname);
	if ((*pathname) == '\0')
	  continue;
	else if ((*pathname) == '\\')
	  pathname++;

	/* mask sub-folders */
	if (strchr(pathname, '\\'))
	  continue;

	/* this folder, contains files */
	bsfile sch; sch.assign(pathname);
	std::transform(sch.begin(), sch.end(), sch.begin(), ::tolower);
	std::replace(sch.begin(), sch.end(), '/', '\\');

	/* go find it, or add it */
	bsfileset::iterator ft = (*dt).files.find(sch);
	if (ft != (*dt).files.end()) {
	  if ((*ft).inp) free((*ft).inp);
	  if ((*ft).oup) free((*ft).oup);
	
	  (*dt).files.erase(*ft);
	  return 0;
	}

	return -1;
      }
    }

    return -1;
  }

  const bsfile *get(const char *pathname) {
    const char *drewind = pathname;

    /* folder-hit? */
    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
      const char *subname = (*dt).data();

      pathname = drewind;
      if (pathname == stristr(pathname, subname)) {
	/* remove folder-prefix */
	pathname += strlen(subname);
	if ((*pathname) == '\0')
	  continue;
	else if ((*pathname) == '\\')
	  pathname++;

	/* mask sub-folders */
	if (strchr(pathname, '\\'))
	  continue;

	/* this folder, contains files */
	bsfile sch; sch.assign(pathname);
	std::transform(sch.begin(), sch.end(), sch.begin(), ::tolower);
	std::replace(sch.begin(), sch.end(), '/', '\\');

	/* go find it, or add it */
	bsfileset::iterator ft = (*dt).files.find(sch);
	if (ft != (*dt).files.end()) {
	  if ((*ft).inp)
	    free((*ft).inp);

	  (*ft).ics = 0;
	  (*ft).inc = 0;
	  (*ft).inp = NULL;
	  (*ft).ins = 0;
	  (*ft).ina = 0;

	  return &(*ft);
	}

	return NULL;
      }
    }

    return NULL;
  }

  const bsfile *put(const char *pathname) {
    const char *drewind = pathname;

    /* root-directory file */
    if (!strchr(pathname, '\\'))
      mkdir("");

    /* folder-hit? */
    for (bsfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
      const char *subname = (*dt).data();

      pathname = drewind;
      if (pathname == stristr(pathname, subname)) {
	/* remove folder-prefix */
	pathname += strlen(subname);
	if ((*pathname) == '\0')
	  continue;
	else if ((*pathname) == '\\')
	  pathname++;

	/* mask sub-folders */
	if (strchr(pathname, '\\'))
	  continue;

	/* this folder, contains files */
	bsfile sch; sch.assign(pathname);
	std::transform(sch.begin(), sch.end(), sch.begin(), ::tolower);
	std::replace(sch.begin(), sch.end(), '/', '\\');

	/* append file if it doesn't exist */
	bsfileset::iterator ft = (*dt).files.find(sch);
	if (ft == (*dt).files.end()) {
	  sch.filetype = filetype(&sch);

	  memset(&sch.iinfo, 0, sizeof(sch.iinfo));
	  memset(&sch.oinfo, 0, sizeof(sch.oinfo));
	  sch.inp = NULL; sch.ics = sch.inc = sch.ins = sch.ina = 0;
	  sch.oup = NULL; sch.ocs = sch.ouc = sch.ous = sch.oua = 0;

	  ft = (*dt).files.insert(sch).first;
	}

	if (ft != (*dt).files.end()) {
	  if ((*ft).oup)
	    free((*ft).oup);

	  (*ft).ocs = 0;
	  (*ft).ouc = 0;
	  (*ft).oup = NULL;
	  (*ft).ous = 0;
	  (*ft).oua = 0;

	  return &(*ft);
	}

	return NULL;
      }
    }

    return NULL;
  }

  size_t raws(const bsfile *file) const {
    /* fetch value */
    if (file->oinfo.offset) {
      abort();
      return 0;
    }

    /* fetch value */
    if (file->iinfo.offset) {
      bool   pfx = (version == SK_BSAHEADER_VERSION) &&
	(obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
      size_t zlb = (file->oinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS)) ^
	(obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES);
      size_t sze = (file->iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS));

      /* objectable exact, but this data don't need to be so exact */
      return sze
	- (pfx ? sizeof(unsigned char) + *((unsigned char *)file->data()) : 0)
	- (zlb ? sizeof(unsigned int) : 0);
    }

    return 0;
  }

  size_t size(const bsfile *file) const {
    /* cached value */
    if (file->ouc)
      return file->ouc;
    /* cached value */
    if (file->inc)
      return file->inc;

    /* fetch value */
    if (file->oinfo.offset) {
      bool   pfx = (version == SK_BSAHEADER_VERSION) &&
	(obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
      size_t zlb = (file->oinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS)) ^
	(obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES);
      size_t sze = (file->oinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS));

      if (sze) {
	/* compression path */
	if (zlb) {
	  /* TODO! */
	  return 0;
	}
      }
      abort();
      return (file->ouc = sze);
    }

    /* fetch value */
    if (file->iinfo.offset) {
      bool   pfx = (version == SK_BSAHEADER_VERSION) &&
	(obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
      size_t zlb = (file->iinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS)) ^
	(obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES);
      size_t sze = (file->iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS));

      if (sze && (zlb || pfx)) {
	size_t ret; int res;

	/* remove size of prefixed name */
	if ((res = ibsa->seek(ibsa->fle, file->iinfo.offset, SEEK_SET)))
	  return 0;

	if (pfx) {
	  unsigned char nlen;

	  if ((ret = ibsa->read(&nlen, 1, sizeof(unsigned char), ibsa->fle)) != sizeof(unsigned char))
	    return 0;
	  if ((res = ibsa->seek(ibsa->fle, nlen, SEEK_CUR)))
	    return 0;

	  sze -= nlen;
	}

	/* read size of compressed blob */
	if (zlb) {
	  if ((ret = ibsa->read(&sze, 1, sizeof(long), ibsa->fle)) != sizeof(long))
	    return 0;
	}
      }

      return (file->inc = sze);
    }

    return 0;
  }

  static unsigned int filetype(bsfile *file) {
    char buf[256]; getext(buf, file->data());

    /**/ if (!stricmp(buf, "nif")) return OB_BSAFILE_NIF;
    else if (!stricmp(buf, "dds")) return OB_BSAFILE_DDS;
    else if (!stricmp(buf, "xml")) return OB_BSAFILE_XML;
    else if (!stricmp(buf, "wav")) return OB_BSAFILE_WAV;
    else if (!stricmp(buf, "mp3")) return OB_BSAFILE_MP3;
    else if (!stricmp(buf, "ogg")) return OB_BSAFILE_OGG;
    else if (!stricmp(buf, "xwm")) return OB_BSAFILE_XWM;
    else if (!stricmp(buf, "txt")) return OB_BSAFILE_TXT;
    else if (!stricmp(buf, "bat")) return OB_BSAFILE_BAT;
    else if (!stricmp(buf, "scc")) return OB_BSAFILE_SCC;
    else if (!stricmp(buf, "spt")) return OB_BSAFILE_SPT;
    else if (!stricmp(buf, "tex")) return OB_BSAFILE_TEX;
    else if (!stricmp(buf, "fnt")) return OB_BSAFILE_FNT;
    else if (!stricmp(buf, "cnt")) return OB_BSAFILE_CTL;

    else if (!stricmp(buf, "html")) return OB_BSAFILE_HTML;

    else if (!stricmp(buf, "lip")) return OB_BSAFILE_LIP;
    else if (!stricmp(buf, "fuz")) return OB_BSAFILE_FUZ;
    else if (!stricmp(buf, "bik")) return OB_BSAFILE_BIK;
    else if (!stricmp(buf, "jpg")) return OB_BSAFILE_JPG;

    return 0;
  }

  size_t read(const bsfile *file, void *block, unsigned int length) {
    bool         pfx = (version == SK_BSAHEADER_VERSION) &&
      (obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
    unsigned int zlb = (file->iinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS)) ^
      (obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES ? OB_BSAFILE_FLAG_COMPRESS : 0);
    unsigned int sze = (file->iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS));
    size_t ret; int res;

    if (!file->inp) {
      /* void read */
      if (!sze)
	return 0;

      file->inc = sze;
      file->ins = sze;
      file->inp = malloc(sze);
      file->ina = 0;

      /* failure */
      if (!file->inp)
	return 0;

      if ((res = ibsa->seek(ibsa->fle, file->iinfo.offset, SEEK_SET)))
	return 0;
      if ((ret = ibsa->read(file->inp, 1, sze, ibsa->fle)) != sze)
	return ret;

      /* decompress blob */
      if (zlb) {
	unsigned int offssze = (pfx ? sizeof(unsigned char) + *((unsigned char *)file->inp) : 0);

	unsigned char *bckpbuffer = (unsigned char *)file->inp;
	unsigned char *backbuffer = (unsigned char *)file->inp,
	              *frntbuffer;

	unsigned int realsze = *((unsigned int *)(backbuffer + offssze));
	unsigned int packsze = realsze;

	file->inc = realsze;
	file->ins = realsze + offssze;
	file->inp = malloc(1024 + realsze + offssze);
	file->ina = 0;

	/* failure */
	if (!(frntbuffer = (unsigned char *)file->inp))
	  return 0;

	/* copy prefix */
	if (offssze) {
	  memcpy(frntbuffer, backbuffer, offssze);

	  frntbuffer = frntbuffer + offssze;
	  backbuffer = backbuffer + offssze;
	}

	/* more's available */
	packsze += 1024;

	/* maybe gzip (+16 is gzip only, +32 is autodetect) */
	res = uncompress2(
	  (Bytef *)frntbuffer,
	  (uLongf *)&packsze,
	  (Bytef *)backbuffer + sizeof(int),
	  (uLongf)sze,
	  32 + MAX_WBITS
	);

	free(bckpbuffer);

	/* failure */
	if (res != Z_OK) {
	  /* this happens with 0/4-size files */
	  if (packsze != realsze) {
	    nfoprintf(stderr, "File \"%s\" has corrupt compression!\n", file->data());
	    return 0;
	  }
	}

	/* collect data */
	compressedinbytes += sze;
      }
      else
	compressedinbytes += sze;
    }

    /* skip prefixed name */
    if (!file->ina && pfx) {
      unsigned char nlen = *((unsigned char *)file->inp);

      file->ina = sizeof(unsigned char) + nlen;
    }

    /* limit reading to the size of the file */
    if (length > (file->ins - file->ina))
      length = file->ins - file->ina;
    else if (length == 0)
      length = file->inc;

    /* copy in the contents */
    if (block && length)
      memcpy(block, (char *)file->inp + file->ina, length);

    /* advance the "cursor" */
    file->ina += length;

    return length;
  }

  char getc(const bsfile *file) {
    char chr = EOF;

    if (!file->inp)
      read(file, &chr, 1);
    else if (file->ina < file->ins)
      chr = ((char *)file->inp)[file->ina++];

    return chr;
  }

  size_t write(const bsfile *file, const void *block, unsigned int length) {
    bool         pfx = (version == SK_BSAHEADER_VERSION) &&
      (obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
    unsigned int zlb = (file->iinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS)) ^
      (obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES ? OB_BSAFILE_FLAG_COMPRESS : 0);
    unsigned int sze = (file->iinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS));

    /* add prefixed name */
    unsigned int buffersize = length;
    if (!file->oup && pfx)
      buffersize += sizeof(unsigned char) + strlen(file->data());

    if (!file->oup) {
      /* void write */
      if (!buffersize)
	return 0;

      file->ouc = length;
      file->ous = buffersize;
      file->oup = malloc(buffersize);
      file->oua = 0;

      /* failure */
      if (!file->oup)
	return 0;
    }

    /* don't limit writing */
    if ((file->oua + buffersize) > file->ous) {
      file->ouc = file->ouc + length;
      file->ous = file->oua + buffersize;
      file->oup = realloc(file->oup, file->ous);
      file->oua = file->oua;

      /* failure */
      if (!file->oup)
	return 0;
    }

    /* add prefixed name (\len+255) */
    if (!file->oua && pfx) {
      const char *lname = file->data();
      unsigned char nlen = (unsigned char)min(file->length(), 255);

      char *beginbuffer = (char *)file->oup;
      *beginbuffer++ = nlen;
      memcpy(beginbuffer, lname, nlen);

      file->oua = sizeof(unsigned char) + nlen;
    }

    /* copy out the contents */
    if (block && length)
      memcpy((char *)file->oup + file->oua, block, length);

    /* advance the "cursor" */
    file->oua += length;

    return length;
  }

  int puts(const bsfile *file, const char *str) {
    return (int)write(file, str, (unsigned int)strlen(str));
  }

  void leave(const bsfile *file) {
    if (file->oup) {
      bool redo = true;

      /* if we have some i/o on the same archive we can remove
       * clones from the processing by comparing if
       * some content really changed
       */
      if (file->inp) {
	if (file->ins == file->ous) {
	  redo = !!memcmp(file->oup, file->inp, file->ins);
	}
      }

      /* problematic, "in" can be uncompressed and "out" suppose to be compressed */
      if (redo || 1) {
	size_t ret; int res;

	if (!obsa) {
//	  obsa = fopen((arcname + ".tmp").data(), "wb+");
	  obsa = iwrap(tmpfile());
	  /* add an offset of 1 so we can check vs. 0 */
	  if (!obsa->fle)
	    shutdown("Unable to open temporary file!");
	  
	  obsa->seek(obsa->fle, 1, SEEK_SET);
	}

	if (obsa->fle) {
	  // don't compress Morrowind-files or files which mustn't be compressed
	  bool         noz = (version == MW_BSAHEADER_VERSION) ||
			    ((version == SK_BSAHEADER_VERSION) &&
	    (file->filetype & OB_BSAFILE_EXLUDE));

	  bool         pfx = (version == SK_BSAHEADER_VERSION) &&
	    (obheader.ArchiveFlags & SK_BSAARCHIVE_PREFIXFULLFILENAMES);
	  unsigned int zlb = (file->oinfo.sizeFlags & ( OB_BSAFILE_FLAG_COMPRESS)) ^
	    (obheader.ArchiveFlags & OB_BSAARCHIVE_COMPRESSFILES ? OB_BSAFILE_FLAG_COMPRESS : 0);
	  unsigned int sze = (file->oinfo.sizeFlags & (~OB_BSAFILE_FLAG_ALLFLAGS));

	  /* compression path */
	  if ((zlb |= (compressbsa && !noz ? OB_BSAFILE_FLAG_COMPRESS : 0))) {
	    unsigned int offssze = file->ous - file->ouc;
	    unsigned int realsze =             file->ouc;
	    unsigned int spcesze = 1024 + sizeof(int) + compressBound(realsze) * 2 + offssze;
	    unsigned int packsze = realsze;
	    unsigned char *backbuffer = (unsigned char *)file->oup,
	                  *frntbuffer;
	    int offset = 0;

	    file->ouc = realsze;
	    file->ous = spcesze;
	    file->oup = malloc(spcesze);
	    file->oua = 0;

	    /* failure */
	    if (!(frntbuffer = (unsigned char *)file->oup))
	      shutdown("Writing BSA failed, running out of memory!");

	    /* copy prefix */
	    if (offssze) {
	      memcpy(frntbuffer, backbuffer, offssze);

	      frntbuffer = frntbuffer + offssze;
	      backbuffer = backbuffer + offssze;
	    }

	    /* search best strategy */
	    int wbits = MAX_WBITS;
	    int strategy = Z_DEFAULT_STRATEGY;
	    if (srchbestbsa) {
	      while ((1UL << (wbits - 1)) > realsze)
		wbits--;

	      /* inverse search */
	      unsigned int defaultsize = 0x7FFFFFFF;
	      unsigned int bestsize = 0x7FFFFFFF;
	      for (int s = Z_FIXED; s >= Z_DEFAULT_STRATEGY; s--) {
		/* space to compress into */
		packsze = spcesze;

		res = compress2(
		  (Bytef *)frntbuffer + sizeof(int),
		  (uLongf *)&packsze,
		  (Bytef *)backbuffer,
		  (uLongf)realsze,
		  compresslevel,
		  Z_DEFLATED,
		  wbits,
		  MAX_MEM_LEVEL,
		  s
		);

		/* non-failure */
		if (res == Z_OK) {
		  if (bestsize > packsze) {
		    bestsize = packsze;
		    strategy = s;
		  }

		  if (s == Z_DEFAULT_STRATEGY)
		    defaultsize = packsze;
		}
		/* failure
		else
		  shutdown("Writing compressed BSA failed!"); */
	      }

#ifdef	DEPEXT_7ZIP
	      /* non-destructive */
	      if (compresslevel == Z_BEST_COMPRESSION) {
		int passes = (optimizequick ? 1 : 5);
		int fastbytes = 255;

		while ((1UL << (24UL - passes)) > realsze)
		  passes++;

		/* space to compress into */
		packsze = spcesze;

		if (compress_rfc1950_7z(
		      (const unsigned char *)backbuffer, realsze,
		      (unsigned char *)frntbuffer + sizeof(int), packsze,
		      passes, fastbytes)) {

#if 0
		  res = uncompress(
		    (Bytef *)backbuffer + 0,
		    (uLongf *)&realsze,
		    (Bytef *)frntbuffer + sizeof(int),
		    (uLongf)packsze
		  );

		  /* failure */
		  if (res != Z_OK)
		    shutdown("Writing compressed BSA failed!");
#endif

		  if (bestsize > packsze) {
		    bestsize = packsze;
		    strategy = -1;
		  }
		}
	      }
#endif

	      /* check a few filetypes */
	      if (thresholdbsa) {
		if (file->filetype & OB_BSAFILE_THRESH) {
		  /* only if it's silly good */
		  if (((100.0f * bestsize) / realsze) >= 90.0f) {
		    strategy = -2;
		  }
		}
		else {
		  /* only not if it's silly bad */
		  if (((100.0f * bestsize) / realsze) >= 99.0f) {
		    strategy = -2;
		  }
		}
	      }

	      /* uh, MicroSoft, no signed "ssize_t"? */
	      ptrdiff_t delta = realsze - defaultsize;
	      ptrdiff_t serch = defaultsize - bestsize;
	      if (delta) {
		if (strategy == -2) {
		  nfoprintf(stderr, "compress revoked: by %d bytes (%.4f%%)\n", delta, (100.0f * bestsize) / realsze);
		}
		else if (strategy == Z_DEFAULT_STRATEGY) {
		  nfoprintf(stderr, "compressed:       by %d bytes (%.4f%%)\n", delta, (100.0f * bestsize) / realsze);
		}
		else if (strategy > Z_DEFAULT_STRATEGY) {
		  nfoprintf(stderr, "compressed:       by %d bytes (%.4f%%) (using simpler algorithm %d)\n", delta, (100.0f * bestsize) / realsze, strategy);
		}
		else {
		  nfoprintf(stderr, "compressed:       by %d bytes (%.4f%%) (%d eliminated through search)\n", delta, (100.0f * bestsize) / realsze, serch);
		}
	      }
	    }

	    if (strategy > -2) {
	      /* apply choosen strategy */
	      if (strategy >= Z_DEFAULT_STRATEGY) {
		/* space to compress into */
		packsze = spcesze;

		res = compress2(
		  (Bytef *)frntbuffer + sizeof(int),
		  (uLongf *)&packsze,
		  (Bytef *)backbuffer,
		  (uLongf)realsze,
		  compresslevel,
		  Z_DEFLATED,
		  wbits,
		  MAX_MEM_LEVEL,
		  strategy
		);

		/* failure */
		if (res != Z_OK)
		  shutdown("Writing compressed BSA failed, zlib error!");

#if 0
		res = uncompress(
		  (Bytef *)backbuffer + 0,
		  (uLongf *)&realsze,
		  (Bytef *)frntbuffer + sizeof(int),
		  (uLongf)*((unsigned int *)frntbuffer)
		);

		/* failure */
		if (res != Z_OK)
		  shutdown("Writing compressed BSA failed!");
#endif
	      }

	      /* check a few filetypes */
	      if (thresholdbsa) {
		if (file->filetype & OB_BSAFILE_THRESH) {
		  /* only if it's silly good */
		  if (((100.0f * packsze) / realsze) >= 90.0f) {
		    strategy = -2;
		  }
		}
		else {
		  /* only not if it's silly bad */
		  if (((100.0f * packsze) / realsze) >= 99.0f) {
		    strategy = -2;
		  }
		}
	      }
	    }

	    if (strategy > -2) {
	      /* and put original size in */
	      *((unsigned int *)frntbuffer) = realsze;

	      /* free old resource */
	      free(backbuffer - offssze);

	      /* write compressed size out */
	      file->ouc = realsze;
	      file->ous = packsze    + offssze + sizeof(unsigned int);
	      file->oup = frntbuffer - offssze;
	      file->oua = 0;

	      /* affirm compression-flag */
	      zlb = true;
	    }
	    else {
	      /* free new resource */
	      free(frntbuffer - offssze);

	      /* write uncompressed size out */
	      file->ouc = realsze;
	      file->ous = realsze    + offssze;
	      file->oup = backbuffer - offssze;
	      file->oua = 0;

	      /* revoke compression-flag */
	      zlb = false;
	    }

	    /* uh, MicroSoft, no signed "ssize_t"? */
	    if (!srchbestbsa) {
	      ptrdiff_t delta = realsze - packsze;
	      if (delta) {
		if (strategy == -2) {
		  nfoprintf(stderr, "compress revoked: by %d bytes (%.4f%%)\n", delta, (100.0f * packsze) / realsze);
		}
		else {
		  nfoprintf(stderr, "compressed:       by %d bytes (%.4f%%)\n", delta, (100.0f * packsze) / realsze);
		}
	      }
	    }

	    /* collect data */
	    compresseddtbytes += (zlb ? realsze - packsze : 0);
	    compressedoubytes += file->ous;
	  }

	  char *fail = NULL;
	  if (file->ous > (~OB_BSAFILE_FLAG_ALLFLAGS))
	    fail = "The file-record exceeds 1GiB!";
	  /* the order of the files in our temporary blob is irrelevant
	   * no need to make it ordered, just prevent overlapping writes
	   */
	  else { io::block(); do {
	    file->oinfo.sizeFlags = file->ous;
	    file->oinfo.offset = obsa->tell(obsa->fle);

	    /* check sanity */
	    if (file->oinfo.offset > 0x7FFFFFFF) {
	      fail = "The temporary file exceeds 2GiB!"; break; }
	    /* write to temporary file */
	    if ((ret = obsa->write(file->oup, 1, file->ous, obsa->fle)) != file->ous) {
	      fail = "Writing BSA failed!"; break; }
	  } while(0); io::release(); }

	  if (fail)
	    shutdown(fail);
	  if (zlb)
	    file->oinfo.sizeFlags |= OB_BSAFILE_FLAG_COMPRESS;

	  /* mark the body to be changed */
	  changedbody = true;
	}
      }

      /* create checksum */
      file->ocs = adler32(adler32(0,0,0), (const Bytef *)file->oup, file->ous);
//    file->ouc = 0;

      /* free the resources of temporary output */
      free(file->oup);

      file->oup = NULL;
      file->ous = 0;
      file->oua = 0;
    }

    if (file->inp) {
      /* create checksum */
      file->ics = adler32(adler32(0,0,0), (const Bytef *)file->inp, file->ins);
//    file->inc = 0;

      /* free the resources of temporary input */
      free(file->inp);

      file->inp = NULL;
      file->ins = 0;
      file->ina = 0;
    }
  }

  bool eof(const bsfile *file) const {
    /* points to the end */
    return (file->ina == file->ins);
  }

  long int tell(const bsfile *file) const {
    /**/ if (file->oup)
      return file->oua;
    else
      return file->ina;
  }

  int seek(const bsfile *file, long offset, int origin) {
    int ola = 0, pos;

    /**/ if (file->oup) {
      ola = file->oua;

      switch (origin) {
	case SEEK_CUR: pos = (int)file->oua + offset; break;
	case SEEK_END: pos = (int)file->ous + offset; break;
	case SEEK_SET: pos = (int)            offset; break;
	default:     __assume(0);                     break;
      }

      if (pos < 0)
	pos = 0;
      if (pos > (int)file->ous)
	write(file, NULL, (unsigned int)(pos - file->ous));

      file->oua = (unsigned int)pos;
    }
    else if (file->inp) {
      ola = file->ina;

      switch (origin) {
	case SEEK_CUR: pos = (int)file->ina + offset; break;
	case SEEK_END: pos = (int)file->ins + offset; break;
	case SEEK_SET: pos = (int)            offset; break;
	default:     __assume(0);                     break;
      }

      if (pos < 0)
	pos = 0;
      if (pos > (int)file->ins)
	pos = (int)file->ins;

      file->ina = (unsigned int)pos;
    }

    return 0;
  }
};

/* ------------------------------------------------------------ */

set<string> cachearc;
map<string, bsarchive> archives;

bool __cdecl isarchive(const char *pathname, int *version) {
  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      if (version) {
	/* load archive into memory */
	bsarchive &arc = archives[arcname];
	if (!arc.loaded)
	  arc.open(arcname);

	*version = arc.version;
      }

      return true;
    }
  }

  char *walk = strdup(pathname);
  char *shrt = walk + strlen(walk);

  while (shrt) {
    *shrt = '\0';
    
    /* allows zips inside bsas */
    if (isext(walk, "zip"))
      break;
    /* direct hit? */
    if (isext(walk, "bsa")) {
      cachearc.insert(walk);

      if (version) {
	/* load archive into memory */
	bsarchive &arc = archives[walk];
	if (!arc.loaded)
	  arc.open(walk);

	*version = arc.version;
      }
      
      free(walk);
      return true;
    }

    shrt = strrchr(walk, '\\');
  }

  free(walk);
  return false;
}

bool __cdecl isarchive(struct io::dir *dir) {
  return !!dir->ad;
}

bool __cdecl isarchive(struct io::file *file) {
  return !!file->af;
}

/* ------------------------------------------------------------ */

int __cdecl stat_arc(const char *pathname, struct io::info *info) {
  int ret = -1;

  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);
      if ((*pathname) == '\0')
	info->io_type = IO_ARCHIVE,
	info->io_size = 0,
	info->io_raws = 0,
	info->io_time = arc.arctime,
	ret = 0;
      else if ((*pathname) == '\\')
	ret = arc.stat(pathname + 1, info);

      break;
    }
  }

  return ret;
}

/* ------------------------------------------------------------ */

int __cdecl mkdir_arc(const char *pathname) {
  int ret = -1;

  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);
      if ((*pathname) == '\0')
	return arc.mkdir("");
      else if ((*pathname) == '\\')
	return arc.mkdir(pathname + 1);

      break;
    }
  }

  char *walk = strdup(pathname);
  char *shrt = walk + strlen(walk);

  do {
    /* allows zips inside bsas */
    if (isext(walk, "zip"))
      break;
    /* direct hit? */
    if (isext(walk, "bsa")) {
      const char *arcname = walk;
      cachearc.insert(walk);

      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);
      if ((*pathname) == '\0')
	ret = arc.mkdir("");
      else if ((*pathname) == '\\')
	ret = arc.mkdir(pathname + 1);

      break;
    }

    *shrt = '0';
  } while ((shrt = strrchr(shrt, '\\')));
  
  free(walk);
  return ret;
}

int __cdecl rmdir_arc(const char *pathname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);
      if ((*pathname) == '\0') {
	if (arc.loaded) {
	  io::flush();

	  arc.close();
	}

	/* cachearc is the string-owner */
	archives.erase(arcname);
	cachearc.erase(arcname);
	
	/* unlink inside something else */
	IOFL *ifc = iopen(arcname, NULL);
	int ret = ifc->unlink(arcname);
	free(ifc);

	return ret;
      }
      else if ((*pathname) == '\\')
	return arc.rmdir(pathname + 1);

      break;
    }
  }

  return -1;
}

class it_dir_arc {
public:
  set<string> contents;
  set<string>::iterator position;

  const char *arcname;
  bsarchive *arc;
  string location;

  struct dirent e;
};

void * __cdecl opendir_arc(const char *pathname) {
  it_dir_arc *r = NULL;

  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);

      r = new it_dir_arc;
      r->arc = &arc;
      r->arcname = arcname;

      if ((*pathname) == '\0') {
	r->location = pathname;
	r->contents = arc.find(pathname);
	r->position = r->contents.begin();
      }
      else if ((*pathname) == '\\') {
	r->location = pathname + 1;
	r->contents = arc.find(pathname + 1);
	r->position = r->contents.begin();
      }

      /* not found (found mean "." & ".." is present) */
      if (r->contents.begin() == r->contents.end()) {
	delete r;
	r = NULL;
      }

      break;
    }
  }

  return r;
}

struct dirent * __cdecl readdir_arc(void *dir) {
  it_dir_arc *r = (it_dir_arc *)dir;

  if (r->position != r->contents.end()) {
    strcpy(r->e.d_name, (*r->position).data());
    r->e.d_namlen = (char)(*r->position).length();

    r->position++;
    return &r->e;
  }
  else {
    return NULL;
  }
}

void __cdecl closedir_arc(void *dir) {
  it_dir_arc *r = (it_dir_arc *)dir;

  if (r->location == "") {
    if (r->arc->loaded) {
      io::flush();

      r->arc->close();
    }

    /* cachearc is the string-owner */
    archives.erase(r->arcname);
    cachearc.erase(r->arcname);
  }

  delete r;
}

void __cdecl free_arc() {
#if 0
  /* TODO */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    /* only throw modified archives */
    bsarchive &arc = archives[arcname];
    if (arc.changed())
      archives.erase(arcname);
  }
#endif

  cachearc.clear();
  archives.clear();
}

/* ------------------------------------------------------------ */

int __cdecl rename_arc(const char *srcname, const char *dstname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (srcname == stristr(srcname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      srcname += strlen(arcname);
      dstname += strlen(arcname);

      if ((*srcname) == '\\') srcname++;
      if ((*dstname) == '\\') dstname++;

      // TODO: support renames across archive/file-system types
      return arc.rename(srcname, dstname);
    }
  }

  return -1;
}

int __cdecl unlink_arc(const char *pathname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);

      if ((*pathname) == '\0')
	;
      else if ((*pathname) == '\\')
	return arc.unlink(pathname + 1);

      break;
    }
  }

  return -1;
}

/* ------------------------------------------------------------ */

class it_file_arc {
public:
  const bsfile *file;

  const char *arcname;
  bsarchive *arc;
  char arcmode;
};

void * __cdecl fopen_arc(const char *pathname, const char *mode) {
  it_file_arc *r = NULL;

  /* cache-hit? */
  for (set<string>::iterator it = cachearc.begin(); it != cachearc.end(); ++it) {
    const char *arcname = (*it).data();

    if (pathname == stristr(pathname, arcname)) {
      /* load archive into memory */
      bsarchive &arc = archives[arcname];
      if (!arc.loaded)
	arc.open(arcname);

      /* remove archive-prefix */
      pathname += strlen(arcname);

      if ((*pathname) == '\0')
	;
      else if ((*pathname) == '\\') {
	const bsfile *file = NULL;
	const char *rmode;

	if ((rmode = strchr(mode, 'w')))
	  file = arc.put(pathname + 1);
	else if ((rmode = strchr(mode, 'r')))
	  file = arc.get(pathname + 1);

	if (file) {
	  r = new it_file_arc;
	  r->arc = &arc;
	  r->arcname = arcname;
	  r->arcmode = rmode[0];
	  r->file = file;
	}
      }

      break;
    }
  }

  return r;
}

size_t __cdecl fread_arc(void *block, size_t elements, size_t size, void *file) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->read(r->file, block, (unsigned int)(elements * size));
}

char __cdecl getc_arc(void *file) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->getc(r->file);
}

size_t __cdecl fwrite_arc(const void *block, size_t elements, size_t size, void *file) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->write(r->file, block, (unsigned int)(elements * size));
}

int __cdecl fputs_arc(const char *str, void *file) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->puts(r->file, str);
}

DWORD __stdcall fclose_arc_async(void *file) {
  it_file_arc *r = (it_file_arc *)file;
  try {
    r->arc->leave(r->file); }
  catch(exception &e) {
    io::rethrow(e.what()); }
  delete file;
  return NULL;
}

void __cdecl fclose_arc(void *file) {
  it_file_arc *r = (it_file_arc *)file;
  /* nowait asynchronous (write-queue) */
  if (r->arcmode == 'w')
    io::dispatch(fclose_arc_async, file);
  else
    fclose_arc_async(file);
}

bool __cdecl feof_arc(void *file) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->eof(r->file);
}

int __cdecl fseek_arc(void *file, long offset, int origin) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->seek(r->file, offset, origin);
}

int __cdecl fstat_arc(void *file, struct io::info *info) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->stat(r->file, info);
}

long int __cdecl ftell_arc(void *file) {
  it_file_arc *r = (it_file_arc *)file;
  return r->arc->tell(r->file);
}

/* ------------------------------------------------------------ */

#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

class istream_arc : public istringstream {
public:
  class it_file_arc *iface;
};

class ostream_arc : public ostringstream {
public:
  class it_file_arc *iface;
};

istream *openistream_arc(const char *pathname) {
  istream_arc *r = NULL;
  class it_file_arc *iface;

  iface = (class it_file_arc *)fopen_arc(pathname, "rb");
  if (iface) {
    r = new istream_arc;
    r->iface = iface;
    r->flags(ios_base::binary);

    /* copy-over the string ... */
    fread_arc(NULL, 1,  0, r->iface);

    string bin((char *)iface->file->inp, iface->file->ins);

    r->str(bin);
    r->istringstream::seekg(0);
  }

  return r;
}

ostream *openostream_arc(const char *pathname) {
  ostream_arc *r = NULL;
  class it_file_arc *iface;

  iface = (class it_file_arc *)fopen_arc(pathname, "wb+");
  if (iface) {
    r = new ostream_arc;
    r->iface = iface;
    r->flags(ios_base::binary);
  }

  return r;
}

void closeistream_arc(istream *ist) {
  istream_arc *r = (istream_arc *)ist;
  fclose_arc(r->iface);
  delete r;
}

void closeostream_arc(ostream *ost) {
  ostream_arc *r = (ostream_arc *)ost;

//ioflushtofile(r->str().data(), r->str().length(), r->iface);

  /* nowait asynchronous (write-queue) */
//#pragma omp single nowait
  {
    size_t written =

    /* copy-over the string ... */
    fwrite_arc(r->str().data(), 1, r->str().length(), r->iface);
    fclose_arc(r->iface);

    delete r;
  }
}

long tellistream_arc(istream *ist) {
  istream_arc *r = (istream_arc *)ist;
//return ftell_arc(r->iface);
  size_t check = r->str().length();
  size_t verfy = ist->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in);
  assert(check == verfy);
  return (long)check;
}

long tellostream_arc(ostream *ost) {
  ostream_arc *r = (ostream_arc *)ost;
//return ftell_arc(r->iface);
  size_t check = r->str().length();
  size_t verfy = ost->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
  assert(check == verfy);
  return (long)check;
}

bool isarchive(istream *ist) {
  return !!dynamic_cast<istream_arc *>(ist);
}

bool isarchive(ostream *ost) {
  return !!dynamic_cast<ostream_arc *>(ost);
}
