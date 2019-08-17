#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>
#include <set>
#include <array>

// g++ -g -O3 search.cpp -Wno-c++11-extensions -o search 
std::random_device r;
std::default_random_engine e1(r());

//largest K to search for
const uint MAX_K = 20;


class hypergraph {
public:
 std::set<std::array<uint,MAX_K> > hg;
 uint items;
 uint rows;
 uint k;
 bool buckets;
 hypergraph(int m, int _k, int n,bool _buckets){
  items = n;
  rows = m;
  k = _k;
  buckets=_buckets;
  // std::cout<<"items: " << items<<", rows: "<<rows<<", k hashes: "<<k<<"\n";
  std::uniform_int_distribution<> uniform_dist;
  //assert(MAX_K>=k);
  if (buckets==true) {
   uniform_dist =  std::uniform_int_distribution<>(0,rows/k-1);
  } else {
   uniform_dist =  std::uniform_int_distribution<>(0,rows-1);
   // std::cout << "Rows: "<<rows<<" "<<buckets<<"\n";
  }
  
  for(int i = 0; i < items; ++i){
   int bucket=0;
   std::array<uint,MAX_K> edge;
   for(int i = 0; i < k; ++i,bucket+=rows/k) {
    int result = uniform_dist(e1);
    // std::cout<<"Inserting into "<<i<<" ->"<<result<<"  which is "<<result+bucket<<"\n";
    if (buckets) {
     edge.at(i)=result+bucket;
    } else {
     edge.at(i)=result;
    }
   }
   hg.insert(edge);
  }
 };
 
 // For debugging
 void print_hg(){
  for(auto edge :hg){
   print_edge(edge);
  }
 };
 
 // For debugging
 void print_edge(std::array<uint,MAX_K>edge){
  std::cout<<"[";
  for(auto v : edge){
   std::cout << v << " "; 
  }
  std::cout<<"]\n";
 }
 
 
 // Peel a hypergraph. This is our main function
 void peel(){
  // std::cout<<"Peeling...\n";
  if (hg.size()==0)  {  return;  }
  
  /* Count the number of times we see each edge... looking for singles */
  std::map<uint,uint> vertex_cnt;
  for(auto edge :hg){
   for(uint v=0;v<k; v++){
    // std::cout<<"vertex: "<<v<<" "<<edge[v]<<"\n";
    vertex_cnt[edge[v]]+=1;
   }
  }
  
  std::set<uint> remove_vert;
  for(auto edge: vertex_cnt){
   if (edge.second==1)  {
    // std::cout<<"single kvertex "<<edge.first<<"\n";
    //assert(edge.first<rows);
    remove_vert.insert(edge.first);
   }
  }
  
  
  if (buckets==true){
   for (auto vert: remove_vert) {//for each vertex r 
    uint r_bucket = floor(vert /(rows/k)) ;
    //assert(r_bucket < k);
    auto itr= hg.begin();
    auto cnt = 0;
    while (itr!=hg.end()) { //search each edge for r
     cnt+=1;
     // std::cout<<cnt<<" remove "<<vert<<" ?"<<itr->at(r_bucket)<<" "<<hg.size()<<"\n";
     if (itr->at(r_bucket) == vert){
      hg.erase(itr);
      itr=hg.end();
      break; // there is only one, so we can stop looking for it
     } 
     if( itr!=hg.end() )
     itr++;
    }
    // std::cout<<" removed.\n";
   }
  } else {
   // if we have buckets, it's a little harder
   for (auto vert: remove_vert) {//for each vertex r 
    // std::cout<<"trying to remove "<<vert<<". Remaining edges: "<<hg.size()<<"\n";
    //assert(vert<rows);
    auto itr= hg.begin();
    int cnt = 0;
    while (itr!=hg.end()) { //search each edge for r
     cnt+=1;
     // std::cout<<"cnt: "<<cnt<<"\n";
     for(uint v=0;v<k; v++){
      // std::cout<<cnt<<" "<<"at "<<"v=="<<v<<"->"<<itr->at(v)<<"\n";
      if (itr->at(v) == vert){
       // std::cout<<"erase!\n";
       itr=hg.erase(itr);
       v=k;
       // break;
       // std::cout<<"success\n";
       // itr=hg.end();
       // break; // there is only one, so we can stop looking for it
      }
     }
     // std::cout<<"itr++!\n";
     if (itr!=hg.end())
     itr++;
    }
   }
  }
 }; //peel
 
 
 
 
 // Check if a hypergraph decoded
 
 bool check_decode(){
  if (items> hg.size())
  return(false);
  uint last_len = hg.size();
  // std::cout <<"hg size: "<<hg.size()<<"\n";
  // print_hg();
  peel();
  while(hg.size()<last_len){
   last_len = hg.size();
   // std::cout <<"hg size: "<<hg.size()<<"\n";
   // print_hg();
   peel();
  }
  return(hg.size()==0);
  
 };
};



// Determine the decode rate using at least 5000 trials.
// Trials stop when 95% confidence interval is met, or 
// mean is within .99999 to 1.00001  """
    
int main(int argc, char* argv[]){
 int entries=atoi(argv[1]);
 int k=atoi(argv[2]);
 int rows=atoi(argv[3]);
 long double goal = 1.0*atoi(argv[4])/atoi(argv[5]);
 bool buckets = atoi(argv[6])==1;
 long double ci_limit = (1-goal)/5;
 long double prob ;
 long double ci;
 int success =0;
 int trials=0;
 bool not_done=true;
 while (not_done ==true)  {
  for(int sub=0;sub<100;sub++,trials++){
   // std::cout<<"\nNew Trial\n";
   hypergraph h = hypergraph(rows,k,entries,buckets);
   if (h.check_decode())
   success+=1;  
  }
  prob = 1.0*success/trials;
  if (success < trials){
   ci = 1.96*sqrt(prob*(1.0-prob)/trials);
  } else {
   ci = -(exp(log(.05)/trials)-1);
  }
  if (trials>= 5000){
    if (prob-ci > goal){
      not_done = false;
      //std::cerr<<"done 1 "<<prob<<" -"<<ci<<">"<<goal <<"\n";
    }
    if (prob+ci <= goal){
      not_done = false;
      //std::cerr<<"done 2\n";
    }
    if ((prob-ci> goal-ci_limit) &&  (prob+ci<goal+ci_limit)){
      not_done = false;
      //std::cerr<<"done 3\n";
    }
  }
 }
 
 std::cout<<success<<","<<trials<<"\n";  
}
