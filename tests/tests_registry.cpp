#include <utpp/utpp.h>
#include <utf8/utf8.h>

const std::string key_name{ u8"αρχείο" };//Greek for "registry" according to Google

SUITE (registry)
{

TEST (create_open)
{
  HKEY key;
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key));
  CHECK(!RegCloseKey (key));

  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegOpenKey (HKEY_CURRENT_USER, key_name, key));
  CHECK(!RegCloseKey (key));
  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

TEST (rename_delete_tree)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegRenameKey (HKEY_CURRENT_USER, key_name, u8"पंजाबी"));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegDeleteTree (HKEY_CURRENT_USER, u8"पंजाबी"));
}

TEST (set_get_query_value)
{
  HKEY key;
  DWORD sz;
  DWORD data = 123, data1 = 0, data2 = 0;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);

  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegSetValue (key, "dword_value", REG_DWORD, &data, sizeof (DWORD)));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegGetValue (key, "", "dword_value", RRF_RT_ANY, &data1, &sz));
  CHECK_EQUAL (data, data1);

  std::string sval = "0123456789", sval1;
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegSetValue (key, "string_value", sval));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegGetValue (key, "", "string_value", sval1));
  CHECK_EQUAL (sval, sval1);

  DWORD type;
  sz = sizeof (DWORD);
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegQueryValue (key, "dword_value", &type, &data2, &sz));
  CHECK_EQUAL (REG_DWORD, type);

  CHECK(!RegCloseKey (key));
  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

TEST (set_get_vector)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);

  std::vector<std::string> in_vect = { "string 1", "string 2", "string 3" };
  std::vector<std::string> out_vect;

  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegSetValue (key, "string_vector", in_vect));
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegGetValue (key, "", "string_vector", out_vect));
  CHECK_EQUAL (in_vect, out_vect);

  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

TEST (enum_keys)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);
  
  std::vector<std::string>in_name{ u8"α1", u8"β2", u8"γ3", u8"😃😎😛"};
  std::vector<std::string>out_name(4);

  for (int i = 0; i < 4; i++)
  {
    HKEY subkey;
    CHECK(!utf8::RegCreateKey (key, in_name[i], subkey));
  }

  for (int i = 0; i < 4; i++)
    CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumKey (key, i, out_name[i]));

  CHECK_EQUAL (in_name, out_name);

  //2nd function
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumKey (key, out_name));
  CHECK_EQUAL (in_name, out_name);

  CHECK(!utf8::RegDeleteTree (HKEY_CURRENT_USER, key_name));
}

TEST (enum_values)
{
  HKEY key;
  utf8::RegCreateKey (HKEY_CURRENT_USER, key_name, key);

  std::vector<std::string>in_vals{ u8"α1", u8"β2", u8"γ3", u8"😃😎😛" };
  std::vector<std::string>out_vals (4);

  for (int i = 0; i < 4; i++)
    CHECK(!utf8::RegSetValue (key, in_vals[i], in_vals[3 - i]));

  for (int i = 0; i < 4; i++)
    CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumValue (key, i, out_vals[i]));

  CHECK_EQUAL (in_vals, out_vals);

  //2nd function
  CHECK_EQUAL (ERROR_SUCCESS, utf8::RegEnumValue (key, out_vals));
  CHECK_EQUAL (in_vals, out_vals);

  CHECK(!utf8::RegDeleteKey (HKEY_CURRENT_USER, key_name));
}

} //end suite