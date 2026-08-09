# RVBBIT Arsenal

## Studying Linux Kernel Manipulation Through Attack, Observability, and Detection

**Author:** buter-chkalova
**Project:** RVBBIT Arsenal
**Research area:** Linux Kernel Security / Rootkit Research / Detection Engineering
**Status:** Experimental research prototype

---

## Abstract

RVBBIT Arsenal is an experimental Linux kernel security research project built to study the relationship between kernel-level manipulation and defensive observability.

The project evolved from Project RVBBIT, an educational Linux kernel rootkit proof of concept implementing several established techniques associated with kernel-level concealment. These experiments raised a second question that became more interesting than concealment itself:

> **When one representation of system state has been manipulated, what evidence remains available elsewhere?**

RVBBIT Arsenal approaches this question by placing offensive experiments and defensive detection work in the same research environment.

The offensive component is used to examine how selected kernel structures and interfaces can be modified so that the state reported to userspace differs from the underlying system state. The defensive component, RvbbitSafe, explores whether those inconsistencies can be identified through alternative observations, integrity validation, and artifact analysis.

The project does not claim to introduce novel rootkit techniques, provide reliable evasion against modern endpoint security products, or implement a general-purpose anti-rootkit.

Instead, RVBBIT Arsenal is intended as a controlled environment for examining a narrower and more useful problem:

**the boundary between kernel manipulation, observable state, and detection.**

The research model used throughout the project is:

```text
Technique
    |
    v
State modification
    |
    v
Visibility change
    |
    v
Residual artifact
    |
    v
Independent observation
    |
    v
Detection hypothesis
    |
    v
Validation
    |
    v
Limitation
```

This paper documents the architecture of the project, the assumptions behind its offensive and defensive components, the limitations of the current implementation, and the direction required to turn individual demonstrations into reproducible detection experiments.

---

# 1. Introduction

Kernel-level security research has an unusual property: the component being observed may also control the interfaces used to perform the observation.

A userspace program rarely observes kernel state directly.

Commands such as `ps`, `ls`, `lsmod`, `ss`, or applications reading information from `/proc` receive representations of system state constructed through kernel data structures and interfaces.

Under normal conditions, treating those representations as authoritative is practical.

Under adversarial conditions, that assumption becomes weaker.

If the mechanism responsible for producing a representation is modified, an observer may receive internally valid but incomplete information.

A process can continue executing while disappearing from a particular enumeration path.

A kernel module can remain resident while no longer appearing through expected module-listing mechanisms.

A network connection can continue to exist while being omitted from a selected reporting interface.

The object has not necessarily disappeared.

The **observer's view of the object** has changed.

This distinction is the central research problem behind RVBBIT Arsenal.

---

# 2. Research motivation

The original Project RVBBIT began primarily as an implementation exercise.

The objective was to reproduce several established Linux kernel rootkit concepts and understand how they interact with real kernel structures rather than studying them only from documentation or historical examples.

That work included experiments around:

* Direct Kernel Object Manipulation (DKOM);
* process visibility;
* kernel-module visibility;
* syscall interception;
* directory-entry filtering;
* network visibility;
* interaction with BPF-related interfaces;
* persistence behavior.

Initially, the natural question was:

> **How can kernel-level code modify what userspace sees?**

While implementing those experiments, a second question emerged:

> **What does the manipulation fail to change?**

That question changes the problem substantially.

An offensive implementation generally needs to alter enough of the system's observable state to achieve its objective.

It does not necessarily alter every possible representation of that state.

The difference between those representations creates an opportunity for detection.

RVBBIT Arsenal was created to investigate that opportunity.

---

# 3. Scope and research claims

A clear scope is necessary because rootkit research is particularly vulnerable to exaggerated claims.

RVBBIT Arsenal does **not** attempt to demonstrate an undetectable Linux rootkit.

It also does not claim that RvbbitSafe provides universal rootkit detection.

The current project should instead be interpreted as a set of controlled experiments around three questions:

1. **How can selected kernel-level mechanisms alter system visibility?**
2. **Which artifacts or inconsistencies remain after that alteration?**
3. **Can those artifacts be converted into defensible detection hypotheses?**

