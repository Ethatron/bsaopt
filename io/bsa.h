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

extern int compresslevel;
extern bool compressbsa;
extern bool srchbestbsa;
extern bool thresholdbsa;
extern int gameversion;

#define MW_BSAHEADER_VERSION 0x00 //!< Version number of an Morrowind BSA
#define OB_BSAHEADER_VERSION 0x67 //!< Version number of an Oblivion BSA
#define F3_BSAHEADER_VERSION 0x68 //!< Version number of a Fallout 3 BSA
#define SK_BSAHEADER_VERSION 0x68 //!< Version number of a Skyrim BSA
#define OX_BSAHEADER_VERSION -OB_BSAHEADER_VERSION //!< Version number of an Oblivion BSA (XBox)
#define FX_BSAHEADER_VERSION -F3_BSAHEADER_VERSION //!< Version number of a Fallout 3 BSA (XBox)
#define SX_BSAHEADER_VERSION -SK_BSAHEADER_VERSION //!< Version number of a Skyrim BSA (XBox)

/* File flags */
#define OB_BSAFILE_NIF  0x0001 //!< Set when the BSA contains NIF files
#define OB_BSAFILE_DDS  0x0002 //!< Set when the BSA contains DDS files
#define OB_BSAFILE_XML  0x0004 //!< Set when the BSA contains XML files
#define OB_BSAFILE_WAV  0x0008 //!< Set when the BSA contains WAV files
#define OB_BSAFILE_MP3  0x0010 //!< Set when the BSA contains MP3 files
#define OB_BSAFILE_TXT  0x0020 //!< Set when the BSA contains TXT files
#define OB_BSAFILE_HTML 0x0020 //!< Set when the BSA contains HTML files
#define OB_BSAFILE_BAT  0x0020 //!< Set when the BSA contains BAT files
#define OB_BSAFILE_SCC  0x0020 //!< Set when the BSA contains SCC files
#define OB_BSAFILE_SPT  0x0040 //!< Set when the BSA contains SPT files
#define OB_BSAFILE_TEX  0x0080 //!< Set when the BSA contains TEX files
#define OB_BSAFILE_FNT  0x0080 //!< Set when the BSA contains FNT files
#define OB_BSAFILE_CTL  0x0100 //!< Set when the BSA contains CTL files

#define OB_BSAFILE_LIP  0x0200 //!< Set when the BSA contains LIP files
#define OB_BSAFILE_BIK  0x0400 //!< Set when the BSA contains BIK files
#define OB_BSAFILE_JPG  0x0800 //!< Set when the BSA contains JPG files

bool __cdecl isarchive(const char *pathname, int *version = NULL);
bool __cdecl isarchive(struct iodir *dir);
bool __cdecl isarchive(struct iofile *file);

#define DELIMITER_BSA  '\\'
#define DELIMITES_BSA  "\\"

/* ------------------------------------------------------------ */

#include <iostream>

using namespace std;

bool __cdecl isarchive(istream *ist);
bool __cdecl isarchive(ostream *ost);

/* ------------------------------------------------------------ */

void __cdecl free_arc();
