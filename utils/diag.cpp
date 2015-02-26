/*********************************************************************/
/* g++ -std=c++11 -o diag diag.cpp -lboost_filesystem -lboost_system */
/*********************************************************************/

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main(int argc, char *argv[]) {
  short tol = 100;
  char *csodir;

  if(argc == 1) {
    std::cout << "Basic data diagnostic tool" << std::endl
	      << "Usage:" << std::endl
	      << argv[0] << " /path/to/cso_dir tol" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    csodir = argv[1];
    if(2 < argc) {
      std::istringstream ss(argv[2]);
      ss >> tol;
    }
  }

  int i = 0;
  int gap = 0;
  int missing = 0;
  std::string s;
  std::string basename;
  fs::path dir(csodir);
  fs::path filepath;

  while(1) {
    s = std::to_string(i);
    basename = std::string(8 - s.length(), '0').append(s);
    basename = basename.append(".cso");
    filepath = dir / basename;
    if (!fs::exists(filepath) || !fs::is_regular_file(filepath)) {
      missing++;
      gap++;
      if(gap > tol) {
	std::cout << "Exceeded tol value of " << tol << ", I give up searching" << std::endl;
	std::cout << "Found " << missing - tol - 1 << " files missing" << std::endl;
	break;
      }
    } else {
      std::cout << basename << std::endl;
      if(gap > 0) {
	gap = 0;
      }
    } 
    i++;
  }
  return 0;
}