The project therefore distinguishes between four different concepts:

### Implementation

Code exists that demonstrates a particular mechanism.

### Observation

An artifact or inconsistency can be observed under the laboratory conditions.

### Detection hypothesis

The observation may be useful for identifying the corresponding manipulation.

### General detection capability

The technique remains reliable against unrelated implementations, kernel versions, environmental changes, and deliberate countermeasures.

RVBBIT Arsenal currently operates primarily within the first three categories.

It does not claim the fourth.

---

# 4. System model

For the purposes of this research, system visibility can be simplified into three layers.

```text
+--------------------------------------------------+
|                 User-space observer              |
|                                                  |
| ps / ls / monitoring / security tooling / /proc |
+-------------------------+------------------------+
                          |
                          v
+--------------------------------------------------+
|              Kernel observation path             |
|                                                  |
| syscalls / seq interfaces / kernel structures    |
+-------------------------+------------------------+
                          |
                          v
+--------------------------------------------------+
|                Underlying state                  |
|                                                  |
| tasks / modules / files / sockets / references   |
+--------------------------------------------------+
```

Under normal conditions:

```text
Underlying state
       |
       v
Observation path
       |
       v
Reported state
```

The reported state is expected to represent the underlying state closely enough for the observer's purpose.

A concealment mechanism introduces another operation:

```text
Underlying state
       |
       v
Modified observation path
       |
       v
Filtered reported state
```

Alternatively, direct manipulation may alter one kernel representation while leaving another representation unchanged:

```text
              Underlying object
                    |
          +---------+---------+
          |                   |
          v                   v
     Structure A          Structure B
          |                   |
      modified              intact
          |                   |
          v                   v
       View A              View B

          View A != View B
```

That inequality is the conceptual basis of cross-view detection.

---

# 5. RVBBIT Arsenal architecture

The repository separates offensive and defensive research explicitly.

```text
rvbbit-arsenal/
|
+-- attack/
|   |
|   +-- kernel-level manipulation experiments
|   +-- visibility experiments
|   `-- supporting research code
|
+-- defense/
|   |
|   +-- detector.c
|   +-- rvbbitsafe_daemon.c
|   `-- defensive experiment tooling
|
`-- docs/
    |
    `-- research documentation
```

Conceptually, however, these are not two independent projects.

They form one experimental loop:

```text
        +----------------------+
        |  Offensive technique |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |  State modification  |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |  Visibility change   |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Residual evidence    |
        +----------+-----------+
                   |
                   v
        +----------------------+
        | Defensive hypothesis |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |      Validation      |
        +----------+-----------+
                   |
                   v
        +----------------------+
        |      Limitation      |
        +----------------------+
```

The purpose of the offensive side is therefore not simply to demonstrate concealment.

It provides controlled state manipulation against which defensive assumptions can be tested.

---

# 6. Offensive research model

## 6.1 Kernel module visibility

A loaded kernel module is represented through kernel-maintained structures used for management and enumeration.

Manipulating the relationships used by normal enumeration can make a module absent from expected administrative views while its code remains resident.

The important distinction is:

```text
Module execution state != Module enumeration state
```

For defensive research, this produces a useful question:

> If normal module enumeration no longer reports the module, what independent evidence of its presence remains?

Potential research directions include comparison with memory-backed evidence, integrity measurements, or other kernel-maintained representations.

The current project demonstrates the offensive side of this visibility problem but does not yet implement a general solution for independent module discovery.

---

## 6.2 Process visibility and DKOM

Process hiding is one of the clearest demonstrations of the difference between object existence and object enumeration.

Linux represents processes through `task_struct` instances and several relationships used for scheduling, PID lookup, and enumeration.

If one of those relationships is modified while another remains usable, two kernel-level views may disagree.

Conceptually:

```text
           Process exists
                 |
        +--------+--------+
        |                 |
        v                 v
 Task enumeration      PID lookup
        |                 |
     altered            intact
        |                 |
        v                 v
  process absent      process found
```

This creates the basis for a cross-view detection strategy.

The defensive problem is no longer:

> Can `ps` see the process?

It becomes:

> Do independent kernel mechanisms agree that the process exists?

