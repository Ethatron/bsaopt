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
 * The Original Code is zipopt.
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

#define	DEPEXT_ZLIB
#define	DEPEXT_LIBZIP
#define	DEPEXT_7ZIP
#include "depext.C"

/* ------------------------------------------------------------ */

struct zipio {
  IOFL *src;
  struct zip_stat st;	/* stat information passed in */
  
  time_t tme;
  zip_uint64_t off;	/* start offset of */
  zip_int64_t len;	/* length of data to copy */
  zip_int64_t remain;	/* bytes remaining to be copied */
  int e[2];		/* error codes */
};

static zip_int64_t read_zipio(void *state, void *data, zip_uint64_t len, enum zip_source_cmd cmd);

struct zip_source *zip_source_io(struct zip *za, IOFL *file, time_t mtime, zip_uint64_t start, zip_int64_t len) {
  struct zipio *f;
  struct zip_source *zs;

  if ((f = (struct zipio *)malloc(sizeof(struct zipio))) == NULL)
    return NULL;

  f->src = file;
  f->tme = mtime;
  f->off = start;
  f->len = (len ? len : -1);
    
  zip_stat_init(&f->st);

  if ((zs = zip_source_function(za, read_zipio, f)) == NULL) {
    free(f);
    return NULL;
  }

  return zs;
}

static zip_int64_t read_zipio(void *state, void *data, zip_uint64_t len, enum zip_source_cmd cmd) {
  struct zipio *z;
  char *buf;
  size_t i, n;

  z = (struct zipio *)state;
  buf = (char *)data;

  switch (cmd) {
    case ZIP_SOURCE_OPEN:
      z->remain = z->len;
      return 0;
	
    case ZIP_SOURCE_READ:
      /* XXX: return INVAL if len > size_t max */
      if (z->remain != -1)
	n = len > (zip_uint64_t)z->remain ? (zip_uint64_t)z->remain : len;
      else
	n = len;

      if (1) {
	/* we might share this file with others, so let's be safe */
	if (z->src->seek(z->src->fle, (off_t)(z->off + (zip_uint64_t)(z->len-z->remain)), SEEK_SET) < 0) {
	  z->e[0] = ZIP_ER_SEEK;
	  z->e[1] = errno;
	  return -1;
	}
      }

      if ((i = z->src->read(buf, 1, n, z->src->fle)) == 0) {
	{
	  z->e[0] = ZIP_ER_READ;
	  z->e[1] = errno;
	  return -1;
	}
      }

      if (z->remain != -1)
	z->remain -= i;

      return (zip_int64_t)i;
	
    case ZIP_SOURCE_CLOSE:
      return 0;

    case ZIP_SOURCE_STAT:
      {
	if (len < sizeof(z->st))
	  return -1;

	if (z->st.valid != 0)
	  memcpy(data, &z->st, sizeof(z->st));
	else {
	  struct zip_stat *st;

	  st = (struct zip_stat *)data;
		
	  zip_stat_init(st);

	  st->mtime = z->tme;
	  st->valid |= ZIP_STAT_MTIME;
	  {
	    st->size = (zip_uint64_t)z->len;
	    st->valid |= ZIP_STAT_SIZE;
	  }
	}

	return sizeof(z->st);
      }

    case ZIP_SOURCE_ERROR:
      if (len < sizeof(int) * 2)
	  return -1;

      memcpy(data, z->e, sizeof(int) * 2);
      return sizeof(int) * 2;

    case ZIP_SOURCE_FREE:
      free(z);
      return 0;

    default:
      ;
  }

  return -1;
}

/* ------------------------------------------------------------ */

class zipfile  ; typedef set<zipfile  , stringi> zipfileset;
class zipfolder; typedef set<zipfolder, stringi> zipfolderset;

class zipfile : public string {

public:
  zipfile() {
    inp = NULL;
    oup = NULL;
  }

  ~zipfile() {
    if (inp) free(inp);
    if (oup) free(oup);
  }

  void clear() {
    inp = NULL; ins = inc = ina = 0; ics = 0;
    oup = NULL; ous = ouc = oua = 0; ocs = 0;
  }

