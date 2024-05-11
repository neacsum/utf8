/*
  (c) Mircea Neacsu 2014-2023. Licensed under MIT License.
  See README file for full license terms.
*/

/// \file utf8.h UTF-8 Conversion functions
#pragma once

#include <string>
#include <vector>

// If USE_WINDOWS_API is not zero, the library issues direct Windows API
// calls. Otherwise it relies only on standard C++17 functions.
// If not defined, USE_WINDOWS_API defaults to 1 on Windows platform.

#define USE_WINDOWS_API 0


#if defined (_WIN32) && !defined (USE_WINDOWS_API)
#define USE_WINDOWS_API 1
#elif !defined (USE_WINDOWS_API)
#define USE_WINDOWS_API 0
#endif

#if USE_WINDOWS_API
#pragma message ("Using Windows API")
#endif

namespace utf8 {

/// Exception thrown on encoding/decoding failure
struct exception : public std::exception
{
  /// Possible causes
  enum reason { invalid_utf8, invalid_wchar, invalid_char32 };

  /// Constructor
  explicit exception (reason c)
    : why (c)
  {}
  const char* what() const noexcept
  {
    return (why == reason::invalid_utf8)   ? "Invalid UTF-8 encoding"
         : (why == reason::invalid_wchar)  ? "Invalid UTF-16 encoding"
         : (why == reason::invalid_char32) ? "Invalid code-point value"
         : "Other UTF-8 exception";
  }

