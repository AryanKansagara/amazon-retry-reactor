#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <vector>
namespace reactor {
enum class Outcome { success, transient, permanent, budget, deadline };
std::string name(Outcome);
struct Schedule { std::vector<Outcome> prefix; Outcome tail=Outcome::success; };
struct Config { std::array<int,3> caps{3,3,3}; int budget=5; int deadline=100; int latency=2; unsigned seed=17; bool shared=true; };
struct Event { int time, operation, call, parent, layer, attempt; std::string phase; Outcome outcome; };
struct Operation { int leaves=0, completed_at=-1; Outcome outcome=Outcome::transient; bool useful=false; };
struct Report { std::vector<Operation> operations; std::vector<Event> events; int work() const; int useful() const; std::string json() const; };
Report run(Config, const std::vector<Schedule>&);
}