That is a substantially stronger research question.

---

# 7. Syscall interception and integrity

System-call interception represents another form of observation-path manipulation.

Rather than changing the underlying object, interception can modify how requests are handled or how results are returned.

From a defensive perspective, the relevant property is not simply that a syscall was hooked.

The important question is whether a security boundary can establish the expected destination of a syscall entry and determine whether that destination has changed.

Conceptually:

```text
Expected syscall entry
         |
         v
Expected kernel handler

            versus

Observed syscall entry
         |
         v
Unexpected handler
```

This turns syscall hooking into an integrity problem.

A detector can therefore reason about the mapping itself rather than attempting to infer every behavioral consequence of the hook.

The current RvbbitSafe prototype contains an experiment around locating the syscall table and restoring selected entries to addresses resolved from kernel symbols.

This should be interpreted as a controlled proof of concept rather than a general integrity architecture.

Kernel symbol availability, kernel version differences, write-protection mechanisms, architecture, and modern kernel hardening all affect the validity of this approach.

---

# 8. Filesystem visibility

Directory listings provide another useful example of observation-path manipulation.

Userspace programs generally do not independently reconstruct filesystem state.

They request directory information from the kernel and display the returned entries.

If entries are filtered before the result reaches userspace, multiple userspace programs may present the same incomplete view because they ultimately depend on the same underlying observation path.

This illustrates an important defensive principle:

> **Different tools are not necessarily independent observers.**

Running several userspace commands does not provide meaningful cross-view validation if they all depend on the same manipulated kernel interface.

A stronger detection strategy requires a source that does not share the same failure mode.

---

# 9. Network visibility

Network-state reporting has the same structural problem.

A connection may exist in kernel networking state while a selected reporting interface omits it.

This produces two distinct concepts:

```text
Connection existence

        versus

Connection representation
```

Filtering a representation does not remove the underlying network activity.

That suggests several defensive research directions:

* independent socket-state enumeration;
* event-based telemetry;
* network observation outside the affected reporting path;
* comparison between host-level and external network evidence.

RVBBIT Arsenal currently uses network hiding primarily as an example of this visibility problem rather than claiming complete network stealth.

---

# 10. eBPF and observability

eBPF is particularly relevant because it changes the relationship between userspace security tooling and kernel events.

Rather than relying only on periodic enumeration, eBPF-based tooling can observe selected events close to where they occur.

From the perspective of rootkit research, this creates a different problem.

The offensive question becomes:

> What happens if a privileged kernel-level component interferes with the ability to establish new observation mechanisms?

The corresponding defensive question is more interesting:

> Can loss of telemetry itself become telemetry?

A monitoring system should ideally distinguish between:

```text
No suspicious events occurred
```

and:

```text
The system lost the ability to observe suspicious events
```

Those states are not equivalent.

RVBBIT includes experiments around interference with selected BPF program-loading operations.

This does not imply universal evasion of eBPF-based security systems.

Existing programs, alternative instrumentation, kernel hardening, external telemetry, and implementation-specific behavior significantly affect the result.

The experiment is therefore better interpreted as an investigation into **sensor integrity**.

---

# 11. Persistence as an observable artifact

Persistence is often discussed only as an offensive capability.

From a detection perspective, persistence is also useful because it creates durable state.

A kernel-resident modification may be transient.

Persistence usually requires interaction with additional system components.

Those interactions can produce:

* configuration changes;
* service definitions;
* module-loading configuration;
* filesystem artifacts;
* startup-state modifications.

This changes the detection problem.

Instead of detecting the kernel behavior directly, a defender may detect the supporting state required to recreate it after reboot.

This demonstrates a broader principle:

> The most visible part of a kernel-level threat may exist outside the kernel.

RVBBIT Arsenal therefore treats persistence artifacts as part of the overall observation model rather than merely as an offensive feature.

---

# 12. RvbbitSafe

RvbbitSafe is the defensive research component of RVBBIT Arsenal.

Its purpose is not to provide a universal countermeasure.

It exists to test specific defensive hypotheses generated by the offensive experiments.

The current implementation contains two primary components:

