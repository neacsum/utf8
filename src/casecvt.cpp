#include <utf8/utf8.h>
#include <algorithm>

using namespace std;


namespace utf8 {

/*!
  Convert UTF-8 string to lower case using case folding table published by
  Unicode Consortium. (http://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)
  \param str UTF-8 string to convert to lowercase.
*/
void tolower (std::string& str)
{
#include "uppertab.c"
  u32string istr = runes (str);
  for (auto ptr = istr.begin (); ptr < istr.end (); ptr++)
  {
    char32_t *f = lower_bound (begin(u2l), end(u2l), *ptr);
    if (f != end(u2l) && *f == *ptr)
      *ptr = lc[f - u2l];
  }
  str = narrow (istr);
}

/*!
  Convert UTF-8 string to upper case using case folding table published by
  Unicode Consortium. (http://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)
  \param str UTF-8 string to convert to uppercase.
*/
void toupper (std::string& str)
{
#include "lowertab.c"
  u32string istr = runes (str);
  for (auto ptr = istr.begin(); ptr < istr.end(); ptr++)
  {
    char32_t *f = lower_bound (begin (l2u), end (l2u), *ptr);
    if (f != end(l2u) && *f == *ptr)
      *ptr = uc[f - l2u];
  }
  str = narrow (istr);
}
}
