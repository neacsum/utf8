/// \file BUFFER.CPP Implementation of utf8::buffer class 

/*
  (c) Mircea Neacsu 2014-2019. Licensed under MIT License.
  See README file for full license terms.
*/
#include <utf8/utf8.h>

using namespace std;

/*!
  \class utf8::buffer
  
  Many Windows API function return output strings as UTF-16 encoded strings.
  This class provides conversion operators that make it easier to go back to
  UTF-8 encoded strings.

  Here is a simple usage example:
````
  utf8::buffer path (_MAX_PATH);
  utf8::buffer fname (_MAX_PATH);

  GetTempPath (path.size (), path);
  GetTempFileName (path, L"ABC", 1, fname);

  string result = fname;
````
  
  Without this class, the equivalent code would be something like:
````
  wstring wpath (_MAX_PATH, L'\0');
  wstring wfname (_MAX_PATH, L'\0');

  GetTempPath (wpath.size (), const_cast<wchar_t*>(wpath.data ()));
  GetTempFileName (wpath.c_str(), L"ABC", 1, const_cast<wchar_t*>(wfname.data ()));

  string result = narrow(wfname);
````

*/
namespace utf8 {

/// Constructor
/// \param size_ buffer size in characters (not bytes)
buffer::buffer (size_t size_)
  : ptr (size_ ? new wchar_t[size_] : nullptr)
  , sz (size_)
{
  if (sz)
    memset (ptr, 0, sz * sizeof (wchar_t));
}

/// Destructor
buffer::~buffer ()
{
  delete ptr;
}

/// Copy constructor
buffer::buffer (const buffer& other)
  : ptr (other.sz ? new wchar_t[other.sz] : nullptr)
  , sz (other.sz)
{
  if (ptr)
    memcpy (ptr, other.ptr, sz * sizeof (wchar_t));
}

/// Principal assignment operator
buffer&
buffer::operator =(const buffer& rhs)
{
  delete ptr;
  sz = rhs.sz;
  if (sz)
    memcpy ((ptr = new wchar_t[sz]), rhs.ptr, sz * sizeof (wchar_t));
  else
    ptr = nullptr;

  return *this;
}

/// String assignment operator
buffer&
buffer::operator =(const std::string& rhs)
{
  wstring wrhs (widen (rhs));
  size_t new_size = wrhs.size () + 1;
  if (new_size > sz)
  {
    delete ptr;
    sz = new_size;
    ptr = new wchar_t[sz];
  }
  memcpy (ptr, wrhs.c_str(), sz * sizeof (wchar_t));
  return *this;
}
}