```text
RvbbitSafe
|
+-- Kernel detector
|   |
|   +-- process-view comparison
|   `-- selected syscall integrity restoration
|
`-- User-space daemon
    |
    +-- process/artifact inspection
    `-- known persistence cleanup
```

These components deliberately operate at different layers.

The kernel detector has access to kernel-side information.

The daemon operates through userspace-visible interfaces and filesystem state.

This separation itself provides useful material for studying trust boundaries.

---

# 13. Hidden-process detection experiment

The current kernel detector performs a simple cross-view experiment.

It iterates processes using one kernel traversal mechanism and attempts to resolve each PID through another kernel PID lookup mechanism.

Conceptually:

```text
View A: task enumeration
           |
           | PID
           v
View B: PID resolution
           |
      +----+----+
      |         |
   agrees    disagrees
      |         |
      v         v
   normal    investigate
```

The useful idea is not the exact implementation.

The useful idea is **view disagreement**.

However, the current implementation also demonstrates an important limitation of cross-view detection:

two views are useful only to the extent that they are genuinely independent.

If an attacker manipulates both underlying structures, the comparison may no longer reveal the discrepancy.

If both views ultimately depend on shared corrupted state, the detector can fail while appearing internally consistent.

Therefore:

> **Cross-view detection is not automatically trustworthy merely because two APIs are compared.**

The independence of the observations must itself be part of the threat model.

---

# 14. Syscall integrity experiment

The current detector also attempts to locate the syscall table and restore selected entries using addresses resolved from kernel symbols.

The research idea is straightforward:

```text
Observed entry
      |
      v
Compare / validate
      |
      v
Expected kernel target
```

If an entry no longer references the expected implementation, the difference may indicate modification.

The current code takes the additional step of attempting restoration.

That is useful for demonstrating remediation concepts, but detection and remediation should be treated separately.

A production-quality integrity system would need to answer several additional questions:

* How was the expected value established?
* Can the source of the expected value also be manipulated?
* Is the observed difference legitimate?
* Is restoration safe while the system is running?
* Could another subsystem still reference the modified code?
* What synchronization guarantees are required?
* Does restoration destroy forensic evidence?

RVBBIT Arsenal does not currently solve those problems.

They are part of the research boundary.

---

# 15. User-space defensive component

The RvbbitSafe user-space daemon performs a different class of defensive work.

It examines userspace-visible process information and removes known persistence artifacts associated with the laboratory scenario.

This provides a useful contrast with the kernel detector.

The kernel component asks:

> Is kernel state internally consistent?

The userspace component asks:

> Are known artifacts of the experiment present?

These are different detection strategies.

One is structural.

The other is indicator-driven.

Indicator-driven detection can be effective against a known implementation but generalizes poorly.

For example, identifying a process by a known command-line characteristic can detect the laboratory scenario while failing immediately if the process identity changes.

Likewise, deleting known persistence paths is useful for controlled cleanup but does not constitute generic persistence detection.

For this reason, the current daemon should be interpreted primarily as **laboratory remediation and artifact handling**, not as a general detection engine.

---

# 16. Trust boundaries

The project exposes a deeper issue: every detector depends on something.

Consider the following chain:

```text
Security tool
     |
     v
Userspace API
     |
     v
Kernel interface
     |
     v
Kernel structure
     |
     v
