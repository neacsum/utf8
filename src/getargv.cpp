#include <utf8/utf8.h>
#include <windows.h>

using namespace std;
namespace utf8 {

/*!
  Converts wide byte command arguments to an array of pointers
  to UTF-8 strings.

  \ingroup utf8

  \param  argc Pointer to an integer that contains number of parameters
  \return array of pointers to each command line parameter

  The space allocated for strings and array of pointers should be freed
  by calling free_utf8argv()
*/
char** get_argv (int *argc)
{
  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), argc);
  char** uargv = new char*[*argc];
  for (int i = 0; i < *argc; i++)
  {
    int nc = WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, 0, 0, 0, 0);
    uargv[i] = new char[nc + 1];
    WideCharToMultiByte (CP_UTF8, 0, wargv[i], -1, uargv[i], nc, 0, 0);
  }
  LocalFree (wargv);
  return uargv;
}

/*!
  Frees the memory allocated by get_argv()
  \ingroup utf8

  \param  argc  number of arguments
  \param  argv  array of pointers to arguments
*/
void free_argv (int argc, char **argv)
{
  for (int i = 0; i < argc; i++)
    delete argv[i];
  delete argv;
}

/*!
  Converts wide byte command arguments to UTF-8 to a vector of UTF-8 strings.
  \ingroup utf8

  \return vector of UTF-8 strings
*/
std::vector<std::string> get_argv ()
{
  int argc;
  vector<string> uargv;

  wchar_t **wargv = CommandLineToArgvW (GetCommandLineW (), &argc);
  for (int i = 0; i < argc; i++)
    uargv.push_back (narrow (wargv[i]));
  LocalFree (wargv);
  return uargv;
}

} //namespace utf8
