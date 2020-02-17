#include <utf8/utf8.h>
#include <algorithm>

using namespace std;


namespace utf8 {

/*!
  Convert UTF-8 string to lower case using case folding table published by
  Unicode Consortium. (http://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)
  \param str UTF-8 string to convert to lowercase.
  \return lower case UTF-8 string
*/

std::string tolower (const std::string& str)
{
  //definition of 'u2l' and 'lc' tables
  #include "uppertab.c"
  u32string wstr = runes (str);
  for (auto ptr = wstr.begin (); ptr < wstr.end (); ptr++)
  {
    char32_t *f = lower_bound (begin (u2l), end (u2l), *ptr);
    if (f != end (u2l) && *f == *ptr)
      *ptr = lc[f - u2l];
  }
  return narrow (wstr);
}

/// In place version converts a UTF-8 encoded string to lowercase
void tolower (std::string& str)
{
  str = tolower (const_cast<const string&>(str));
}

/*!
  Convert UTF-8 string to upper case using case folding table published by
  Unicode Consortium. (http://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)
  \param str UTF-8 string to convert to uppercase.
  \return upper case UTF-8 string
*/
std::string toupper (const std::string& str)
{
  // definition of 'l2u' and 'uc' tables
  #include "lowertab.c"
  u32string wstr = runes (str);
  for (auto ptr = wstr.begin(); ptr < wstr.end(); ptr++)
  {
    char32_t *f = lower_bound (begin (l2u), end (l2u), *ptr);
    if (f != end(l2u) && *f == *ptr)
      *ptr = uc[f - l2u];
  }
  return narrow (wstr);
}

/// In place version converts a UTF-8 encoded string to lowercase
void toupper (std::string& str)
{
  str = toupper (const_cast<const string&>(str));
}

}