Hardware state
```

If the attacker operates below the detector's trust boundary, the detector may receive manipulated information.

A userspace detector cannot automatically trust `/proc` if the kernel is compromised.

A kernel detector cannot automatically trust every kernel structure if another kernel component has modified it.

Even a cross-view detector may fail if both views share a compromised dependency.

Therefore, every detection mechanism should identify:

1. **What data does it trust?**
2. **Why is that data expected to remain trustworthy?**
3. **Can the attacker modify the same source?**
4. **What independent observation exists outside that trust boundary?**

This is one of the most important conclusions produced by the project.

---

# 17. Detection matrix

The following matrix summarizes the intended relationship between the offensive experiments and defensive research.

| Offensive experiment | Visibility affected          | Residual evidence                            | Defensive direction                | Current maturity               |
| -------------------- | ---------------------------- | -------------------------------------------- | ---------------------------------- | ------------------------------ |
| Process hiding       | Normal process enumeration   | Disagreement between process representations | Cross-view analysis                | Prototype                      |
| Module hiding        | Module enumeration           | Independent module/memory evidence           | Integrity / cross-view analysis    | Research direction             |
| Syscall interception | System-call execution path   | Unexpected entry target                      | Integrity validation               | Prototype                      |
| File filtering       | Directory enumeration        | Underlying filesystem state                  | Independent filesystem observation | Research direction             |
| Network filtering    | Selected network reporting   | Socket/event/network evidence                | Cross-source comparison            | Research direction             |
| BPF interference     | Sensor deployment capability | Loss/failure of expected telemetry           | Sensor-integrity monitoring        | Research direction             |
| Persistence          | Startup/system configuration | Durable configuration artifacts              | Artifact analysis                  | Prototype / laboratory cleanup |

The distinction between **prototype** and **research direction** is intentional.

A design idea should not be presented as an implemented capability until it has been implemented and tested.

---

# 18. What the current prototype demonstrates

The current state of RVBBIT Arsenal supports several limited conclusions.

### 18.1 Kernel visibility can be path-dependent

An object can remain present while disappearing from a selected observation mechanism.

This is the basic premise demonstrated by the offensive experiments.

### 18.2 Manipulation can create inconsistencies

If one representation changes while another remains intact, disagreement becomes observable.

This provides the conceptual basis for cross-view detection.

### 18.3 Integrity can sometimes be easier to reason about than behavior

Rather than attempting to identify every possible consequence of syscall interception, a detector can investigate whether an expected kernel reference has changed.

### 18.4 Persistence increases the observable surface

Maintaining state across reboot typically creates artifacts outside the original kernel manipulation.

### 18.5 Detection depends on independent trust

Comparing two sources is useful only when they do not share the same compromised assumptions.

This is probably the most important defensive conclusion of the project so far.

---

# 19. What the current prototype does not prove

Equally important are the conclusions that cannot currently be made.

RVBBIT Arsenal does **not** demonstrate that:

* its offensive techniques evade modern EDR products;
* the offensive implementation is undetectable;
* RvbbitSafe detects arbitrary Linux rootkits;
* cross-view process detection survives an attacker that manipulates both compared representations;
* syscall restoration is universally safe;
* the project works consistently across modern kernel versions;
* the defensive mechanisms remain reliable against modified implementations;
* eBPF-based security tooling can be universally neutralized;
* the current laboratory results generalize to hardened production systems.

These limitations are not secondary details.

They define the next research questions.

---

# 20. Experimental validity

For RVBBIT Arsenal to mature from a proof-of-concept collection into a stronger research platform, experiments need to become reproducible.

Each experiment should eventually record at least:

### Environment

* Linux distribution;
* kernel version;
* architecture;
* relevant kernel configuration;
* virtualization environment;
* security features enabled.

### Initial state

The expected system state before manipulation.

### Manipulation

Which research technique was activated.

### Observation

Which data sources were examined.

### Detection result

Whether the expected discrepancy was observed.

### Negative control

Whether the detector reports the same condition without the manipulation present.

### Variation

Whether the result survives changes to the offensive implementation.

### Limitations

Known conditions under which the experiment fails.

Without this information, a successful detection is a demonstration.

With it, the result begins to become an experiment.

---

# 21. False positives and false negatives

Detection research is incomplete if it documents only successful detections.

For every defensive mechanism, two failure classes matter.

## False positive

The detector identifies suspicious behavior when no relevant manipulation occurred.

## False negative

The manipulation occurs but the detector does not identify it.

For cross-view analysis, false positives may result from legitimate transient disagreement, race conditions, lifecycle transitions, or assumptions about synchronization.

False negatives may result from manipulation of both compared views or from an attacker moving below the detector's trust boundary.

Future versions of RvbbitSafe should therefore measure not only:

```text
Did detection occur?
```

but:

```text
Under which conditions did detection occur?

Under which conditions did it fail?

