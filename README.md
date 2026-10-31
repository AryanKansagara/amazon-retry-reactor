# Retry Reactor — Amazon-inspired systems experiment

Amazon operates retail/logistics and AWS cloud infrastructure. AWS documents retries in asynchronous Lambda delivery and configurable SDK retry behavior. Composing those mechanisms with application retries is the research question here; this is an independent project, not an AWS implementation or roadmap claim.

This runnable C++20 discrete-event lab compares three nested retry layers (delivery → application → SDK) against one modeled leaf service. These are three modeled retry layers, not three network services. Naive policy keeps independent caps; the paired policy adds an operation-wide leaf-attempt budget and deadline admission. Both receive the exact same per-operation, leaf-ordinal failure schedule, latency, caps and deterministic counter-based backoff. Policies do not consume a shared mutable random stream. C++ keeps the event queue native; there is no inference model here.

Run with g++ and Make, without packages, credentials or cloud access:

```sh
make test
make demo
./retry-reactor > report.json
```

`demo.json` records actual local executions of five authored schedules. It reports causal call/parent IDs, layer/attempt, event times, leaf work and useful completions. Healthy requests and recoverable failures test useful work as well as reduced attempts. Permanent failures are terminal. The outage control demonstrates independent cap multiplication (27 leaf attempts) versus a shared budget of five; both deliver zero useful completions. That is modeled work, not measured AWS cost or reliability. A zero-work policy cannot count as a successful request. A success after the configured deadline is explicitly not useful.

Deadline semantics: the shared policy refuses a call when another leaf cannot finish by its deadline; it never admits a leaf crossing that deadline. A scheduled retry may wake after the deadline and be refused, so the operation's terminal notification can arrive later. No in-flight cancellation is modeled. Requests have independent budgets, not a shared tenant quota. Transient faults are service outcomes; budget/deadline outcomes never retry. Caps are 1–8 per layer, three layers fixed; service latency is fixed positive integer ticks. Event timestamps represent simulated time, not CPU timings. Tests cover normal execution, deterministic replay, causal recovery, cap multiplication, permanent errors, deadline admission, zero budget, and multiple operations.

## Scope and next validation

Implemented: deterministic queue, per-operation fault fixtures, bounded retry composition, causal JSON ledger and adverse controls. Unverified: AWS SDK/Lambda adapters, actual delivery semantics, production fault distributions, pricing, tracing and hosted deployment. A future AWS adapter needs an isolated account, SDK version and explicit experiment budget. Existing AWS retry controls and idempotency tooling already address parts of this problem; this lab studies policy composition rather than replacing them.

Primary research sources (read for the research brief, 2026-10-31):

- https://docs.aws.amazon.com/lambda/latest/dg/invocation-async-error-handling.html
- https://docs.aws.amazon.com/sdkref/latest/guide/feature-retry-behavior.html

The project opportunity is our inference from these documented constraints, not evidence that Amazon lacks a solution.
