/*!
  \file casecvt.cpp Implementation of toupper and tolower functions

  These functions use the case folding table published by Unicode Consortium.
  A small ancillary program (gen_casetab) converts the original table 
  in two tables of equal size, one with the upper case letters and the other
  with the lower case ones. The upper case table is sorted to allow for
  binary searching. If a code is found in the upper case table, it is replaced
  with the matching code from the lower case.

  Each function takes about 11k for the case folding table. Finding a code takes
  at most 11 comparisons.
*/
/*
  (c) Mircea Neacsu 2014-2020. Licensed under MIT License.
  See README file for full license terms.
*/

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