  mutable zipfolderset::iterator fld;

public:
  /* ... */
  mutable zip_int64_t zindex;
  mutable struct zip_stat zinfo;
  mutable unsigned int filetype;
  mutable zip_uint64_t inc; mutable unsigned int ics;
  mutable zip_uint64_t ouc; mutable unsigned int ocs;

//mutable unsigned int ia32; mutable unsigned int ic32; mutable unsigned int isha;
//mutable unsigned int oa32; mutable unsigned int oc32; mutable unsigned int osha;

  /* in/out to make read-write to the same file
   * in the same archive possible
   */
  mutable void *inp; mutable zip_uint64_t ins; mutable zip_uint64_t ina;
  mutable void *oup; mutable zip_uint64_t ous; mutable zip_uint64_t oua;
};

class zipfolder : public string {

public:
  mutable zipfileset files;

public:
  /* ... */
};

class zipdir {

public:
  zipdir() :
    loaded(false),
    changedzip(false),
    iozip(NULL),
    temp(NULL) {}
  ~zipdir() {
    if (iozip) zip_close(iozip);
    if (temp) { if (temp->fle) temp->close(temp->fle); free(temp); temp = NULL; }
  }

  bool shutdown(const char *message) {
    loaded = false;
    folders.clear();
    
    if (iozip) zip_unchange_all(iozip), zip_close(iozip);
    if (temp) { if (temp->fle) temp->close(temp->fle); free(temp); temp = NULL; }

    throw runtime_error(message);
    return true;
  }
  
public:
  bool loaded, changedzip;
  struct zip *iozip;
  IOFL *temp;

  time_t ziptime;
  string zipname;
  zipfolderset folders;

public:
  bool open(const char *pathname) {
    zipname.assign(pathname);
    ziptime = 0; struct stat sinfo;
    if (!::stat(pathname, &sinfo))
      ziptime = sinfo.st_ctime;

    iozip = zip_open(pathname, ZIP_CREATE, NULL);
    if (iozip) {
      int num = zip_get_num_files(iozip);
      
      for (int n = 0; n < num; n++) {
	struct zip_stat zinfo;
	if (zip_stat_index(iozip, n, 0, &zinfo) != -1) {
	  zipfolder folder;
	  
	  folder.assign(zinfo.name);
	  std::replace(folder.begin(), folder.end(), '/', '\\');

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
	  zipfolderset::iterator folderref = folders.end();
	  for (zipfolderset::iterator it = folders.begin(); it != folders.end(); ++it) {
	    if (!stricmp(folder.data(), (*it).data())) {
	      folderref = it;
	      break;
	    }
	  }
	  
	  if (folderref == folders.end())
	    folderref = (folders.insert(folder)).first;

	  // directories are also entries (ending on "/")
	  if (nfle != "") {
	    zipfile file;

	    file.assign(nfle);
	    file.clear();
	    file.inc = zinfo.size;

	    file.fld = folderref;
	    file.zindex = n;
	    file.zinfo = zinfo;
	    file.filetype = filetype(&file);

	    folderref->files.insert(file);
	  }
	}
      }
    }

    return (loaded = (iozip != NULL));
  }

  bool changed() {
    return changedzip;
  }

  bool close() {
    if (changed()) {
      SetTopic("Consolidating ZIP-fragments:");
    }
      
    if (iozip) {
      zip_close(iozip); iozip = NULL;
    }

    return true;
  }

  int stat(const char *pathname, struct io::info *info) const { 
    const char *drewind = pathname;
    int ret = -1;

    /* cache-hit? */
    for (zipfolderset::const_iterator it = folders.begin(); it != folders.end(); ++it) {
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
	  info->io_time = ziptime;

	  ret = 0;
	  break;
	}
	else if ((*pathname) == '\\')
	  pathname++;

	/* mask sub-folders */
	if (!strchr(pathname, '\\')) {
	  zipfile sch; sch.assign(pathname);
	  std::replace(sch.begin(), sch.end(), '/', '\\');

	  zipfileset::iterator ft = (*it).files.find(sch);
	  if (ft != (*it).files.end()) {
	    info->io_type = IO_FILE;
	    info->io_size = size(&(*ft));
	    info->io_raws = raws(&(*ft));
	    info->io_time = time(&(*ft));

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
	  info->io_time = ziptime;

	  ret = 0;
	  break;
	}
      }
    }

