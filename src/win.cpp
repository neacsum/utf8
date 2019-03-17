#include <Windows.h>
#include <utf8/utf8.h>


using namespace std;
namespace utf8 {

int MessageBox (HWND hWnd, const std::string& text, const std::string& caption,
  unsigned int type)
{
  return ::MessageBoxW (hWnd, widen (text).c_str (), widen (caption).c_str (), type);
}

}