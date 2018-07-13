#include <Python.h>
#include <cassert>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream, std::stringbuf
#include <iomanip>      // std::setbase

#include "iblt.h"
#include "utilstrencodings.h"


// https://stackoverflow.com/questions/145270/calling-c-c-from-python
// https://stackoverflow.com/questions/1615813/how-to-use-c-classes-with-ctypes

extern "C" {

  /*
    Constructor. We'll pass the pointer to our class object using longs and some
    casts that wouldn't mornally be good style. When python passes the pointers
    back as unsigned longs, we'll just reinterpret_cast them.
  */
  void pyblt_set_parameter_file(char * filename){
    IBLT::set_parameter_file(filename);
  }

  // create an IBLT and manually set the hedge and numHashes 
  unsigned long  pyblt_manual(size_t entries,size_t valuesize, float hedge, size_t numHashes){
    return (unsigned long) new IBLT(entries,valuesize,hedge, numHashes);
  }

  // let the parameter file choose hedge and numHashes 
  unsigned long  pyblt_new(size_t entries,size_t valuesize){
    return (unsigned long) new IBLT(entries,valuesize);
  }
  /* free some memory */
  void pyblt_delete(unsigned long ptr){
    IBLT * g = reinterpret_cast<IBLT *>(ptr);
    delete g;
  }

  /* Dump the internal representation of the table. */
  char * pyblt_dump_table(unsigned long ptr){
    IBLT * g = reinterpret_cast<IBLT *>(ptr);
    char * result=new char[g->DumpTable().size()+1];
    sprintf(result,"%s" ,g->DumpTable().c_str());
    return(result);
  }

  /* Insert a (key,value) pair. The value is converted to a vector of uint8_t */
  void pyblt_insert(unsigned long ptr,uint64_t key, char * value){
    IBLT * g = reinterpret_cast<IBLT *>(ptr);
    g->insert(key,  ParseHex(value));
  }

  /* Erase a (key,value) pair. */
  void pyblt_erase(unsigned long ptr,uint64_t key, char * value){
    IBLT * g = reinterpret_cast<IBLT *>(ptr);
    g->erase(key,  ParseHex(value));
  }

  // Results that contain keys and value from the iblt
  typedef struct _RESULT {
    bool decoded;
    uint pos_len;
    uint neg_len;
    uint64_t * pos_keys;
    uint64_t * neg_keys;
    char * pos_str;
    char * neg_str;
  } RESULT; 

  // results that contain only the keys, not values
  typedef struct _KEYS {
      uint pos_len;
      uint neg_len;
      uint64_t * pos_keys;
      uint64_t * neg_keys;
  } KEYS; 

  // Return all entries by decoding, and returning results as a string.
  
  RESULT pyblt_list_entries(unsigned long ptr,int type=0){
    IBLT * g = reinterpret_cast<IBLT *>(ptr);
    std::set<std::pair<uint64_t,std::vector<uint8_t> > > pos;
    std::set<std::pair<uint64_t,std::vector<uint8_t> > > neg;

    RESULT res;
    res.decoded = g->listEntries(pos,neg);
    res.pos_len=pos.size();
    res.neg_len=neg.size();
    res.pos_keys = new uint64_t[pos.size()];
    res.neg_keys = new uint64_t[neg.size()];
    std::ostringstream pos_result;
    int i=0;
    for(auto iter=pos.begin(); iter!=pos.end();++iter){
      res.pos_keys[i++]=iter->first;
      for(uint8_t i : iter->second) {
          pos_result << std::setw(1) <<std::setbase(16) << unsigned(i);
	    };
      pos_result<<std::string("\n");
    }
    res.pos_str=new char[pos_result.str().size()+1]; 
    sprintf(res.pos_str,"%s" ,pos_result.str().c_str());

    std::ostringstream neg_result;
    i=0;
    for(auto iter=neg.begin(); iter!=neg.end();++iter){
      res.neg_keys[i++]=iter->first;
      for(uint8_t i : iter->second) {
          neg_result << std::setw(1) <<std::setbase(16) << unsigned(i);
	    };
      neg_result<<std::string("\n");
    }
    res.neg_str=new char[neg_result.str().size()+1]; 
    sprintf(res.neg_str,"%s" ,neg_result.str().c_str());
    return(res);
  }

KEYS pyblt_peel_entries(unsigned long ptr,int type=0){
    // this returns only keys, not values. (TODO...)
    IBLT * g = reinterpret_cast<IBLT *>(ptr);
    std::set<std::pair<uint64_t,std::vector<uint8_t> > > pos;
    std::set<std::pair<uint64_t,std::vector<uint8_t> > > neg;
    g->peelEntries(pos,neg);
    KEYS res;
    res.pos_len=pos.size();
    res.neg_len=neg.size();
    res.pos_keys = new uint64_t[pos.size()];
    res.neg_keys = new uint64_t[neg.size()];
    int i=0;
    for(auto iter=pos.begin(); iter!=pos.end();++iter)
      res.pos_keys[i++]=iter->first;
    i=0;
    for(auto iter=neg.begin(); iter!=neg.end();++iter)
      res.neg_keys[i++]=iter->first;
    return(res);
  }

  /* Eppstein's subtract function. */
  unsigned long pyblt_subtract(unsigned long   iblt,  unsigned long other){
    IBLT * g = reinterpret_cast<IBLT *>(iblt);
    IBLT * h = reinterpret_cast<IBLT *>(other);

    //allocate enough memory to copy over the result
    IBLT  * result=  new IBLT(g->hashTableSize(),g->valueSize,1.0,g->numHashes);
    *result= *g - *h;
    return (unsigned long) result;
  }

  /* The number rows in the IBLT */
  int pyblt_capacity(unsigned long iblt) {
    IBLT * g = reinterpret_cast<IBLT *>(iblt);
    return(g->hashTableSize());

  }

}
