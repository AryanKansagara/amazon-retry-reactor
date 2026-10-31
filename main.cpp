#include "reactor.hpp"
#include <iostream>
using namespace reactor;
int main(int argc,char**){if(argc!=1){std::cerr<<"Usage: retry-reactor (JSON paired fixture report)\n";return 2;} Config c;
 std::vector<std::pair<std::string,Schedule>> cases={{"healthy",{{},Outcome::success}},{"one_transient",{{Outcome::transient},Outcome::success}},{"recover_after_four",{{Outcome::transient,Outcome::transient,Outcome::transient,Outcome::transient},Outcome::success}},{"permanent",{{},Outcome::permanent}},{"outage",{{},Outcome::transient}}};
 std::cout<<"{\"model\":\"local deterministic discrete-event; three retry layers, one leaf service\",\"latency_ticks\":2,\"deadline_ticks\":100,\"shared_leaf_budget\":5,\"layer_caps\":[3,3,3],\"aws_integration\":\"unverified\",\"cases\":[";
 bool comma=false;for(auto&[label,s]:cases){if(comma)std::cout<<',';comma=true;c.shared=false;auto a=run(c,{s});c.shared=true;auto b=run(c,{s});std::cout<<"{\"scenario\":\""<<label<<"\",\"naive\":"<<a.json()<<",\"shared\":"<<b.json()<<'}';}std::cout<<"]}\n";}
