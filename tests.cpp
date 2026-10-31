#include "reactor.hpp"
#include <iostream>
#include <stdexcept>
using namespace reactor;
int checks=0;void check(bool b){++checks;if(!b)throw std::runtime_error("check "+std::to_string(checks));}
int main(){try{Config c;Schedule healthy{{},Outcome::success},outage{{},Outcome::transient},permanent{{},Outcome::permanent};
 auto h=run(c,{healthy});check(h.work()==1&&h.useful()==1);int starts=0;for(auto&e:h.events)starts+=e.phase=="start";check(starts==3);
 c.shared=false;auto n=run(c,{outage});check(n.work()==27);check(n.useful()==0);check(n.json()==run(c,{outage}).json());
 c.shared=true;auto b=run(c,{outage});check(b.work()==5);check(b.operations[0].outcome==Outcome::budget);check(b.useful()==0);
 for(bool policy:{false,true}){c.shared=policy;auto p=run(c,{permanent});check(p.work()==1&&p.operations[0].outcome==Outcome::permanent);auto r=run(c,{{{Outcome::transient},Outcome::success}});check(r.work()==2&&r.useful()==1);}
 c.shared=true;c.budget=0;auto z=run(c,{healthy});check(z.work()==0&&z.useful()==0);c.budget=5;c.deadline=1;auto d=run(c,{healthy});check(d.work()==0&&d.operations[0].outcome==Outcome::deadline);
 c.deadline=9;auto tight=run(c,{outage});for(auto&e:tight.events)if(e.phase=="leaf_admitted")check(e.time+c.latency<=c.deadline);check(tight.operations[0].outcome==Outcome::deadline);
 c.deadline=100;auto fair=run(c,{outage,healthy,outage});check(fair.operations[0].leaves==5&&fair.operations[1].leaves==1&&fair.operations[2].leaves==5);check(fair.useful()==1);
 c.shared=false;auto late=run(c,{outage});check(late.work()==27);c.deadline=1;late=run(c,{healthy});check(late.work()==1&&late.useful()==0&&late.operations[0].outcome==Outcome::success);
 c.deadline=100;auto prefix=run(c,{{{Outcome::transient,Outcome::transient,Outcome::transient,Outcome::transient},Outcome::success}});c.shared=true;auto paired=run(c,{{{Outcome::transient,Outcome::transient,Outcome::transient,Outcome::transient},Outcome::success}});check(prefix.json()==paired.json());check(paired.useful()==1);
 c.caps={1,1,1};check(run(c,{outage}).work()==1);c.caps={0,1,1};bool rejected=false;try{run(c,{healthy});}catch(const std::invalid_argument&){rejected=true;}check(rejected);
 std::cout<<checks<<" assertions passed\n";
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