    return ret;
  }

  int stat(const zipfolder *file, struct io::info *info) const {
    info->io_type = IO_DIRECTORY;
    info->io_size = 0;
    info->io_raws = 0;
    info->io_time = ziptime;

    return 0;
  }

  int stat(const zipfile *file, struct io::info *info) const {
    info->io_type = IO_FILE;
    info->io_size = size(file);
    info->io_raws = raws(file);
    info->io_time = time(file);

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
    zipfolder sch; sch.assign(pathname);
    std::replace(sch.begin(), sch.end(), '/', '\\');

    /* go find it, or skip it */
    zipfolderset::iterator dt = folders.find(sch);
    if (dt == folders.end()) {
      /* mark the head & possibly body to be changed */
      changedzip = changedzip || true;
      
      string unix; unix.assign(pathname); unix += "\\";
      std::replace(unix.begin(), unix.end(), '\\', '/');

      zip_add_dir(iozip, unix.data());

      folders.insert(sch);
      ret = 0;
    }

    return ret;
  }

  int rmdir(const char *pathname) {
    int ret = -1;

    /* man, all lowercase ... */
    zipfolder sch; sch.assign(pathname);
    std::replace(sch.begin(), sch.end(), '/', '\\');

    /* go find it, or skip it */
    zipfolderset::iterator dt = folders.find(sch);
    if (dt != folders.end()) {
      /* mark the head & possibly body to be changed */
      changedzip = changedzip || !!(*dt).files.size();

      /* this folder, contains files */
      for (zipfileset::const_iterator ft = (*dt).files.begin(); ft != (*dt).files.end(); ++ft) {
	string unix; unix.assign(*dt); unix += "\\"; unix += (*ft); unix += "\\";
	std::replace(unix.begin(), unix.end(), '\\', '/');

	zip_int64_t zidx = zip_name_locate(iozip, unix.data(), 0);
	if (zidx != -1)
	  zip_delete(iozip, zidx);
      }

      folders.erase(*dt);
      ret = 0;
    }

    return ret;
  }
  
  set<string> find(const char *pathname) const {
    set<string> contents;

    /* folder-hit? */
    for (zipfolderset::const_iterator dt = folders.begin(); dt != folders.end(); ++dt) {
      const char *subname = (*dt).data(), *ssn;

      if (subname == stristr(subname, pathname)) {
	/* remove folder-prefix */
	subname += strlen(pathname);

	if ((*subname) == '\0') {
	  /* this folder, contains files */
	  for (zipfileset::const_iterator ft = (*dt).files.begin(); ft != (*dt).files.end(); ++ft)
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
    const char *srewind = srcname;
    const char *drewind = dstname;
    zipfolderset::iterator sdt;
    zipfolderset::iterator ddt;
    zipfileset::iterator sft;
    zipfileset::iterator dft;
    zipfile sf;
    zipfile df;
    bool sfolder = false;
    bool dfolder = false;

    /* folder-hit? */
    for (sdt = folders.begin(); sdt != folders.end(); ++sdt) {
      const char *subname = (*sdt).data();

      srcname = srewind;
      if (srcname == stristr(srcname, subname)) {
	/* remove folder-prefix */
	srcname += strlen(subname);
	if ((*srcname) == '\0') {
	  sfolder = true; break; }
	else if ((*srcname) == '\\')
	  srcname++;

	/* mask sub-folders */
	if (strchr(srcname, '\\'))
	  continue;
	
	/* this folder, contains files */
	sf.assign(srcname);
	std::replace(sf.begin(), sf.end(), '/', '\\');

	/* go find it, or add it */
	sft = (*sdt).files.find(sf);
	if (sft == (*sdt).files.end()) {
	  sfolder = false; break; }
	
	sfolder = false; break;
      }
    }
    
    // no src-directory match
    if (sdt == folders.end())
      return -1;
    // no src-file match
    if (sft == (*sdt).files.end())
      return -1;

    /* folder-hit? */
    for (ddt = folders.begin(); ddt != folders.end(); ++ddt) {
      const char *subname = (*ddt).data();

      dstname = drewind;
      if (dstname == stristr(dstname, subname)) {
	/* remove folder-prefix */
	dstname += strlen(subname);
	if ((*dstname) == '\0') {
	  dfolder = true; break; }
	else if ((*dstname) == '\\')
	  dstname++;

	/* mask sub-folders */
	if (strchr(dstname, '\\'))
	  continue;
	
	/* this folder, contains files */
	df.assign(dstname);
	std::replace(df.begin(), df.end(), '/', '\\');

	/* go find it, or add it */
	dft = (*ddt).files.find(df);
	if (dft == (*ddt).files.end()) {
	  dfolder = false; break; }
	
	dfolder = false; break;
      }
    }
    
    /* no dst-directory match
    if (ddt == folders.end())
      return; */

    if (sfolder) {
      // rename sub-folders and stuff, really ugly
      abort();
    }
    else if (dfolder) {/*
      (*dt).files.erase(*ft);
      (*ft).assign();

      folders.insert(*dt);*/
      abort();
    }
    else {
      /* no dst-directory match */
      if (ddt == folders.end())
	return -1;

      /* dst-file match */
      if (dft != (*ddt).files.end())
	zip_delete(iozip, dft->zindex);
      /* no dst-file match */
      else {
	df.fld = ddt;
	dft = (*ddt).files.insert(df).first;
      }
      
      /* src-file match */
      zip_rename(iozip, sft->zindex, dstname);
      
      /* copy data */
      (*dft).ics = (*sft).ics;
      (*dft).ocs = (*sft).ocs;
      (*dft).inc = (*sft).inc;
      (*dft).ouc = (*sft).ouc;
      (*dft).ina = (*sft).ina;
      (*dft).oua = (*sft).oua;
      (*dft).inp = (*sft).inp; (*sft).inp = NULL;
      (*dft).oup = (*sft).oup; (*sft).oup = NULL;
      (*dft).zindex = (*sft).zindex;
      (*dft).zinfo = (*sft).zinfo;
      (*dft).filetype = (*sft).filetype;

      /* mark the body to be changed */
      changedzip = true;

      /* kill old file */
      (*sdt).files.erase(*sft);
      return 0;
    }
  }

  int unlink(const char *pathname) {
    const char *drewind = pathname;

    /* folder-hit? */
    for (zipfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
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
	zipfile sch; sch.assign(pathname);
	std::replace(sch.begin(), sch.end(), '/', '\\');

	/* go find it, or add it */
	zipfileset::iterator ft = (*dt).files.find(sch);
	if (ft != (*dt).files.end()) {
	  if ((*ft).inp) free((*ft).inp);
	  if ((*ft).oup) free((*ft).oup);

	  zip_delete(iozip, ft->zindex);
	  
	  /* mark the body to be changed */
	  changedzip = true;
	  
	  /* kill old file */
	  (*dt).files.erase(*ft);
	  return 0;
	}
	
	return -1;
      }
    }

    return -1;
  }

  const zipfile *get(const char *pathname) {
    const char *drewind = pathname;

    /* folder-hit? */
    for (zipfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
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
	zipfile sch; sch.assign(pathname);
	std::replace(sch.begin(), sch.end(), '/', '\\');

	/* go find it, or add it */
	zipfileset::iterator ft = (*dt).files.find(sch);
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

  const zipfile *put(const char *pathname) {
    const char *drewind = pathname;

    /* root-directory file */
    if (!strchr(pathname, '\\'))
      mkdir("");

    /* folder-hit? */
    for (zipfolderset::iterator dt = folders.begin(); dt != folders.end(); ++dt) {
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
	zipfile sch; sch.assign(pathname);
	std::replace(sch.begin(), sch.end(), '/', '\\');

	/* append file if it doesn't exist */
	zipfileset::iterator ft = (*dt).files.find(sch);
	if (ft == (*dt).files.end()) {
	  sch.filetype = filetype(&sch);
	  sch.fld = dt;
	  sch.zindex = -1;

	  zip_stat_init(&sch.zinfo);

	  sch.inp = NULL; sch.ins = sch.inc = sch.ina = 0; sch.ics = 0;
	  sch.oup = NULL; sch.ous = sch.ouc = sch.oua = 0; sch.ocs = 0;

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
  
  time_t time(const zipfile *file) const {
    /* fetch value */
    if (file->zindex >= 0) {
      return (time_t)file->zinfo.mtime;
    }

    return ziptime;
  }

  size_t raws(const zipfile *file) const {
    /* fetch value */
    if (file->zindex >= 0) {
      return (size_t)file->zinfo.comp_size;
    }

    return 0;
  }

  size_t size(const zipfile *file) const {
    /* cached value */
    if (file->ouc)
      return (size_t)file->ouc;
    /* cached value */
    if (file->inc)
      return (size_t)file->inc;

    /* fetch value */
    if (file->zindex >= 0) {
      return (size_t)(file->inc = file->zinfo.size);
    }

    return 0;
  }

  static unsigned int filetype(zipfile *file) {
    char buf[256]; getext(buf, file->data());

    return 0;
  }

  size_t read(const zipfile *file, void *block, size_t length) {
    struct zip_file *zfile;

    zip_uint64_t sze = file->zinfo.size;
    size_t ret;

    if (!file->inp) {
      /* void read */
      if (!sze)
	return 0;

      file->inc = sze;
      file->ins = sze;
      file->inp = malloc((size_t)sze);
      file->ina = 0;

      /* failure */
      if (!file->inp)
	return 0;
      if (file->zindex < 0)
	return 0;
      
      if ((zfile = zip_fopen_index(iozip, file->zindex, ZIP_FL_UNCHANGED)) == NULL)
	return 0;
      if ((ret = (size_t)zip_fread(zfile, file->inp, sze)) != sze)
	return ret;

      zip_fclose(zfile);

      /* collect data */
      compressedinbytes += (size_t)sze;
    }

    /* limit reading to the size of the file */
    if (length > (size_t)(file->ins - file->ina))
      length = (size_t)(file->ins - file->ina);
    else if (length == 0)
      length = (size_t)(file->inc);

    /* copy in the contents */
    if (block && length)
      memcpy(block, (char *)file->inp + file->ina, length);

    /* advance the "cursor" */
    file->ina += length;

    return length;
  }

  char getc(const zipfile *file) {
    char chr = EOF;

    if (!file->inp)
      read(file, &chr, 1);
    else if (file->ina < file->ins)
      chr = ((char *)file->inp)[file->ina++];

    return chr;
  }

  size_t write(const zipfile *file, const void *block, size_t length) {
    /* add prefixed name */
    size_t buffersize = length;

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
      file->oup = realloc(file->oup, (size_t)file->ous);
      file->oua = file->oua;

      /* failure */
      if (!file->oup)
	return 0;
    }

    /* copy out the contents */
    if (block && length)
      memcpy((char *)file->oup + file->oua, block, length);

    /* advance the "cursor" */
    file->oua += length;

    return length;
  }

  int puts(const zipfile *file, const char *str) {
    return (int)write(file, str, strlen(str));
  }

  void leave(const zipfile *file) {
    if (file->oup) {
      bool redo = true;

      /* if we have some i/o on the same archive we can remove
       * clones from the processing by comparing if
       * some content really changed
       */
      if (file->inp) {
	if (file->ins == file->ous) {
	  redo = !!memcmp(file->oup, file->inp, (size_t)file->ins);
	}
      }

      /* problematic, "in" can be uncompressed and "out" suppose to be compressed */
      if (redo || 1) {
	size_t ret;

	if (!temp) {
//	  temp = fopen((arcname + ".tmp").data(), "wb+");
	  temp = iwrap(tmpfile());
	  /* add an offset of 1 so we can check vs. 0 */
	  if (!temp->fle)
	    shutdown("Unable to open temporary file!");
	  
	  temp->seek(temp->fle, 1, SEEK_SET);
	}

	if (temp->fle) {
	  size_t offset = 1;

	  char *fail = NULL;
	  if (file->ous > 0x3FFFFFFF)
	    fail = "The file-record exceeds 1GiB!";
	  /* the order of the files in our temporary blob is irrelevant
	   * no need to make it ordered, just prevent overlapping writes
	   */
	  else { io::block(); do {
	    offset = temp->tell(temp->fle);

	    /* check sanity */
	    if (offset > 0x7FFFFFFF) {
	      fail = "The temporary file exceeds 2GiB!"; break; }
	    /* write to temporary file */
	    if ((ret = temp->write(file->oup, 1, file->ous, temp->fle)) != file->ous) {
	      fail = "Writing BSA failed!"; break; }
	  } while(0); io::release(); }

	  if (fail)
	    shutdown(fail);
	  
	  if (1) {
	    string unix; unix.assign(*(file->fld)); unix += "\\"; unix += (*file);
	    std::replace(unix.begin(), unix.end(), '\\', '/');

	    struct zip_source *zs = zip_source_io(iozip, temp, ziptime, offset, file->ous);
	
	    if (file->zindex < 0) {
	      if ((file->zindex = zip_add(iozip, unix.data(), zs)) == -1)
		shutdown("Writing compressed Zip failed, add error!");
	    }
	    else {
	      if (zip_replace(iozip, file->zindex, zs) == -1)
		shutdown("Writing compressed Zip failed, replacement error!");
	    }

	    zip_stat_index(iozip, file->zindex, 0, &file->zinfo);

	    /* collect data */
	    compresseddtbytes += 0/*realsze - packsze*/;
	    compressedoubytes += (size_t)file->ous;
	  }

	  /* mark the body to be changed */
	  changedzip = true;
	}
      }

      /* create checksum */
      file->ocs = adler32(adler32(0,0,0), (const Bytef *)file->oup, (uInt)file->ous);
//    file->ouc = 0;

      /* free the resources of temporary output
      free(file->oup); */

      file->oup = NULL;
      file->ous = 0;
      file->oua = 0;
    }

    if (file->inp) {
      /* create checksum */
      file->ics = adler32(adler32(0,0,0), (const Bytef *)file->inp, (uInt)file->ins);
//    file->inc = 0;

      /* free the resources of temporary input */
      free(file->inp);

      file->inp = NULL;
      file->ins = 0;
      file->ina = 0;
    }
  }

  bool eof(const zipfile *file) const {
    /* points to the end */
    return (file->ina == file->ins);
  }

  long int tell(const zipfile *file) const {
    /**/ if (file->oup)
      return (long int)file->oua;
    else
      return (long int)file->ina;
  }

  int seek(const zipfile *file, long offset, int origin) {
    zip_int64_t ola = 0, pos;

    /**/ if (file->oup) {
      ola = file->oua;

      switch (origin) {
	case SEEK_CUR: pos = file->oua + offset; break;
	case SEEK_END: pos = file->ous + offset; break;
	case SEEK_SET: pos =             offset; break;
	default:     __assume(0);                break;
      }

      if (pos < 0)
	pos = 0;
      if (pos > file->ous)
	write(file, NULL, (size_t)(pos - file->ous));

      file->oua = pos;
    }
    else if (file->inp) {
      ola = file->ina;

      switch (origin) {
	case SEEK_CUR: pos = file->ina + offset; break;
	case SEEK_END: pos = file->ins + offset; break;
	case SEEK_SET: pos =             offset; break;
	default:     __assume(0);                break;
      }

      if (pos < 0)
	pos = 0;
      if (pos > file->ins)
	pos = file->ins;

      file->ina = pos;
    }

    return 0;
  }
};

/* ------------------------------------------------------------ */

set<string> cachezip;
map<string, zipdir> directories;

bool __cdecl iszip(const char *pathname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      return true;
    }
  }

  char *walk = strdup(pathname);
  char *shrt = walk + strlen(walk);

  while (shrt) {
    *shrt = '\0';
    
    /* allows bsas inside zips */
    if (isext(walk, "bsa"))
      break;
    /* direct hit? */
    if (isext(walk, "zip")) {
      cachezip.insert(walk);

      free(walk);
      return true;
    }

    shrt = strrchr(walk, '\\');
  }

  free(walk);
  return false;
}

bool __cdecl iszip(struct io::dir *dir) {
  return !!dir->zd;
}

bool __cdecl iszip(struct io::file *file) {
  return !!file->zf;
}

/* ------------------------------------------------------------ */

int __cdecl stat_zip(const char *pathname, struct io::info *info) {
  int ret = -1;

  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);
      if ((*pathname) == '\0')
	info->io_type = IO_ZIP,
	info->io_size = 0,
	info->io_raws = 0,
	info->io_time = zip.ziptime,
	ret = 0;
      else if ((*pathname) == '\\')
	ret = zip.stat(pathname + 1, info);

      break;
    }
  }

  return ret;
}

