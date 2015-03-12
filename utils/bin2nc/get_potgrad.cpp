#include <iostream>
#include <vector>  
#include <string>  
#include <cstdio>  
#include <cstdlib>
#include <pqxx/pqxx> 
#include <netcdfcpp.h>
#include <cgicc/CgiDefs.h> 
#include <cgicc/Cgicc.h> 
#include <cgicc/HTTPHTMLHeader.h> 
#include <cgicc/HTMLClasses.h>  
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace cgicc;
using namespace pqxx;

const string dir_base("");
const boost::filesystem::path basedir("/tmp");
const string wwwroot("");
const string prefix("");

const string dbname = "";  
const string user = "";
const string password = "";  
const string hostaddr = "127.0.0.1";
const string port = "5432";
 
struct sample {
  double t;
  short d;
};

struct tmpf {
  boost::filesystem::path inner;
  boost::filesystem::path outer;
};

tmpf gen_tmpf(string wwwroot,
	      boost::filesystem::path basedir, 
	      string prefix) 
{
  string filename;
  boost::filesystem::path filepath_inner;
  boost::filesystem::path filepath_outer;
  boost::uuids::uuid suffix = boost::uuids::random_generator()();
  filename = prefix + boost::lexical_cast<string>(suffix);
  filepath_outer = basedir / filename;
  filepath_inner = wwwroot / filepath_outer;
  if(!boost::filesystem::exists(filepath_inner)) {
    tmpf filepath;
    filepath.inner = filepath_inner;
    filepath.outer = filepath_outer;
    return filepath;
  } else {
    return gen_tmpf(wwwroot, basedir, prefix);
  }
}

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
    exit(EXIT_FAILURE);
  } 
  return to_string(timegm(&ts_tm)) + FracSec;
}

int main ()
{
  Cgicc params;
  string start, stop;
   
  form_iterator fi = params.getElement("start");  
  if( !fi->isEmpty() && fi != (*params).end()) {  
    start = **fi;  
  }else{
    exit(EXIT_FAILURE);  
  }
  
  fi = params.getElement("stop");  
  if( !fi->isEmpty() &&fi != (*params).end()) {
    stop = ** fi;
  }else{
    exit(EXIT_FAILURE); 
  }

  //cout << "Content-type:text/html\r\n\r\n";

  string epoch_start_str(ts2epoch(start));
  string epoch_stop_str(ts2epoch(stop));
  double epoch_start = stod(epoch_start_str);
  double epoch_stop = stod(epoch_stop_str);

  if(epoch_stop <= epoch_start) {
    exit(EXIT_FAILURE); // stop date must be bigger than start date
  }
  
  if(epoch_stop - epoch_start > 172800) {
    exit(EXIT_FAILURE); // requested time range too large
  }
  
  string conn;
  string sql;

  double * time;
  short * digit;
  vector<sample> v;

  try{
    conn = "dbname=" + dbname 
      + " user=" + user
      + " password=" + password
      + " hostaddr=" + hostaddr
      + " port=" + port;
    connection C(conn);
    
    if (!C.is_open()) {
      exit(EXIT_FAILURE);
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
  	f.seekg (0, ios::beg);
  	sample s;
  	while(f.read(reinterpret_cast<char*>(&s),10)) {
  	  if((epoch_start <= s.t) && (s.t < epoch_stop)) {
  	    v.push_back(s);
  	  }
  	}
  	f.close();
      } else {
       	exit(EXIT_FAILURE);
      }
    }
    C.disconnect ();
    
  }catch (const std::exception &e){
    cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
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
  
  tmpf path_out;
  path_out = gen_tmpf(wwwroot, basedir, prefix);
  NcFile dataFile(path_out.inner.string().c_str(), NcFile::Replace);
  // check whether a netCDF file creation or open
  // constructor succeeded.
  if (!dataFile.is_valid())
    {
      exit(EXIT_FAILURE);
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
  
  cout << "Content-type:application/x-netcdf\r\n";
  cout << "Location:"<< path_out.outer.string().c_str() << "\r\n\r\n";

  return 0;
}
