///  \file dir.cpp Directory functions with UTF-8 encoded arguments

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README.md file for full license terms.
*/
#include <utf8/utf8.h>


using namespace std;
namespace utf8 {

/*!
  Creates a new directory

  \param dirname UTF-8 path for new directory
  \return true if successful, false otherwise
*/
bool mkdir (const char* dirname)
{
  return (_wmkdir (widen (dirname).c_str ()) == 0);
}

/*!
  Creates a new directory

  \param dirname UTF-8 path for new directory
  \return true if successful, false otherwise
*/
bool mkdir (const std::string& dirname)
{
  return (_wmkdir (widen (dirname).c_str ()) == 0);
}

/*!
  Deletes a directory

  \param dirname UTF-8 path of directory to be removed
  \return true if successful, false otherwise
*/
bool rmdir (const char* dirname)
{
  return (_wrmdir (widen (dirname).c_str ()) == 0);
}

/*!
  Deletes a directory

  \param dirname UTF-8 path of directory to be removed
  \return true if successful, false otherwise
*/
bool rmdir (const std::string& dirname)
{
  return (_wrmdir (widen (dirname).c_str ()) == 0);
}


/*!
  Changes the current working directory

  \param dirname UTF-8 path of new working directory
  \return true if successful, false otherwise
*/
bool chdir (const char* dirname)
{
  return (_wchdir (widen (dirname).c_str ()) == 0);
}

/*!
  Changes the current working directory

  \param dirname UTF-8 path of new working directory
  \return true if successful, false otherwise
*/
bool chdir (const std::string& dirname)
{
  return (_wchdir (widen (dirname).c_str ()) == 0);
}

/*!
  Gets the current working directory
  \return UTF-8 encoded name of working directory
*/
std::string getcwd ()
{
  wchar_t tmp[_MAX_PATH];
  if (_wgetcwd (tmp, _countof (tmp)))
    return narrow (tmp);
  else
    return string ();
}


}  //namespace utf8
