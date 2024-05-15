/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file ini.cpp Implementation of IniFile class.

//Stop Visual Studio from nagging
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <math.h>        // for atof
#include <assert.h>
#include <utf8/utf8.h>
#include <functional>
#include <filesystem>
#include <thread>

/// Maximum line length for a line in an INI file
#define INI_BUFFERSIZE  1024
using namespace std;

namespace utf8 {

/*!
  \defgroup inifile INI File Replacement API
  An object-oriented replacement for working with INI files

  The basic Windows API functions for reading and writing INI files, 
  [GetPrivateProfileStringW]
  (https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getprivateprofilestringw)
  and [WritePrivateProfileStringW]
  (https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-writeprivateprofilestringw),
  combine both the file name and the information to be read or written in one API call.

  Using the utf8::widen() function to convert all strings passed to this API
  produces an INI file that contains UTF-16 characters.

  The solution is to completely replace the Windows API functions with an IniFile
  object that is UTF-8 aware and provides a rich interface.

  This implementation struggles to be as compatible as possible with the original Windows API.
  There are no arbitrary extensions to the file format and I've done a lot of
  testing to identify different corner cases. Here are the rules I discovered
  by trying different combination of calls to the original Windows API:
  - the only comments lines are the ones starting with a semi-colon
  (hashes are not considered comments by Windows API)
  - there are no trailing comments; anything after the '=' sign is part of the key value
  - leading and trailing spaces are removed both from returned strings and from parameters

  The only changes compared to the Windows API are:
  - line length defaults to 1024 (the INI_BUFFER_SIZE value) while Windows API limits it to 256 characters
  - files without a path are in current directory while Windows API places them in Windows folder
*/

static int enum_keys (FILE* fp, const char *section, std::function<void (const char*)> fun);
static int enum_sections (FILE* fp, std::function<void (const char*)> fun);
static bool findsection (const char *section, FILE *rf, FILE *wf, char *buffer, size_t bsize);
static bool putkey (const char *key, const char *value, const char *section, const char *filename);
static bool getkey (FILE* fp, const char* section, const char* key, char* buffer, size_t BufferSize);
static void writesection (const char* Section, FILE *fp);
static void writekey (const char* key, const char* value, FILE *fp);

static bool tmp_rename (const std::string& filename);
static bool same_file (const std::string& f1, const std::string& f2);

//case insensitive compare functions have different names in VS and GCC
#ifdef _WIN32
#define icompare _stricmp
#define icomparen _strnicmp
#else
#define icompare strcasecmp
#define icomparen strncasecmp
#endif


//----------------------------------------------------------------------------
//  Some string manipulation functions.
//  Template-ized to allow for const/non-const arguments

// Skip leading spaces and non-printing characters
template <class T> static T* skipleading (T* str)
{
  assert (str);
  while (*str > 0 && *str <= ' ')
    str++;
  return str;
}

// Return a pointer to the first trailing space character
template <class T> static T* skiptrailing (T* str)
{
  assert (str);
  T* tail = str;
  unsigned char c;
  while ((c = *str++) != 0)
  {
    if (c > ' ')
      tail = str;
  }
  return tail;
}

// Return string's length not including any trailing spaces
template <class T> static size_t trimmed_len (T* str)
{
  T* endp = skiptrailing<T> (str);
  return endp - str;
}

// Trim a string to the last non-space character
static char *trimtrailing (char *str)
{
  char *end = skiptrailing (str);
  *end = 0;
  return end;
}

//-----------------------------------------------------------------------------
//  File manipulation functions 

inline
static FILE *openread (const std::string& fname)
{
#ifdef _WIN32
  return utf8::fopen (fname, "rb, ccs=UTF-8");
#else
  return fopen (fname.c_str (), "rb");
#endif
}

inline
static FILE *openwrite (const std::string& fname)
{
#ifdef _WIN32
  return utf8::fopen (fname, "wb, ccs=UTF-8");
#else
  return fopen (fname.c_str (), "wb");
#endif

}

/*
  Get a temporary file name to copy to.
  Use the existing name and append a '~' character at the end.
*/
inline
static std::string tempname (const std::string& source)
{
  return source + '~';
}

//-----------------------------------------------------------------------------
/*!
  \class IniFile
  Provides a handy object oriented encapsulation of functions needed to
  manipulate INI (profile) files.
  \ingroup inifile
*/

/// Constructor 
IniFile::IniFile (const std::string& file)
  : temp_file {false}
  /* get the fully qualified path name in case current directory changes after creation */
#ifdef _WIN32
# if USE_WINDOWS_API
  , filename { utf8::fullpath (file) }
# else
  , filename{ narrow (std::filesystem::absolute (widen (file))) }
# endif
#else
  , filename{ std::filesystem::absolute (file) }
#endif
{
}

///  Creates a temporary file as filename.
IniFile::IniFile ()
  : temp_file {true}
#if USE_WINDOWS_API
  , filename (utf8::GetTempFileName(".", "INI", 0))
#else
  , filename (tmpnam(NULL))
#endif
{
}

/// Copy constructor
IniFile::IniFile (const IniFile& p)
  : filename {p.filename}
  , temp_file {false}
{
}

/// Destructor. If this was a temporary file it is deleted now.
IniFile::~IniFile()
{
  if (temp_file)
    std::filesystem::remove (filename);
}

/*!
  Changes the file associated with this object. If previous one was a
  temporary file, it is deleted now (loosing all settings in the process).
  
  \param fname New file name. If empty it creates a temporary file.
*/
void IniFile::File (const std::string& fname)
{
  if (temp_file)
    std::filesystem::remove (filename);

  if (!fname.empty())
  {
    filename = fname;
    temp_file = false;
  }
  else
  {
#if USE_WINDOWS_API
    wchar_t tmp[_MAX_PATH];
    GetTempFileNameW (L".", L"INI", 0, tmp);
    filename = utf8::narrow (tmp);
#else
    filename = tmpnam (NULL);
#endif
    temp_file = true;
  }
}

///  Assignment operator performs a file copy of the passed object.
IniFile& IniFile::operator = (const IniFile& p)
{
  // Copy the source file to the destination file.
#if USE_WINDOWS_API
  CopyFile (p.filename, filename, false);
#else
  std::filesystem::copy (p.filename, filename, std::filesystem::copy_options::overwrite_existing);
#endif
  return *this;
}

/*!
  Return \b true if specified key exists in the INI file.
*/
bool IniFile::HasKey (const std::string& key, const std::string& section) const
{
  char buffer[80];
  return (GetString (buffer, sizeof(buffer), key, section) != 0);
}

/*!
  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
*/
int IniFile::GetInt (const std::string& key, const std::string& section, int defval) const
{
  char buffer[80];
  return GetString (buffer, sizeof (buffer), key, section) ? atoi (buffer) : defval;
}

/*!
  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
*/
double IniFile::GetDouble (const std::string& key, const std::string& section, double defval) const
{
  char value[80];
  return GetString (value, sizeof (value), key, section) ? atof (value) : defval;
}

/*!
  \param key      key name
  \param value    key value
  \param section  section name
  \return         true if successful, false otherwise
*/
bool IniFile::PutInt (const std::string& key, long value, const std::string& section)
{
  return PutString (key, std::to_string (value), section);
}


/*!
  \param key      key name
  \param value    key value
  \param section  section name
  \param dec      number of decimals
  \return         true if successful, false otherwise
*/
bool IniFile::PutDouble (const std::string& key, double value, const std::string& section, int dec)
{
  char buffer[80];
  sprintf (buffer, "%.*lf", dec, value);
  return PutString (key, buffer, section);
}

#ifdef _WIN32
/*!
  Font is specified by a string containing the following comma-separated values:
  height, width, escapement, orientation, weight, italic, underline, strikeout,
  charset, precision, clip precision, quality, pitch and family, face name.

  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
*/
HFONT IniFile::GetFont (const std::string& key, const std::string& section, HFONT defval) const
{
  LOGFONTW lfont;
  auto value = GetString (key, section);
  auto ptr = value.c_str();

  if (*ptr)
  {
    lfont.lfHeight = atoi (ptr);
    ptr = strchr (ptr, ',');
  }
  else if (defval)
    return defval;
  else
    return (HFONT)GetStockObject(SYSTEM_FONT);

  if (ptr && *ptr)
  {
    lfont.lfWidth = atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfWidth = 0;
  if (ptr && *ptr)
  {
    lfont.lfEscapement = atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfEscapement = 0;
  if (ptr && *ptr)
  {
    lfont.lfOrientation = atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfOrientation = 0;
  if (ptr && *ptr)
  {
    lfont.lfWeight = atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfWeight = FW_NORMAL;
  if (ptr && *ptr)
  {
    lfont.lfItalic = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfItalic = 0;
  if (ptr && *ptr)
  {
    lfont.lfUnderline = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfUnderline = 0;
  if (ptr && *ptr)
  {
    lfont.lfStrikeOut = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfStrikeOut = 0;
  if (ptr && *ptr)
  {
    lfont.lfCharSet = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfCharSet = ANSI_CHARSET;
  if (ptr && *ptr)
  {
    lfont.lfOutPrecision = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
  if (ptr && *ptr)
  {
    lfont.lfClipPrecision = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  if (ptr && *ptr)
  {
    lfont.lfQuality = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfQuality = DEFAULT_QUALITY;
  if (ptr && *ptr)
  {
    lfont.lfPitchAndFamily = (BYTE)atoi (++ptr);
    ptr = strchr (ptr, ',');
  }
  else
    lfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
  if (ptr && *ptr)
    wcscpy_s (lfont.lfFaceName, utf8::widen(++ptr).c_str() );
  else
    wcscpy_s (lfont.lfFaceName, L"Courier");
  return CreateFontIndirectW (&lfont);
}

/*!
  Color is assumed to be in the same format as written by PutColor i.e. 
  R G B numbers separated by spaces.

  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
*/
COLORREF IniFile::GetColor (const std::string& key, const std::string& section, COLORREF defval) const
{
  int R=0, G=0, B=0;
  char value[80];

  if (GetString (value, sizeof(value), key, section)
   && sscanf_s (value, "%i%i%i", &R, &G, &B) == 3)
    return RGB (R,G,B);
  else
    return defval;
}

/*!
  Color is saved as RGB numbers separated by spaces.

  \param key      key name
  \param section  section name
  \param c        color specification
  \return         true if successful, false otherwise
*/
bool IniFile::PutColor (const std::string& key, COLORREF c, const std::string& section)
{
  char buffer[80];

  sprintf_s (buffer, "%i %i %i", GetRValue(c), GetGValue(c), GetBValue(c));
  return PutString (key, buffer, section);
}


/*!
  The format is the same as that interpreted by GetFont

  \param key      key name
  \param section  section name
  \param font     key value
  \return         true if successful, false otherwise
*/
bool IniFile::PutFont (const std::string& key, HFONT font, const std::string& section)
{
  LOGFONTW lfont;
  char buffer[256];

  if (!GetObjectW (font, sizeof(lfont), &lfont))
    return false;
  sprintf_s (buffer, "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%s",
    lfont.lfHeight,
    lfont.lfWidth,
    lfont.lfEscapement,
    lfont.lfOrientation,
    lfont.lfWeight,
    lfont.lfItalic,
    lfont.lfUnderline,
    lfont.lfStrikeOut,
    lfont.lfCharSet,
    lfont.lfOutPrecision,
    lfont.lfClipPrecision,
    lfont.lfQuality,
    lfont.lfPitchAndFamily,
    utf8::narrow(lfont.lfFaceName).c_str());
  return PutString (key, buffer, section);
}
#endif

/*!
  True values can be specified by any of "on", "yes", "true" or "1".
  Anything else is considered as FALSE. Strings are case-insensitive.

  \param key      key name
  \param section  section name
  \param defval   default value if key is missing
*/
bool IniFile::GetBool (const std::string& key, const std::string& section, bool defval) const
{
  char buffer[80];

  if (!GetString (buffer, sizeof(buffer), key, section))
    return defval;
  return (!icompare (buffer, "on") 
       || !icompare (buffer, "yes") 
       || !icompare (buffer, "true")
       || (atoi (buffer) == 1));
}

/*!
  Boolean values are encoded as "On" "Off" strings.

  \param key      key name
  \param section  section name
  \param value    key value
  \return         true if successful, false otherwise
*/
bool IniFile::PutBool (const std::string& key, bool value, const std::string& section)
{
  return PutString (key, (value)?"On":"Off", section);
}

/*!
  Copy a whole section form another INI file.
  \param  from_file   source INI file
  \param  from_sect   source section
  \param  to_sect     destination section
  \return             true if successful, false otherwise

  If \p to_sec is empty the destination section is the same as the source section.

  The previous content of destination section is erased.

  \pre from_sect cannot be empty
*/
bool IniFile::CopySection (const IniFile& from_file, const std::string& from_sect, const std::string& to_sect)
{
  assert (!from_sect.empty());
  char buffer[INI_BUFFERSIZE];

  //trivial case: same file, same section
  if (same_file(filename, from_file.filename) && (to_sect.empty() || from_sect == to_sect) )
    return true;

  string dest_sect = to_sect.empty () ? from_sect : to_sect;

  FILE *f_out = NULL;
  FILE *f_to = NULL;
  FILE *f_from = openread (from_file.filename);
  if (f_from == NULL)
    return false;

  //locate [from section]
  if (!findsection (from_sect.c_str (), f_from, NULL, buffer, sizeof (buffer)))
  {
    fclose (f_from);   // from_sect not found
    return true;
  }

  if (!std::filesystem::exists (filename))
  {
    //if destination file doesn't exist create it now
    f_out = openwrite (filename);
    if (f_out == NULL)
    {
      fclose (f_from);
      return false;
    }
    fputs ("\xEF\xBB\xBF\r\n", f_out); //write BOM mark
  }
  else
  {
    //destination file exists -> copy to temporary
    f_to = openread (filename);
    if (!f_to)
    {
      fclose (f_from);
      return false;
    }
    f_out = openwrite (tempname (filename));
    if (!f_out)
    {
      fclose (f_from);
      fclose (f_to);
      return false;
    }
    //copy everything up to destination section
    findsection (dest_sect.c_str (), f_to, f_out, buffer, sizeof (buffer));
  }
  writesection (dest_sect.c_str (), f_out);

  //copy [from_section]
  while (fgets (buffer, sizeof (buffer), f_from) && *skipleading (buffer) != '[')
    fputs (buffer, f_out);

  fclose (f_from);

  //copy any remaining content
  if (f_to)
  {
    char chr = 0;
    //skip over previous content of destination section
    while (fgets (buffer, sizeof (buffer), f_to)
      && ((chr = *skipleading (buffer)) != '['))
      ;
    if (chr == '[')
    {
      fputs ("\n", f_out); // keep trailing newline at end of section
      fputs (buffer, f_out);
    }
    while (fgets (buffer, sizeof (buffer), f_to))
      fputs (buffer, f_out);
    fclose (f_to);
    fclose (f_out);
    return tmp_rename (filename);
  }
  else
  {
    fclose (f_out);
    return true;
  }
}

/*!
  \param section  section name
*/
bool IniFile::HasSection (const std::string& section)
{
  char buffer[256];  //Doesn't matter if buffer is small. We want to check only
              //if there are any entries

  return GetKeys (buffer, sizeof(buffer), section) != 0;
}

/*!
  \param key      key name
  \param section  section name
  \return         true if successful, false otherwise
*/
bool IniFile::DeleteKey (const std::string& key, const std::string& section)
{
  return putkey (key.c_str (), NULL, section.c_str (), filename.c_str ());
}

/*!
  \param section  section name
*/
bool IniFile::DeleteSection (const std::string& section)
{
  return putkey (NULL, NULL, section.c_str (), filename.c_str ());
}

/*!
  Key names are returned as null-terminated strings followed by one final null.

  \param keys     buffer for returned keys
  \param sz       size of buffer
  \param section  section name
  \return number of keys in section
*/
int IniFile::GetKeys (char *keys, size_t sz, const std::string& section)
{
  FILE* fp;

  if (keys == 0 || sz == 0)
    return 0;

  if (!(fp = openread(filename)))
    return 0;

  int cnt = 0;
  sz -= 2;   //leave space for terminating NULL

  auto f = [&keys, &sz] (const char *k) 
    {
      size_t l = min (strlen(k), sz);
      strncpy (keys, k, sz);
      keys += l;
      *keys++ = 0;
      sz -= l + 1;
    };
  cnt = enum_keys (fp, section.c_str (), f);
  *keys++ = 0; // append one final null
  fclose (fp);
  return cnt;
}

/*!
  Key names are returned as a deque array.

  \param keys     container for returned keys
  \param section  section name
  \return number of keys in section
*/
size_t IniFile::GetKeys (std::deque<std::string>& keys, const std::string& section)
{
  FILE* fp;

  if (!(fp = openread (filename)))
    return 0;

  keys.clear ();

  auto f = [&keys](const char *key) {keys.push_back (key); };
  int cnt = enum_keys (fp, section.c_str (), f);

  fclose (fp);
  return cnt;
}

/*!
  \param value    buffer for returned string
  \param len      length of buffer
  \param key      key name
  \param section  section name
  \param defval   default value
  \return         length of returned string
*/
size_t IniFile::GetString (char *value, size_t len, const std::string& key, const std::string& section, const std::string& defval) const
{
  FILE *fp;
  bool found = false;

  if (!value || !len)
    return 0;
  fp = openread (filename);
  if (fp) 
  {
    found = getkey (fp, section.c_str(), key.c_str(), value, len);
    fclose(fp);
  }
  if (!found)
  {
    strncpy (value, defval.c_str (), len - 1);
    value[len - 1] = 0;
  }
  return strlen (value);
}

/*!
  \param key      key name
  \param section  section name
  \param defval   default value
  \return         key value
*/
std::string IniFile::GetString (const std::string& key, const std::string& section, const std::string& defval) const
{
  FILE *fp;
  bool found = false;
  char buffer[INI_BUFFERSIZE];
  fp = openread (filename);
  if (fp)
  {
    found = getkey (fp, section.c_str(), key.c_str(), buffer, sizeof(buffer));
    fclose (fp);
  }
  return found ? buffer : defval;
}


/*!
  \param value    key value
  \param key      key name
  \param section  section name
  \return         true if successful, false otherwise
*/
bool IniFile::PutString (const std::string& key, const std::string& value, const std::string& section)
{
  return putkey (key.c_str (), value.c_str (), section.c_str (), filename.c_str ());
}

/*!
  Section names are returned as null-terminated strings followed by one 
  final null.

  \param sects    buffer for returned keys
  \param sz       size of buffer
  \return         number of sections found
  \pre sects !=NULL

  The number of sections returned is the total number of sections in the INI
  file even if the buffer is too small to hold sections' names.
*/

size_t IniFile::GetSections (char *sects, size_t sz)
{
  assert (sects);
  assert (sz);

  FILE *fp = openread (filename);
  int cnt = 0;

  auto f = [&sects, &sz](const char *s)
  {
    if (sz)
    {
      size_t l = min (strlen (s)+1, sz-1);
      strncpy (sects, s, sz);
      sects += l;
      *sects = 0;
      sz -= l;
    }
  };

  sz--; //leave space for final null
  if (fp)
  {
    cnt = enum_sections (fp, f);
    fclose (fp);
  }
  else
    *sects++ = 0;
  *sects = 0; //terminating null
  return cnt;
}

/*!
  Section names are returned as a deque of strings

  \param sects    deque of sections
  \return         number of sections found
*/
size_t IniFile::GetSections (std::deque<std::string>& sects)
{
  FILE *fp = openread (filename);
  int cnt = 0;
  auto f = [&sects](const char *s) {sects.push_back (s); };

  sects.clear ();
  if (fp)
  {
    cnt = enum_sections (fp, f);
    fclose (fp);
  }
  return cnt;
}

// Invoke an enumeration function on each section of an INI file
static int enum_sections (FILE *fp, std::function<void (const char *str)> func)
{
  char buffer[INI_BUFFERSIZE];
  int cnt = 0;
  char *sp, *ep;
  while (fgets (buffer, sizeof (buffer), fp))
  {
    sp = skipleading (buffer);
    if (*sp++ == '[' && (ep = strchr (sp, ']')))
    {
      *ep = 0;
      trimtrailing (sp);
      func (sp);
      cnt++;
    }
  }
  return cnt;
}

// Invoke an enumeration function on each key in a section
static int enum_keys (FILE* fp, const char *section, std::function<void(const char*)> fun)
{
  char buffer[INI_BUFFERSIZE];
  int cnt = 0;
  section = skipleading (section);
  if (!findsection (section, fp, NULL, buffer, sizeof(buffer)))
    return 0;

  //Start enumerating keys
  char *sp;
  while (fgets (buffer, sizeof (buffer), fp) && *(sp = skipleading (buffer)) != '[')
  {
    char *ep;
    if (*sp == ';' || !(ep=strchr (sp, '='))) //ignore comment or malformed lines
      continue;
    *ep = 0;
    *skiptrailing (sp) = 0;
    fun (sp);
    cnt++;
  }
  return cnt;
}

/*!
  Locate the beginning of a section in an INI file
  \param section section to find
  \param rf input file handle
  \param wf output file handle
  \param buffer line reading buffer
  \param bsize size of line reading buffer
  \return true if section was found

  If wf is not NULL all lines read while going through the input file
  are written to the output file.

  The section line itself (if found) is not written to the output file.
*/
static bool findsection (const char *section, FILE *rf, FILE *wf, char *buffer, size_t bsize)
{
  const char *sp;
  size_t len;

  assert (section);
  assert (rf);

  section = skipleading (section);
  len = trimmed_len (section);

  while (true)
  {
    if (!fgets (buffer, (int)bsize, rf))
      return false;
    sp = skipleading (buffer);
    if (*sp == '[' && strchr(buffer, ']'))
    {
      sp = skipleading (sp + 1);
      
      if (!icomparen (sp, section, len))
        return true;
    }
    if (wf)
      fputs (buffer, wf);
  }
}

/*
  Write a string in the INI file. This is the back engine used by all Put... functions.

  \param  key       name of key to write
  \param  value     key value
  \param  section   section name
  \param  filename  name of INI file
  \return           true if successful, false otherwise

  All parameters are UTF8 encoded.
  If \p key is NULL the whole section of the INI file is deleted
  if \p value is NULL the key is deleted.
*/
static bool putkey (const char *key, const char *value, const char *section, const char *filename)
{
  FILE* rfp;
  FILE* wfp;

  char buffer[INI_BUFFERSIZE];
  const char* sp;
  size_t len;

  assert (section);

  if (!(rfp = openread(filename))) 
  {
    /* If the .ini file doesn't exist, make a new file */
    if (key && value) 
    {
      if (!(wfp = openwrite (filename)))
        return false;

      fputs ("\xEF\xBB\xBF\r\n", wfp); //write BOM mark
      writesection(section, wfp);
      writekey(key, value, wfp);
      fclose(wfp);
    }
    return true;
  }

  /* If parameters 'key' and 'value' are valid (so this is not an "erase" request)
     and the setting already exists and it already has the correct value, do
     nothing. This early bail-out avoids rewriting the INI file for no reason. */
  if (key && value
    && getkey (rfp, section, key, buffer, sizeof (buffer))
    && !strcmp (buffer, value))
  {
    fclose (rfp);
    return true;
  }
  // key not found, or different value -> proceed (but rewind the input file first)
  fseek (rfp, 0, SEEK_SET);

  if (!(wfp = openwrite(tempname(filename)))) 
  {
    fclose (rfp);
    return false;
  }

  /* Move through the file one line at a time until a section is matched
     or until EOF. Copy to output file as it is read. */
  if (findsection (section, rfp, wfp, buffer, sizeof(buffer)))
  {
    if (key)
    {
      // make sure section line is terminated with '\n'
      char *ep = buffer + strlen (buffer) - 1;
      if (*ep != '\n')
      {
        *ep++ = '\n';
        *ep = 0;
      }
      fputs (buffer, wfp); //write section line

      //start searching for key
      key = skipleading (key);
      len = trimmed_len (key);
      while ( (sp=fgets (buffer, sizeof (buffer), rfp))  // not end of file
           && *(sp = skipleading (buffer)) != '['   // not end of section
           && (!strchr (buffer, '=') || icomparen (key, sp, len))) //key not found
        fputs (buffer, wfp);

      if (value)
        writekey (key, value, wfp);
      //otherwise we just don't write the key in the output file
    }
    else
    {
      //deleting the section -> skip all entries until next section or end of file 
      while ((sp = fgets (buffer, sizeof (buffer), rfp)) && *(sp = skipleading (buffer)) != '[')
        ;
    }
    if (sp && *sp == '[')
      fputs (buffer, wfp); //write next section line
    // Copy the rest of the INI file
    while (fgets (buffer, sizeof (buffer), rfp))
      fputs (buffer, wfp);
  }
  else if (key && value)
  {
    fputs ("\n", wfp);  /* force a new line behind the last line of the INI file */
    writesection (section, wfp);
    writekey (key, value, wfp);
  }
  else
  {
    //delete section or key for inexistent section
    fclose (rfp);
    fclose (wfp);
    std::filesystem::remove (tempname (filename));
    return true;
  }
  fclose (rfp);
  fclose (wfp);
  return tmp_rename (filename);  // clean up and rename
}

/*!
  Read a key string in a C-style buffer.
  \param  fp      input file
  \param  section section name
  \param  key     input key
  \param  val     key value buffer
  \param  size    length of value buffer

  \return true if key was found; false otherwise
*/
static bool getkey(FILE *fp, const char *section, const char *key, char *val, size_t size)
{
  const char *sp;
  char *vs, *ve;  //value start /end pointers
  size_t len;
  char buffer[INI_BUFFERSIZE];
  assert (fp);
  assert (section);
  assert (key);
  
  // Move through file 1 line at a time until the section is matched or EOF.
  if (!findsection (section, fp, NULL, buffer, sizeof(buffer)))
    return false;

  /* Now that the section has been found, find the entry.
     Stop searching upon leaving the section's area. */
  key = skipleading (key);
  len = trimmed_len (key);
  bool found = false;
  do 
  {
    if (!fgets (buffer, sizeof (buffer), fp) || *(sp = skipleading (buffer)) == '[')
      return false;
    if (*sp == ';' || !(vs = strchr (buffer, '=')))  //Ignore comment or malformed lines
      continue;
    found = !icomparen (sp, key, len);
  } while (!found);

  // Copy up to 'size' chars to buffer
  assert (vs != NULL);
  vs = skipleading(vs + 1);
  ve = skiptrailing (vs);
  *ve = 0;
  strncpy (val, vs, size-1);
  val[size - 1] = 0;
  return true;
}

/// Writes a section entry
static void writesection (const char* section, FILE *fp)
{
  char buffer[INI_BUFFERSIZE];
  assert (section);
  section = skipleading (section);
  if (strlen (section) > 0)
  {
    buffer[0] = '[';
    strncpy (buffer + 1, section, sizeof (buffer) - 5 );
    trimtrailing (buffer);
    strcat (buffer, "]\r\n");
    fputs (buffer, fp);
  }
}

/// Writes a key entry
static void writekey (const char* key, const char* value, FILE *fp)
{
  char buffer[INI_BUFFERSIZE];
  char *p;
  key = skipleading (key);
  strncpy (buffer, key, sizeof (buffer) - 3);
  p = trimtrailing (buffer);
  *p++ = '=';
  strncpy (p, skipleading(value), sizeof (buffer) - (p - buffer) - 3);
  p = trimtrailing (p);
  *p++ = '\r';  *p++ = '\n';
  *p = 0;
  fputs (buffer, fp);
}


/// Renames the output (temporary) file to input file name.
/// Previous input file is deleted.
static bool tmp_rename (const std::string& filename)
{
  const int RETRIES = 50;
  int i;
  string tmpname = tempname (filename);

  /* Had an issue with Kaspersky anti-virus that seems to keep opened the old
     input file making the rename operation to fail. Solved by adding a few
     retries before failing. */
  i = 0;
  while (i++ < RETRIES && !utf8::remove(filename))
    std::this_thread::yield ();

  if (i >= RETRIES)
    return false;

  i = 0;
  while (i++ < RETRIES && !utf8::rename(tmpname, filename))
    std::this_thread::yield ();

  return (i < RETRIES);
}

/// Return true if 2 file names refer to the same file
static bool same_file (const std::string& f1, const std::string& f2)
{
  if (!std::filesystem::exists(f1))
    return !std::filesystem::exists (f2); // both files don't exist

  if (!std::filesystem::exists (f2))
    return false; //first file exists, 2nd doesn't

  return std::filesystem::equivalent (f1, f2);
}

}

