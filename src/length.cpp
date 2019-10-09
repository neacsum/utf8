/// \file LENGTH.CPP UTF-8 character counting.

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README.md file for full license terms.
*/
#include <utf8/utf8.h>

using namespace std;
namespace utf8 {
/*!
  Counts number of characters in an UTF8 encoded string
  
  \param s UTF8-encoded string
  \return number of characters in string

  \note Algorithm from http://canonical.org/~kragen/strlen-utf8.html
*/
size_t length (const std::string& s)
{
  size_t nc = 0;
  auto p = s.begin ();
  while (p != s.end ())
  {
    if ((*p++ & 0xC0) != 0x80)
      nc++;
  }
  return nc;
}

} //namespace utf8
