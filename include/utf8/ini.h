/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file ini.h Definition of IniFile class
#pragma once

#include <Windows.h>
#include <string>
#include <deque>

namespace utf8 {

///Operations on INI files
class IniFile
{
public:
  /// Constructor based on an existing or new INI file.
  IniFile (const std::string& name);

  /// Default constructor uses a temporary file.
  IniFile ();

  /// Copy constructor
  IniFile (const IniFile& p);

  /// Destructor
  ~IniFile ();

  /// Assignment operator
  IniFile& operator= (const IniFile&);

  /// Return file name associated with this object
  const std::string& File () const
    { return filename; };

  /// Set the file name associated with this object
  void File (const std::string& filename);
  
  ///Get a string key
  size_t GetString (char *value, size_t len, const std::string& key, const std::string& section, const std::string& defval = std::string()) const;

  ///Return a string key
  std::string GetString (const std::string& key, const std::string& section, const std::string& defval = std::string ()) const;

  ///Return an integer key
  int GetInt (const std::string& key, const std::string& section, int defval=0) const;

  /// Return a floating point value
  double GetDouble (const std::string& key, const std::string& section, double defval=0.) const;

  ///Return a color specification key
  COLORREF GetColor (const std::string& key, const std::string& section, COLORREF defval=RGB(0,0,0)) const;

  ///Return a boolean key
  bool GetBool (const std::string& key, const std::string& section, bool defval=false) const;

  ///Return a font specification key
  HFONT GetFont (const std::string& key, const std::string& section, HFONT defval=NULL) const;

  ///Check for key existence
  bool HasKey (const std::string& key, const std::string& section) const;

  /// Write a string key
  bool PutString (const std::string& key, const std::string& value, const std::string& section);

  /// Write an integer key
  bool PutInt (const std::string& key, long value, const std::string& section);

  /// Write a font specification key
  bool PutFont (const std::string& key, HFONT font, const std::string& section);

  /// Write a boolean key
  bool PutBool (const std::string& key, bool value, const std::string& section);

  /// Write a color specification key
  bool PutColor (const std::string& key, COLORREF value, const std::string& section);

  /// Write a floating point value key
  bool PutDouble (const std::string& key, double value, const std::string& section, int dec = 2);

  /// Delete a key
  bool DeleteKey (const std::string& key, const std::string& section);

  /// Delete an entire section
  bool DeleteSection (const std::string& section);

  /// Copy all keys from one section to another.
  bool CopySection (const IniFile& from_file, const std::string& from_sect, const std::string& to_sect=std::string());

  /// Return \b true if profile contains a non empty section with the given name
  bool HasSection (const std::string& section);

  /// Retrieve names of all keys in a section.
  int GetKeys (char *buffer, size_t sz, const std::string& section);

  /// Retrieve names of all keys in a section.
  size_t GetKeys (std::deque<std::string>& keys, const std::string& section);

  /// Return the names of all sections in the INI file.
  size_t GetSections (char *sects, size_t sz);

  /// Return the names of all sections in the INI file.
  size_t GetSections (std::deque<std::string>& sections);

private:

  std::string filename;
  bool temp_file;
};


}