/* ------------------------------------------------------------ */

int __cdecl mkdir_zip(const char *pathname) {
  int ret = -1;

  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);
      if ((*pathname) == '\0')
	return zip.mkdir("");
      else if ((*pathname) == '\\')
	return zip.mkdir(pathname + 1);

      break;
    }
  }

  char *walk = strdup(pathname);
  char *shrt = walk + strlen(walk);

  do {
    /* allows bsas inside zips */
    if (isext(walk, "bsa"))
      break;
    /* direct hit? */
    if (isext(walk, "zip")) {
      const char *zipname = walk;
      cachezip.insert(walk);

      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);
      if ((*pathname) == '\0')
	ret = zip.mkdir("");
      else if ((*pathname) == '\\')
	ret = zip.mkdir(pathname + 1);

      break;
    }

    *shrt = '0';
  } while ((shrt = strrchr(shrt, '\\')));
  
  free(walk);
  return ret;
}

int __cdecl rmdir_zip(const char *pathname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);
      if ((*pathname) == '\0') {
	if (zip.loaded) {
	  io::flush();

	  zip.close();
	}

	/* cachezip is the string-owner */
	directories.erase(zipname);
	cachezip.erase(zipname);
	
	/* unlink inside something else */
	IOFL *ifc = iopen(zipname, NULL);
	int ret = ifc->unlink(zipname);
	free(ifc);

	return ret;
      }
      else if ((*pathname) == '\\')
	return zip.rmdir(pathname + 1);

      break;
    }
  }

  return -1;
}

