/*********************************************************************/
/* g++ -std=c++11 -o trange trange.cpp                               */
/*********************************************************************/

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <dirent.h>

struct trange {
  double start;
  double stop;
}; 

trange get_trange(char *path) {
  std::ifstream f(path, std::ios::in|std::ios::binary);
  double r;
  trange tr;

  if (f.is_open()) {
    f.read(reinterpret_cast<char*>(&r),8);
    tr.start = r;
    f.seekg (-10, std::ios::end);
    f.read(reinterpret_cast<char*>(&r),8);
    tr.stop = r;
    f.close();
  } else {
    std::cout << "Could not open file" << std::endl; 
    exit(EXIT_FAILURE);
  }

  return tr;
}

int main(int argc, char *argv[]) {
  char *bindir;
  
  if(argc == 1) {
    std::cout << "Tells the start time and end time of a cso file" << std::endl
	      << "Usage:" << std::endl
	      << argv[0] << " /path/to/cso_dir" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    bindir = argv[1];
  }
  
  DIR *dir;
  struct dirent *ent;
  trange tr;
  std::string bindir_string(bindir);
  std::string csofile_string;

  if ((dir = opendir(bindir)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_name[0] != '.') {
	csofile_string = bindir_string + "/" + ent->d_name;
	tr = get_trange(&csofile_string[0]);
	std::cout << std::fixed << "start time: " << tr.start
		  << " stop time: " << tr.stop << std::endl;
      }
    }
    closedir(dir);
  } else {
    std::cout << "Could not open directory" << std::endl;
    exit(EXIT_FAILURE);
  }

  //trange tr = get_trange(csofile);


  return 0;
}
