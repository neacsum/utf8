/*
  Generate case folding tables (UCASETAB.C and LCASETAB.C) from
  CASEFOLDING.TXT file.

*/
#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

struct codept {
  char32_t uc, lc;
  string descr;
};

vector<codept> tab;

int main (int argc, char **argv)
{
  char line[1024];
  char *ptr;
  codept code;
  ifstream in ("casefolding.txt");

  while (in)
  {
    in.getline (line, sizeof (line));
    if (line[0] == '#')
      continue; //ignore comment lines
    if (!(ptr = strtok (line, "; ")))
      continue;
    code.uc = strtol (ptr, nullptr, 16);
    ptr = strtok (nullptr, "; ");
    if (*ptr != 'C' && *ptr != 'S')
      continue;
    ptr = strtok (nullptr, "; ");
    code.lc = strtol (ptr, nullptr, 16);
    ptr = strtok (nullptr, ";");
    ptr = strchr (ptr, '#') + 1;
    code.descr = ptr;
    tab.push_back (code);
  }

  //Table should be sorted by uppercase code but doesn't hurt to check
  sort (tab.begin (), tab.end (),
    [](codept& p1, codept& p2)->bool {return p1.uc < p2.uc; });

  //check for duplicates (there aren't any but again better be safe)
  for (int i = 0; i < tab.size () - 1; i++)
  {
    if (tab[i].uc == tab[i + 1].uc)
    {
      printf ("Duplicate uppercase: %05x:\n - %s\n and  %05x %s\n\n",
        (int)tab[i].uc, tab[i].descr.c_str (), (int)tab[i + 1].uc,
        tab[i + 1].descr.c_str ());
      tab.erase (tab.begin () + i + 1);
    }
  }

  ofstream out ("../src/uppertab.c");
  out << "//Upper case table" << endl
    << "static char32_t u2l [" << tab.size() << "] = { " << endl;
  out << hex;
  for (int i=0; i<tab.size(); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].uc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << "// " << tab[i].descr.c_str() << endl;
  }
  out << dec << endl;
  out << "//Lower case equivalents" << endl
    << "static char32_t lc [" << tab.size () << "] = { " << endl;
  out << hex;
  for (int i = 0; i < tab.size (); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].lc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << endl;
  }
  out.close ();


  //Sort table by lower case codes
  sort (tab.begin (), tab.end (),
    [](codept& p1, codept& p2)->bool {return p1.lc < p2.lc; });

  //check for duplicates
  for (int i = 0; i < tab.size () - 1; i++)
  {
    if (tab[i].lc == tab[i + 1].lc)
    {
      printf ("Duplicate lowercase: %05x:\n Upper case %05x - %s\n and  %05x %s\n\n",
        (int)tab[i].lc, (int)tab[i].uc, tab[i].descr.c_str (), (int)tab[i + 1].uc,
        tab[i + 1].descr.c_str ());
      tab.erase (tab.begin () + i+1);
    }
  }

  out.open ("../src/lowertab.c");
  out << "//Lower case table" << dec << endl
    << "static char32_t l2u [" << tab.size () << "] = { " << endl;
  out << hex;
  for (int i = 0; i < tab.size (); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].lc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << endl;
  }
  out << dec << endl;
  out << "//Upper case equivalents" << endl
    << "static char32_t uc [" << tab.size () << "] = { " << endl;
  out << hex;
  for (int i = 0; i < tab.size (); i++)
  {
    out << "  0x" << std::setfill ('0') << std::setw (5) << tab[i].uc;
    if (i == tab.size () - 1)
      out << "};";
    else
      out << ", ";
    out << "// " << tab[i].descr.c_str () << endl;
  }
  out.close ();

  return 0;
}