class it_dir_zip {
public:
  set<string> contents;
  set<string>::iterator position;

  const char *zipname;
  zipdir *zip;
  string location;

  struct dirent e;
};

void * __cdecl opendir_zip(const char *pathname) {
  it_dir_zip *r = NULL;

  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);

      r = new it_dir_zip;
      r->zip = &zip;
      r->zipname = zipname;

      if ((*pathname) == '\0') {
	r->location = pathname;
	r->contents = zip.find(pathname);
	r->position = r->contents.begin();
      }
      else if ((*pathname) == '\\') {
	r->location = pathname + 1;
	r->contents = zip.find(pathname + 1);
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

struct dirent * __cdecl readdir_zip(void *dir) {
  it_dir_zip *r = (it_dir_zip *)dir;

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

void __cdecl closedir_zip(void *dir) {
  it_dir_zip *r = (it_dir_zip *)dir;

  if (r->location == "") {
    if (r->zip->loaded) {
      io::flush();

      r->zip->close();
    }

    /* cachezip is the string-owner */
    directories.erase(r->zipname);
    cachezip.erase(r->zipname);
  }

  delete r;
}

void __cdecl free_zip() {
#if 0
  /* TODO */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    /* only throw modified directories */
    zipdir &zip = directories[zipname];
    if (zip.changed())
      directories.erase(zipname);
  }
#endif

  cachezip.clear();
  directories.clear();
}

/* ------------------------------------------------------------ */

int __cdecl rename_zip(const char *srcname, const char *dstname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (srcname == stristr(srcname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      srcname += strlen(zipname);
      dstname += strlen(zipname);

      if ((*srcname) == '\\') srcname++;
      if ((*dstname) == '\\') dstname++;
      
      // TODO: support renames across archive/file-system types
      return zip.rename(srcname, dstname);
    }
  }

  return -1;
}

int __cdecl unlink_zip(const char *pathname) {
  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);

      if ((*pathname) == '\0')
	;
      else if ((*pathname) == '\\')
	return zip.unlink(pathname + 1);

      break;
    }
  }

  return -1;
}