  /// What triggered the exception
  reason why;
};

/// Replacement character used for invalid encodings
const char32_t REPLACEMENT_CHARACTER = 0xfffd;


/// \addtogroup basecvt
/// @{
std::string narrow (const wchar_t* s, size_t nch=0);
std::string narrow (const std::wstring& s);
std::string narrow (const char32_t* s, size_t nch = 0);
std::string narrow (const std::u32string& s);
std::string narrow (char32_t r);

std::wstring widen (const char* s, size_t nch = 0);
std::wstring widen (const std::string& s);
std::u32string runes (const char* s, size_t nch = 0);
std::u32string runes (const std::string& s);

char32_t rune (const char* p);
char32_t rune (const std::string::const_iterator& p);
/// @}

bool is_valid (const char* p);
bool is_valid (std::string::const_iterator p, const std::string::const_iterator last);
bool valid_str (const char* s, size_t nch = 0);
bool valid_str (const std::string& s);

char32_t next (std::string::const_iterator& ptr, const std::string::const_iterator last);
char32_t next (const char*& ptr);
char32_t next (char*& p);

char32_t prev (const char*& ptr);
char32_t prev (char*& ptr);
char32_t prev (std::string::const_iterator& ptr, const std::string::const_iterator first);

size_t length (const std::string& s);
size_t length (const char* s);

/*!
  \addtogroup folding
  @{
*/
void make_lower (std::string& str);
void make_upper (std::string& str);
std::string tolower (const std::string& str);
std::string toupper (const std::string& str);
int icompare (const std::string& s1, const std::string& s2);
/// @}

/*!
  \addtogroup charclass
  @{
*/

bool isspace (char32_t r);
bool isspace (const char* p);
bool isspace (std::string::const_iterator p);

bool isblank (char32_t r);
bool isblank (const char* p);
bool isblank (std::string::const_iterator p);

bool isdigit (char32_t r);
bool isdigit (const char* p);
bool isdigit (std::string::const_iterator p);

bool isalnum (char32_t r);
bool isalnum (const char* p);
bool isalnum (std::string::const_iterator p);

bool isalpha (char32_t r);
bool isalpha (const char* p);
bool isalpha (std::string::const_iterator p);

bool isxdigit (char32_t r);
bool isxdigit (const char* p);
bool isxdigit (std::string::const_iterator p);

bool isupper (char32_t r);
bool isupper (const char* p);
bool isupper (std::string::const_iterator p);

bool islower (char32_t r);
bool islower (const char* p);
bool islower (std::string::const_iterator p);
/// @}


// INLINES --------------------------------------------------------------------

/*!
  Check if pointer points to a valid UTF-8 encoding
  \param p pointer to string
  \return `true` if there is a valid UTF-8 encoding at the current pointer position,
          `false` otherwise.
*/
inline
bool is_valid (const char* p)
{
  return next (p) != REPLACEMENT_CHARACTER;
}

/*!
  Check if iterator points to a valid UTF-8 encoding
  \param p    Iterator
  \param last Iterator pointing to end of range
  \return `true` if there is a valid UTF-8 encoding at the current iterator position,
          `false` otherwise.
*/
inline
bool is_valid (std::string::const_iterator p, const std::string::const_iterator last)
{
  return next (p, last) != REPLACEMENT_CHARACTER;
}

/*!
  Conversion from UTF-8 to UTF-32

  \param p pointer to character
  \return UTF-32 encoded character or utf8::REPLACEMENT_CHARACTER (0xfffd)
          if it is an invalid UTF-8 encoding
*/
inline
char32_t rune (const char* p)
{
  return next (p);
}


/*!
  Decodes a UTF-8 encoded character and advances pointer to next character

  \param ptr    <b>Reference</b> to character pointer to be advanced
  \return     decoded character

  If the string contains an invalid UTF-8 encoding, the function returns
  utf8::REPLACEMENT_CHARACTER (0xfffd) and advances pointer to beginning of
  next character or end of string.
*/
inline
char32_t next (char*& ptr)
{
  return next (const_cast<const char*&>(ptr));
}

/*!
  Decrements a character pointer to previous UTF-8 character

  \param ptr    <b>Reference</b> to character pointer to be decremented
  \return       previous UTF-8 encoded character

  If the string contains an invalid UTF-8 encoding, the function returns
  REPLACEMENT_CHARACTER (0xfffd) and pointer remains unchanged.
*/
inline
char32_t prev (char*& ptr)
{
  return prev (const_cast<const char*&>(ptr));
}


/*!
  Verifies if string is a valid UTF-8 encoded string
  \param s character string to verify
  \return `true` if string is a valid UTF-8 encoded string, `false` otherwise
*/
inline
bool valid_str (const std::string& s)
{
  return valid_str (s.c_str (), s.size());
}

/// @copydoc rune()
inline
char32_t rune (const std::string::const_iterator& p)
{
  return rune (&(*p));
}


/*!
  Return true if character is blank(-ish).
  \param p pointer to character to check
  \return `true` if character is blank, `false` otherwise

  Returns `true` if Unicode character has the "White_Space=yes" property in the
  [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
*/
inline
bool isspace (const char* p)
{
  return isspace (rune (p));
}

/// \copydoc isspace(const char* p)
inline
bool isspace (std::string::const_iterator p)
{
  return isspace (rune(p));
}


/*!
  Check if character is space or tab
  \param p pointer to character to check

  \return `true` if character is `\t` (0x09) or is in the "Space_Separator" (Zs)
          category, `false` otherwise.

  See [Unicode Character Database](https://www.unicode.org/Public/UCD/latest/ucd/PropList.txt)
  for a list of characters in the Zs (Space_Separator) category. The function adds
  HORIZONTAL_TAB (0x09 or '\\t') to the space separator category for compatibility
  with standard `isblank (char c)` C function.
*/
inline
bool isblank (const char *p)
{
  return isblank(rune(p));
}

/// \copydoc isblank(const char* p)
inline
bool isblank (std::string::const_iterator p)
{
  return isblank (rune (p));
}

/*!
  Check if character is a decimal digit (0-9)
  \param r character to check
  \return true if character is a digit, false otherwise
*/
inline
bool isdigit (char32_t r)
{
  return '0' <= r && r <= '9';
}

/*!
  Check if character is a decimal digit (0-9)
  \param p pointer to character to check
  \return `true` if character is a digit, `false` otherwise
*/
inline
bool isdigit (const char *p)
{
  return isdigit (rune (p));
}

/// \copydoc isdigit(const char* p)
inline
bool isdigit (std::string::const_iterator p)
{
  return isdigit (rune (p));
}

/*!
  Check if character is an alphanumeric character (0-9 or A-Z or a-z)
  \param r character to check
  \return `true` if character is alphanumeric, `false` otherwise
*/
inline
bool isalnum (char32_t r)
{
  return ('0' <= r && r <= '9') || ('A' <= r && r <= 'Z') || ('a' <= r && r <= 'z');
}

/*!
  Check if character is an alphanumeric character (0-9 or A-Z or a-z)
  \param p pointer to character to check
  \return `true` if character is alphanumeric, `false` otherwise
*/
inline
bool isalnum (const char *p)
{
  return isalnum (rune (p));
}

/// \copydoc isalnum(const char *p)
inline
bool isalnum (std::string::const_iterator p)
{
  return isalnum (rune (p));
}

/*!
  Check if character is an alphabetic character (A-Z or a-z)
  \param r character to check
  \return `true` if character is alphabetic, `false` otherwise
*/
inline
bool isalpha (char32_t r)
{
  return ('A' <= r && r <= 'Z') || ('a' <= r && r <= 'z');
}

/*!
  Return true if character is an alphabetic character (A-Z or a-z)
  \param p pointer to character to check
  \return true if character is alphabetic, false otherwise
*/
inline
bool isalpha (const char *p)
{
  return isalpha (rune (p));
}

/// \copydoc isalpha(const char *p)
inline
bool isalpha (std::string::const_iterator p)
{
  return isalpha (&*p);
}


/*!
  Check if character is a hexadecimal digit (0-9 or A-F or a-f)
  \param r character to check
  \return `true` if character is hexadecimal, `false` otherwise
*/
inline
bool isxdigit (char32_t r)
{
  return ('0' <= r && r <= '9') || ('A' <= r && r <= 'F') || ('a' <= r && r <= 'f');
}

/*!
  Check if character is a hexadecimal digit (0-9 or A-F or a-f)
  \param p pointer to character to check
  \return `true` if character is hexadecimal, `false` otherwise
*/
inline
bool isxdigit (const char *p)
{
  return isxdigit(rune(p));
}

/// \copydoc isxdigit(const char* p)
inline
bool isxdigit (std::string::const_iterator p)
{
  return isxdigit (rune(p));
}

/// \copydoc isupper(const char* p)
inline
bool isupper (std::string::const_iterator p)
{
  return isupper (rune(p));
}

/// \copydoc islower(const char*p)
inline
bool islower (std::string::const_iterator p)
{
  return islower (rune(p));
}


}; //namespace utf8

#ifdef _WIN32
#include <utf8/winutf8.h>
#endif
#include <utf8/ini.h>

#ifdef _MSC_VER
#pragma comment (lib, "utf8")
#endif
