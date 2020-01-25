#pragma once
#include <Windows.h>
#include <string>

#undef MessageBox

namespace utf8 {

  int MessageBox (HWND hWnd, const std::string& text, const std::string& caption,
    unsigned int type);

  /// A simple buffer for caching values returned by Windows API 
  class buffer {
  public:
    buffer (size_t size_);
    ~buffer ();
    buffer (const buffer& other);
    buffer& operator = (const buffer& rhs);
    buffer& operator = (const std::string& rhs);
    operator wchar_t* ();
    operator std::string () const;
    DWORD size () const;

  private:
    wchar_t *ptr;
    size_t sz;
  };

  /// Return a pointer to buffer
  inline
    buffer::operator wchar_t* ()
  {
    return ptr;
  }

  /// Convert buffer to an UTF-8 encoded string
  inline
    buffer::operator std::string () const
  {
    return narrow (ptr);
  }

  /// Return buffer size
  inline DWORD
    buffer::size () const
  {
    return (DWORD)sz;
  }

}