/* ------------------------------------------------------------ */

class it_file_zip {
public:
  const zipfile *file;

  const char *zipname;
  zipdir *zip;
  char zipmode;
};

void * __cdecl fopen_zip(const char *pathname, const char *mode) {
  it_file_zip *r = NULL;

  /* cache-hit? */
  for (set<string>::iterator it = cachezip.begin(); it != cachezip.end(); ++it) {
    const char *zipname = (*it).data();

    if (pathname == stristr(pathname, zipname)) {
      /* load archive into memory */
      zipdir &zip = directories[zipname];
      if (!zip.loaded)
	zip.open(zipname);

      /* remove archive-prefix */
      pathname += strlen(zipname);

      if ((*pathname) == '\0')
	;
      else if ((*pathname) == '\\') {
	const zipfile *file = NULL;
	const char *rmode;

	if ((rmode = strchr(mode, 'w')))
	  file = zip.put(pathname + 1);
	else if ((rmode = strchr(mode, 'r')))
	  file = zip.get(pathname + 1);

	if (file) {
	  r = new it_file_zip;
	  r->zip = &zip;
	  r->zipname = zipname;
	  r->zipmode = rmode[0];
	  r->file = file;
	}
      }

      break;
    }
  }

  return r;
}

size_t __cdecl fread_zip(void *block, size_t elements, size_t size, void *file) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->read(r->file, block, (unsigned int)(elements * size));
}

