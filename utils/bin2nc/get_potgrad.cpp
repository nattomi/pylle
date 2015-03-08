#include <iostream>
#include <pqxx/pqxx> 
#include <netcdfcpp.h>
#include <string>
#include <cstdlib>
#include <fstream>

using namespace std;
using namespace pqxx;

const string dir_base("/tmp/bin");
static const short ERR_ = 1; // return value for error

const string dbname = "";  
const string user = "";
const string password = "";  
const string hostaddr = "127.0.0.1";
const string port = "5432";
 

struct sample {
  double t;
  short d;
};

string full_path(string basename) {
  return dir_base + "/" + basename.substr(0,15) 
    + "/" + basename;
}

string ts2epoch(string ts) {
  size_t sep_loc = ts.find(".");
  string FracSec = "";

  if (sep_loc != string::npos) {
    FracSec = ts.substr(sep_loc);
    ts = ts.substr(0,sep_loc);
  }
  struct tm ts_tm;
  if (strptime(ts.c_str(), "%Y-%m-%dT%H:%M:%S", &ts_tm) == NULL) {                 
    cout << "strptime() failed" << endl;                                           
    exit(ERR_);                                                                  
  } 
  return to_string(timegm(&ts_tm)) + FracSec;
}
  
int main(int argc, char* argv[])
{

  string conn;

  string start = "2013-08-19T00:00:00.4560";
  string stop = "2013-08-20T00:00:00.34";
  string epoch_start_str(ts2epoch(start));
  string epoch_stop_str(ts2epoch(stop));
  double epoch_start = stod(epoch_start_str);
  double epoch_stop = stod(epoch_stop_str);
  string sql;

  double * time;
  short * digit;
  vector<sample> v;
  char path_out[] = "/tmp/out.nc";

  try{
    conn = "dbname=" + dbname 
      + " user=" + user
      + " password=" + password
      + " hostaddr=" + hostaddr
      + " port=" + port;
    connection C(conn);
   
    if (C.is_open()) {
      cout << "Opened database successfully: " << C.dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return ERR_;
    }
    /* Create SQL statement */
    sql = "select * from potgrad_basenames(" 
      + epoch_start_str + "," 
      + epoch_stop_str + ")";
 
    /* Create a non-transactional object. */
    nontransaction N(C);
     
    /* Execute SQL query */
    result R(N.exec(sql));

    /* List down all the records */
    for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
      string file_bin = full_path(c[0].as<string>());
      ifstream f(file_bin, ios::in|ios::binary);
      if (f.is_open()) {
  	cout << file_bin << endl;
  	f.seekg (0, ios::beg);
  	sample s;
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
    }
    cout << "data loaded to memory successfully" << endl;
    C.disconnect ();
  
  }catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }

  /* writing out result as netCDF binary */
  // fill cdf variables
  time = new double[v.size()];
  digit = new short[v.size()];
  for(unsigned int i = 0; i < v.size(); ++i) {
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
      return ERR_;
    }

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

  cout << "*** SUCCESS writing example file " << path_out << "!" << endl;
}