What legitimate state produces the same signal?
```

That distinction is necessary if the project is to make meaningful defensive claims.

---

# 22. Kernel-version dependence

Linux kernel internals are not a stable malware-development API.

Structures change.

Symbols become unavailable.

Interfaces evolve.

Hardening mechanisms change the feasibility of techniques that worked on earlier systems.

This affects both sides of RVBBIT Arsenal.

An offensive experiment may fail because an assumption about kernel internals is no longer valid.

A defensive experiment may fail for exactly the same reason.

For this project, compatibility failure should not automatically be treated as something to hide or patch around.

It is useful evidence.

A compatibility matrix would make those assumptions explicit:

| Kernel    | Offensive experiment | Defensive experiment | Result | Notes |
| --------- | -------------------- | -------------------- | ------ | ----- |
| Version A | Technique X          | Detection X          | TBD    |       |
| Version B | Technique X          | Detection X          | TBD    |       |
| Version C | Technique X          | Detection X          | TBD    |       |

Future research should populate this table with measured results rather than assumptions.

---

# 23. Relationship to modern defensive architecture

RVBBIT Arsenal intentionally operates at a relatively low level.

That does not mean low-level detection should exist in isolation.

A realistic defensive architecture can combine evidence from several layers:

```text
             External telemetry
                    |
                    v
          +-------------------+
          | Network evidence  |
          +---------+---------+
                    |
                    v
          +-------------------+
          | Userspace sensors |
          +---------+---------+
                    |
                    v
          +-------------------+
          | Kernel telemetry  |
          +---------+---------+
                    |
                    v
          +-------------------+
          | Integrity checks  |
          +-------------------+
```

The value of multiple layers is not merely having more telemetry.

The value comes from **different failure modes**.

If every sensor depends on the same compromised representation, sensor count does not produce independence.

---

# 24. Research methodology going forward

The future direction of RVBBIT Arsenal is therefore not to maximize the number of offensive features.

The preferred development cycle is:

```text
1. Select one kernel manipulation
              |
              v
2. Define expected visibility change
              |
              v
3. Identify residual evidence
              |
              v
4. Implement detection hypothesis
              |
              v
5. Create negative control
              |
              v
6. Modify offensive implementation
              |
              v
7. Retest detection
              |
              v