char __cdecl getc_zip(void *file) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->getc(r->file);
}

size_t __cdecl fwrite_zip(const void *block, size_t elements, size_t size, void *file) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->write(r->file, block, (unsigned int)(elements * size));
}

int __cdecl fputs_zip(const char *str, void *file) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->puts(r->file, str);
}

DWORD __stdcall fclose_zip_async(void *file) {
  it_file_zip *r = (it_file_zip *)file;
  try {
    r->zip->leave(r->file); }
  catch(exception &e) {
    io::rethrow(e.what()); }
  delete file;
  return NULL;
}

void __cdecl fclose_zip(void *file) {
  it_file_zip *r = (it_file_zip *)file;
  /* nowait asynchronous (write-queue) */
  if (r->zipmode == 'w')
    io::dispatch(fclose_zip_async, file);
  else
    fclose_zip_async(file);
}

bool __cdecl feof_zip(void *file) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->eof(r->file);
}

int __cdecl fseek_zip(void *file, long offset, int origin) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->seek(r->file, offset, origin);
}

int __cdecl fstat_zip(void *file, struct io::info *info) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->stat(r->file, info);
}

long int __cdecl ftell_zip(void *file) {
  it_file_zip *r = (it_file_zip *)file;
  return r->zip->tell(r->file);
}

