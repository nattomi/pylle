/*
  g++ -o bin2nc -lnetcdf_c++ bin2nc.cpp
*/

#include <iostream>
#include <netcdfcpp.h>
#include <fstream>
#include <cstdlib>
#include <vector>

using namespace std;

// We are writing 1D data. 
static const int NDIMS = 1;
//static const int NX = 2;

// Return this in event of a problem.
static const int NC_ERR = 2;

struct sample {
  double t;
  short d;
};

int main(void)
{
  double epoch_start = 1376870400;//1376830581.85487
  double epoch_stop = 1376956800;
  double * time;
  short * digit;
  char path_in[] = "in";
  char path_out[] = "2013-08-19.nc";
  vector<sample> v;
  sample s;
  ifstream f(path_in, ios::in|ios::binary);
  int nReadings;
  
  if (f.is_open()) {
    f.seekg (0, ios::beg);
    while(f.read(reinterpret_cast<char*>(&s),10)) {
      if((epoch_start <= s.t) && (s.t < epoch_stop)) { 
	v.push_back(s);
      }
    }
    f.close(); 
  } else {
    cout << "Could not open file" << endl;
    exit(EXIT_FAILURE);
  }

  // delete elements of v to fit time limits
  // while(v[0].t < epoch_start) {
  //   cout << fixed << v[0].t << endl;
  //   v.erase(v.begin());
  // }
  
  cout << v.size() << endl;
  // fill cdf variables
  time = new double[v.size()];
  digit = new short[v.size()];

  for(int i = 0; i < v.size(); ++i) {
    time[i] = v[i].t;
    digit[i] = v[i].d;
  }

  // Create the file. The Replace parameter tells netCDF to overwrite
  // this file, if it already exists.
  NcFile dataFile(path_out, NcFile::Replace);
  
  // check whether a netCDF file creation or open
  // constructor succeeded.
  if (!dataFile.is_valid())
    {
      cout << "Couldn't open file!\n";
      return NC_ERR;
    }
  
  // For other method calls, the default behavior of the C++ API is
  // to exit with a message if there is an error.  If that behavior
  // is OK, there is no need to check return values in simple cases
  // like the following.
  
  // When we create netCDF dimensions, we get back a pointer to an
  // NcDim for each one.
  NcDim* xDim = dataFile.add_dim("x", v.size());
  
  // Define netCDF variables
  NcVar *time_data = dataFile.add_var("time", ncDouble, xDim);
  NcVar *digit_data = dataFile.add_var("digit", ncShort, xDim);
  
  // Write the data to the file.
  time_data->put(&time[0], v.size());
  digit_data->put(&digit[0], v.size());
    

  delete[] time;
  delete[] digit;
  // The file will be automatically closed when the NcFile object goes
  // out of scope. This frees up any internal netCDF resources
  // associated with the file, and flushes any buffers.
  cout << "*** SUCCESS writing example file " << path_out << "!" << endl;
  
  return 0;
}