8. Document failure conditions
```

This cycle produces more useful research than simply adding another concealment mechanism.

---

# 25. Proposed experiment format

Future experiments should be documented consistently.

## Technique

What behavior is being investigated?

## Mechanism

Which kernel structure or observation path is involved?

## Hypothesis

What change is expected?

## Observable artifact

What evidence should remain?

## Telemetry source

Where is that evidence obtained?

## Detection logic

How is the discrepancy identified?

## Validation

How was the hypothesis tested?

## Negative control

What happens without the offensive modification?

## Evasion test

What happens when the implementation changes?

## False positives

What legitimate conditions produce similar evidence?

## Limitations

Where does the technique stop being reliable?

This format should eventually become the core documentation model for RVBBIT Arsenal.

---

# 26. Ethical design

Kernel rootkit research has obvious dual-use implications.

RVBBIT Arsenal is intended to preserve the educational value of studying kernel manipulation without treating operational capability as the objective.

The research focus is therefore placed on:

* mechanism understanding;
* observable artifacts;
* defensive reasoning;
* controlled experiments;
* limitations;
* reproducibility.

The project should be tested only on systems owned by the researcher or systems where explicit authorization has been provided.

Disposable virtual machines and isolated laboratory environments are strongly preferred.

The public research should prioritize understanding and detection over operational deployment.

---

# 27. Independent community analysis

The original Project RVBBIT received independent attention outside the repository.

Spanish cybersecurity publication **Hackplayers** published a technical analysis of the project covering its architecture and several kernel-level concealment mechanisms.

A separate Chinese technical article on **CSDN** later examined the project from another perspective.

These publications are relevant not because they prove the correctness of every implementation decision, but because they demonstrate that the project was sufficiently concrete for independent researchers to inspect, interpret, and criticize.

That distinction matters.

External attention is not technical validation.

Independent reproducibility and adversarial testing are stronger forms of validation.

The next stage of RVBBIT Arsenal should therefore aim to make experiments easier for other researchers to reproduce rather than relying on visibility or repository popularity as evidence of correctness.

---

# 28. Discussion

The original motivation behind RVBBIT was offensive.

The most useful result has been defensive.

Implementing concealment mechanisms makes one fact difficult to ignore:

> **Security tooling observes representations of reality, not reality itself.**

Every representation has a provenance.

Every observation has a trust boundary.

Every detector depends on assumptions about which parts of the system remain trustworthy.

Once those assumptions are written down, detection becomes easier to reason about.

Instead of asking:

> Is this process hidden?

we can ask:

> Which structures describe this process, which of them can be manipulated, and which independent observation survives that manipulation?

Instead of asking:

> Is this syscall hooked?

we can ask:

> What establishes the expected control-flow target, and can that source itself be trusted?

Instead of asking:

> Did the security sensor report anything?

we can ask:

> Was the sensor still capable of observing the system during the experiment?

Those questions are more useful than simply labeling a technique "stealthy" or a detector "effective."

---

# 29. Conclusion

RVBBIT Arsenal is not intended to represent a contest between a perfect rootkit and a perfect anti-rootkit.

Neither exists here.

The project is better understood as a laboratory for studying disagreement.

Disagreement between:

* underlying state and reported state;
* one kernel representation and another;
* expected control flow and observed control flow;
* configured state and expected state;
* sensor availability and assumed sensor availability.

Those disagreements create opportunities for detection.

They also expose the assumptions on which detection depends.

The original RVBBIT project asked:

> **How can kernel manipulation change what userspace sees?**

RVBBIT Arsenal asks the next question:

> **When that view can no longer be trusted, what evidence remains?**

The long-term goal is to answer that question experimentally rather than rhetorically.

That means documenting not only techniques that work, but also detections that fail, assumptions that break, kernel versions that invalidate the experiment, and legitimate conditions that produce the same signals.

A useful security research project should not attempt to make either side look unbeatable.

It should make the boundary between them measurable.

---

# 30. Future work

The next research milestones are:

1. Build reproducible laboratory scenarios for individual techniques.
2. Record kernel and environment versions for each experiment.
3. Expand process cross-view validation.
4. Investigate independent module-visibility evidence.
5. Separate syscall integrity detection from remediation.
6. Add negative controls to defensive experiments.
7. Document false-positive conditions.
8. Test detections against modified offensive implementations.
9. Investigate eBPF-based telemetry from a sensor-integrity perspective.
10. Develop an ATT&CK mapping only where the technical behavior maps cleanly.
11. Create an experiment-results directory containing measured observations.
12. Build a compatibility matrix across selected Linux kernel versions.

The desired end state is:

```text
Offensive technique
        |
        v
Documented state change
        |
        v
Measured artifact
        |
        v
Detection hypothesis
        |
        v
Reproducible experiment
        |
        v
Adversarial variation
        |
        v
Known limitations
```

At that point, RVBBIT Arsenal becomes more than a collection of attack and defense code.

It becomes a framework for asking reproducible questions about Linux kernel observability.

---

# References and project resources

## Project repositories

**Project RVBBIT**
Original Linux Kernel Rootkit Research PoC
https://github.com/buter-chkalova/project-rvbbit

**RVBBIT Arsenal**
Offensive & Defensive Linux Kernel Security Research
https://github.com/buter-chkalova/rvbbit-arsenal

## Independent analysis

**Hackplayers — Spain**
*RVBBIT: anatomía de un rootkit LKM moderno basado en stealth, DKOM y anti-eBPF*
https://www.hackplayers.com/2026/04/rvbbit-anatomia-de-un-rootkit-lkm.html

**CSDN — China**
*内核里的“幽灵”：一套Linux Rootkit隐身术完全拆解*
https://blog.csdn.net/chen1415886044/article/details/161462914

---

# Legal and ethical notice

RVBBIT Arsenal is provided for cybersecurity education, controlled experimentation, and authorized security research.

Kernel-level experiments can destabilize systems and should be performed only in isolated laboratory environments.

Do not deploy or test offensive components against systems, networks, or infrastructure without explicit authorization.

The project makes no claim of production suitability, universal detection capability, or reliable stealth against modern security products.