/* ------------------------------------------------------------ */

#include <string>
#include <istream>
#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

class istream_zip : public istringstream {
public:
  class it_file_zip *iface;
};

class ostream_zip : public ostringstream {
public:
  class it_file_zip *iface;
};

istream *openistream_zip(const char *pathname) {
  istream_zip *r = NULL;
  class it_file_zip *iface;

  iface = (class it_file_zip *)fopen_zip(pathname, "rb");
  if (iface) {
    r = new istream_zip;
    r->iface = iface;
    r->flags(ios_base::binary);

    /* copy-over the string ... */
    fread_zip(NULL, 1,  0, r->iface);

    string bin((char *)iface->file->inp, iface->file->ins);

    r->str(bin);
    r->istringstream::seekg(0);
  }

  return r;
}

ostream *openostream_zip(const char *pathname) {
  ostream_zip *r = NULL;
  class it_file_zip *iface;

  iface = (class it_file_zip *)fopen_zip(pathname, "wb+");
  if (iface) {
    r = new ostream_zip;
    r->iface = iface;
    r->flags(ios_base::binary);
  }

  return r;
}

void closeistream_zip(istream *ist) {
  istream_zip *r = (istream_zip *)ist;
  fclose_zip(r->iface);
  delete r;
}

void closeostream_zip(ostream *ost) {
  ostream_zip *r = (ostream_zip *)ost;

//ioflushtofile(r->str().data(), r->str().length(), r->iface);

  /* nowait asynchronous (write-queue) */
//#pragma omp single nowait
  {
    size_t written =

    /* copy-over the string ... */
    fwrite_zip(r->str().data(), 1, r->str().length(), r->iface);
    fclose_zip(r->iface);

    delete r;
  }
}

long tellistream_zip(istream *ist) {
  istream_zip *r = (istream_zip *)ist;
//return ftell_zip(r->iface);
  size_t check = r->str().length();
  size_t verfy = ist->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in);
  assert(check == verfy);
  return (long)check;
}

long tellostream_zip(ostream *ost) {
  ostream_zip *r = (ostream_zip *)ost;
//return ftell_zip(r->iface);
  size_t check = r->str().length();
  size_t verfy = ost->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
  assert(check == verfy);
  return (long)check;
}

bool iszip(istream *ist) {
  return !!dynamic_cast<istream_zip *>(ist);
}

bool iszip(ostream *ost) {
  return !!dynamic_cast<ostream_zip *>(ost);
}
