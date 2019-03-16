#include <utf8/utf8.h>

using namespace std;

namespace utf8 {

/*!
  Creates, modifies, or removes environment variables.
  This is a wrapper for [_wputenv function]
  (https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/putenv-wputenv)

  \return true if successful, false otherwise.
*/
bool putenv (const std::string & str)
{
  return (_wputenv (utf8::widen (str).c_str ()) == 0);
}

/*!
  Creates, modifies, or removes environment variables.
  This is a wrapper for [_wputenv_s function]
  https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/putenv-s-wputenv-s

  \param var  name of environment variable
  \param val  new value of environment variable. If empty, the environment
              variable is removed
  \return true if successful, false otherwise
*/
bool putenv (const std::string& var, const std::string& val)
{
  return (_wputenv_s (widen (var).c_str (),
    widen(val).c_str()) == 0);
}

/// Gets a value from the current environment
std::string getenv (const std::string & var)
{
  size_t nsz;
  wstring wvar = widen (var);
  _wgetenv_s (&nsz, 0, 0, wvar.c_str ());
  if (!nsz)
    return string ();

  wstring wval (nsz, L'\0');
  _wgetenv_s (&nsz, &wval[0], nsz, wvar.c_str ());
  wval.resize (nsz - 1);
  return narrow (wval);
}

} //namespace utf8
