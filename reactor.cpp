#include "reactor.hpp"
#include <memory>
#include <sstream>
#include <stdexcept>
#include <tuple>
namespace reactor {
std::string name(Outcome o) { switch(o) { case Outcome::success:return "success";case Outcome::transient:return "transient";case Outcome::permanent:return "permanent";case Outcome::budget:return "budget";case Outcome::deadline:return "deadline"; } throw std::logic_error("outcome"); }
int Report::work() const {int n=0;for(auto&o:operations)n+=o.leaves;return n;}
int Report::useful() const {int n=0;for(auto&o:operations)n+=o.useful;return n;}
std::string Report::json() const {std::ostringstream s;s<<"{\"work_units\":"<<work()<<",\"useful_completions\":"<<useful()<<",\"operations\":[";bool comma=false;for(auto&o:operations){if(comma)s<<',';comma=true;s<<"{\"leaves\":"<<o.leaves<<",\"completed_at\":"<<o.completed_at<<",\"outcome\":\""<<name(o.outcome)<<"\",\"useful\":"<<(o.useful?"true":"false")<<'}';}s<<"],\"events\":[";comma=false;for(auto&e:events){if(comma)s<<',';comma=true;s<<"{\"time\":"<<e.time<<",\"operation\":"<<e.operation<<",\"call\":"<<e.call<<",\"parent\":"<<e.parent<<",\"layer\":"<<e.layer<<",\"attempt\":"<<e.attempt<<",\"phase\":\""<<e.phase<<"\",\"outcome\":\""<<name(e.outcome)<<"\"}";}s<<"]}";return s.str();}
namespace {
struct Tick {int time;std::uint64_t seq;std::function<void()> fn;};
struct Later {bool operator()(const Tick&a,const Tick&b)const{return std::tie(a.time,a.seq)>std::tie(b.time,b.seq);}};
struct Call {int id,parent,layer,op,attempt=0;std::function<void(Outcome)> done;};
class Engine {
 Config cfg;const std::vector<Schedule>& schedules;Report report;std::priority_queue<Tick,std::vector<Tick>,Later> queue;int now=0,nextid=1;std::uint64_t seq=0;
 void enqueue(int delay,std::function<void()> fn){queue.push({now+delay,seq++,std::move(fn)});}
 void record(const std::shared_ptr<Call>& c,std::string phase,Outcome o){report.events.push_back({now,c->op,c->id,c->parent,c->layer,c->attempt,std::move(phase),o});}
 int backoff(const Call& c)const{ // Counter-based jitter: scheduling unrelated operations never consumes this stream.
  std::uint32_t x=cfg.seed ^ (std::uint32_t(c.op+1)*0x9e3779b9u) ^ (std::uint32_t(c.layer+1)*0x85ebca6bu) ^ (std::uint32_t(c.attempt)*0xc2b2ae35u);
  x^=x>>16;x*=0x7feb352du;x^=x>>15;return (1<< (c.attempt-1))+int(x%3);
 }
 void begin(int layer,int op,int parent,std::function<void(Outcome)> done){auto c=std::make_shared<Call>();c->id=nextid++;c->parent=parent;c->layer=layer;c->op=op;c->done=std::move(done);attempt(c);}
 void finish(const std::shared_ptr<Call>&c,Outcome o){record(c,"finish",o);c->done(o);}
 void returned(const std::shared_ptr<Call>&c,Outcome o){
  if(o==Outcome::transient && c->attempt<cfg.caps[c->layer]){int delay=backoff(*c);record(c,"retry",o);enqueue(delay,[this,c]{attempt(c);});}else finish(c,o);
 }
 void attempt(const std::shared_ptr<Call>&c){
  if(cfg.shared && now+cfg.latency>cfg.deadline){finish(c,Outcome::deadline);return;}
  ++c->attempt;record(c,"start",Outcome::transient);
  if(c->layer>0){begin(c->layer-1,c->op,c->id,[this,c](Outcome o){returned(c,o);});return;}
  auto&op=report.operations[c->op];
  if(cfg.shared && op.leaves>=cfg.budget){finish(c,Outcome::budget);return;}
  int ordinal=op.leaves++;const auto&sch=schedules[c->op];Outcome o=ordinal<int(sch.prefix.size())?sch.prefix[ordinal]:sch.tail;
  record(c,"leaf_admitted",o);enqueue(cfg.latency,[this,c,o]{record(c,"leaf_complete",o);returned(c,o);});
 }
public:
 Engine(Config c,const std::vector<Schedule>&s):cfg(c),schedules(s){
  for(int cap:c.caps)if(cap<1||cap>8)throw std::invalid_argument("caps must be 1..8");
  if(c.budget<0||c.deadline<0||c.deadline>1000000||c.latency<1||c.latency>10000)throw std::invalid_argument("invalid budget/deadline/latency");
  for(auto&sch:s) {if(sch.tail!=Outcome::success&&sch.tail!=Outcome::transient&&sch.tail!=Outcome::permanent)throw std::invalid_argument("invalid service outcome");for(auto o:sch.prefix)if(o!=Outcome::success&&o!=Outcome::transient&&o!=Outcome::permanent)throw std::invalid_argument("invalid service outcome");}
  report.operations.resize(s.size());
 }
 Report execute(){for(int i=0;i<int(schedules.size());++i)enqueue(0,[this,i]{begin(2,i,0,[this,i](Outcome o){auto&op=report.operations[i];op.outcome=o;op.completed_at=now;op.useful=o==Outcome::success&&now<=cfg.deadline;});});while(!queue.empty()){auto t=queue.top();queue.pop();now=t.time;t.fn();}return std::move(report);}
};
}
Report run(Config c,const std::vector<Schedule>&s){return Engine(c,s).execute();}
}
