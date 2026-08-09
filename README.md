# RVBBIT Arsenal

### Offensive & Defensive Linux Kernel Security Research

RVBBIT Arsenal is an experimental Linux kernel security research environment built around a simple idea:

> **If we understand how a system can be manipulated, we should also study what evidence that manipulation leaves behind.**

The repository brings offensive rootkit experiments and defensive detection work into the same project.

It grew out of the original [Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit), which began as an educational Linux kernel rootkit proof of concept.

RVBBIT Arsenal takes the next step.

Instead of asking only how kernel-level concealment works, the project also asks:

* What state was modified?
* What disappeared from normal visibility?
* What artifacts remain?
* Can another source expose the inconsistency?
* What can a defender measure or verify independently?

The long-term research model is:

```text
Technique
   |
   v
Kernel modification
   |
   v
Visibility gap
   |
   v
Observable artifact
   |
   v
Telemetry
   |
   v
Detection
   |
   v
Limitation
```

> **Research only:** RVBBIT Arsenal is intended for cybersecurity education, controlled laboratory experiments, malware-analysis research, and defensive detection work. It is not intended for operational deployment.

---

## Why RVBBIT Arsenal exists

Project RVBBIT originally focused on the offensive side of Linux kernel visibility.

The experiments explored techniques such as:

* Direct Kernel Object Manipulation (DKOM);
* syscall interception;
* process and module hiding;
* filesystem visibility manipulation;
* network visibility manipulation;
* persistence behavior;
* interference with selected eBPF-based observation paths.

While developing those experiments, another question became more interesting:

> **What did the hiding mechanism fail to hide?**

That question became the foundation of RVBBIT Arsenal.

The goal is not simply to place a rootkit and an "anti-rootkit" next to each other.

The goal is to study both sides of the same system state.

---

## Repository structure

```text
rvbbit-arsenal/
|
|-- attack/
|   `-- Offensive Linux kernel research experiments
|
|-- defense/
|   `-- RvbbitSafe detection and integrity experiments
|
|-- docs/
|   `-- Technical documentation and research notes
|
|-- CHANGELOG.md
|-- LICENSE
`-- README.md
```

### `attack/`

Contains the offensive research side derived from Project RVBBIT.

The code is used to study how kernel-level manipulation can alter what normal userspace tools are able to observe.

### `defense/`

Contains **RvbbitSafe**, the defensive research side of the project.

RvbbitSafe explores detection and integrity-validation approaches related to the artifacts produced by the offensive experiments.

### `docs/`

Contains longer-form documentation and technical notes describing the research.

---

## The research model

RVBBIT Arsenal is organized around the relationship between an offensive action and the evidence it produces.

| Stage              | Question                                               |
| ------------------ | ------------------------------------------------------ |
| **Technique**      | What behavior is being tested?                         |
| **Modification**   | What kernel state or interface changes?                |
| **Visibility gap** | What does normal userspace stop seeing?                |
| **Artifact**       | What evidence remains elsewhere?                       |
| **Telemetry**      | Which independent source can expose it?                |
| **Detection**      | How can that inconsistency be identified?              |
| **Limitation**     | Where does the detection fail or generate uncertainty? |

This model is more important to the project than any single rootkit technique.

---

## Offensive research

The offensive side of RVBBIT Arsenal is derived from the original RVBBIT experiments.

Research areas include:

| Area                      | Research question                                                                      |
| ------------------------- | -------------------------------------------------------------------------------------- |
| **DKOM**                  | What happens when kernel objects or their relationships are manipulated directly?      |
| **Process hiding**        | Can process existence differ from process enumeration?                                 |
| **Module hiding**         | What remains observable after normal module enumeration is altered?                    |
| **Syscall interception**  | How does redirecting kernel interfaces affect userspace visibility?                    |
| **Filesystem visibility** | What happens when directory information is filtered before reaching userspace?         |
| **Network visibility**    | Can one networking interface present a different view from another?                    |
| **eBPF interaction**      | What happens when kernel-level code interferes with selected observability mechanisms? |
| **Persistence**           | Which artifacts are created by attempts to survive reboot or restart?                  |

These techniques are implemented as research experiments.

They are not presented as novel, universally stealthy, or capable of reliably bypassing current endpoint security products.

---

## Defensive research: RvbbitSafe

**RvbbitSafe** is the defensive side of RVBBIT Arsenal.

It was created after the offensive experiments raised a more useful question:

> **If one source of system information can be manipulated, can another source reveal the disagreement?**

The current defensive work includes experiments around:

* hidden-process discovery;
* cross-view comparison;
* syscall-table integrity validation;
* restoration of altered kernel references;
* persistence-artifact discovery;
* cleanup of artifacts associated with the test environment;
* comparison between kernel and userspace observations.

RvbbitSafe should not be interpreted as a complete anti-rootkit or endpoint security product.

It is a research prototype used to explore detection ideas against known behavior in the RVBBIT laboratory environment.

---

## Attack vs. defense

The relationship between both sides of the project can be represented like this:

```text
                OFFENSIVE SIDE
                       |
                       v
             Kernel manipulation
                       |
          +------------+------------+
          |                         |
          v                         v
   Reported state              Actual state
          |                         |
          +------------+------------+
                       |
                       v
                 Inconsistency
                       |
                       v
                 Observation
                       |
                       v
                    Detection
                       |
                       v
                 DEFENSIVE SIDE
```

The project increasingly focuses on those inconsistencies.

A hidden object does not necessarily stop existing.

A filtered interface does not necessarily remove the underlying state.

A disabled sensor may itself become an observable event.

Those gaps are often where defensive research becomes useful.

---

## Cross-view detection

One of the main ideas behind the defensive side is **cross-view analysis**.

Suppose one kernel structure reports one set of processes while an independent structure implies another.

Neither view should automatically be treated as absolute truth.

The interesting signal is the disagreement.

The same idea can be applied to several areas:

```text
Process enumeration
        vs.
Independent kernel state

Module enumeration
        vs.
Memory / integrity evidence

/proc network view
        vs.
Other networking telemetry

Configured persistence
        vs.
Expected system state
```

This approach is not specific to RVBBIT.

The broader research question is whether multiple partially independent observations can expose manipulation that remains invisible through a single interface.

---

## A note on stealth

RVBBIT Arsenal does not use *stealth* as a synonym for *undetectable*.

The project demonstrates concealment from particular paths or sources.

That distinction matters.

A process can disappear from one enumeration mechanism and still leave evidence somewhere else.

A module can disappear from a standard listing while still occupying memory.

A connection can be filtered from one interface while remaining observable through another part of the networking stack.

For that reason:

> **The defensive value often begins where the offensive concealment stops.**

---

## Limitations

RVBBIT Arsenal is a learning and research prototype.

It does not claim to:

* introduce previously unknown Linux rootkit techniques;
* defeat modern EDR products reliably;
* bypass modern hypervisor-based integrity mechanisms;
* provide universal compatibility across Linux kernel versions;
* represent a production-ready offensive framework;
* represent a production-ready defensive product;
* provide complete coverage of current Linux kernel threats.

Several offensive techniques intentionally reproduce established approaches because understanding their assumptions and detection opportunities is part of the project.

Likewise, the defensive side is tested primarily against behavior associated with the RVBBIT research environment.

A successful detection in this laboratory should not be interpreted as proof that the same mechanism detects every unrelated kernel threat.

---

## Why limitations are part of the project

A security experiment is less useful if every failure is hidden.

For RVBBIT Arsenal, limitations are research results too.

If an offensive technique becomes visible because of a particular artifact, that tells us something useful.

If a detection generates false positives, that tells us something useful.

If a kernel update breaks an assumption, that tells us something useful.

If two supposedly independent telemetry sources turn out to depend on the same kernel interface, that is especially useful.

The objective is not to make either side look unbeatable.

The objective is to understand where each side breaks.

---

## Relationship to Project RVBBIT

The original [Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit) is where the research started.

Its central question was:

> **How can kernel-level manipulation change what userspace sees?**

RVBBIT Arsenal continues from there:

> **What evidence remains when that view can no longer be trusted?**

The relationship between the projects is:

```text
Project RVBBIT
      |
      | Offensive kernel experiments
      v
Understanding concealment
      |
      | What remains visible?
      v
RVBBIT Arsenal
      |
      +-- attack/
      |
      +-- defense/
      |
      `-- docs/
```

The first repository represents the original proof of concept.

Arsenal represents the broader research direction.

---

## Independent coverage of RVBBIT research

The original RVBBIT project has been independently examined by cybersecurity researchers outside the repository.

### Hackplayers — Spain

Spanish cybersecurity publication **Hackplayers** published a technical analysis:

**[RVBBIT: anatomía de un rootkit LKM moderno basado en stealth, DKOM y anti-eBPF](https://www.hackplayers.com/2026/04/rvbbit-anatomia-de-un-rootkit-lkm.html)**

The article discusses the original project's architecture, DKOM behavior, syscall interception, hiding mechanisms, persistence, and interaction with eBPF-based observability.

### CSDN — China

A separate Chinese technical article on **CSDN** analyzed Project RVBBIT:

**[内核里的“幽灵”：一套Linux Rootkit隐身术完全拆解](https://blog.csdn.net/chen1415886044/article/details/161462914)**

The article independently examines several of the project's kernel-level concealment mechanisms and discusses possible defensive perspectives.

The external interest in the original project contributed to the decision to continue the research through RVBBIT Arsenal rather than simply expand the rootkit proof of concept.

---

## Documentation

Longer-form documentation is available under:

```text
docs/
```

The project documentation is being developed around both offensive behavior and defensive interpretation.

The intended direction is to document experiments using a consistent format:

```text
Technique
  |
  +-- Mechanism
  +-- Changed state
  +-- Observable artifact
  +-- Detection idea
  +-- False-positive considerations
  `-- Limitations
```

This structure will gradually replace broad claims about "attack" and "defense" with reproducible research notes.

---

## Research roadmap

The areas I want to explore further include:

* cross-view process detection;
* kernel integrity checking;
* module visibility discrepancies;
* eBPF-based telemetry;
* persistence artifact analysis;
* detection engineering;
* ATT&CK mapping where technically appropriate;
* reproducible laboratory scenarios;
* kernel-version compatibility testing;
* false-positive documentation;
* defensive validation against modified variants of the offensive experiments.

The long-term objective is for meaningful offensive experiments to have corresponding defensive analysis.

Not:

```text
rootkit + anti-rootkit
```

but:

```text
technique
   |
artifact
   |
telemetry
   |
detection
   |
validation
   |
limitation
```

---

## Safety and responsible use

RVBBIT Arsenal is intended for:

* cybersecurity education;
* malware-analysis laboratories;
* controlled Linux kernel experiments;
* authorized security research;
* defensive detection research.

Use the project only on systems you own, disposable laboratory environments, or systems where you have explicit authorization to perform security testing.

Do not deploy offensive components against third-party systems or infrastructure without permission.

---

## Feedback

Technical criticism is welcome.

I am particularly interested in feedback related to:

* Linux kernel internals;
* detection methodology;
* incorrect assumptions;
* cross-view analysis;
* eBPF observability;
* kernel-version compatibility;
* integrity verification;
* false positives;
* reproducibility.

The project is intended to evolve through testing and criticism rather than through claims of perfect stealth or perfect detection.

---

## Author

**buter-chkalova**

Independent security research focused on Linux kernel internals, offensive security, rootkit behavior, and defensive detection.

### RVBBIT research

* **[Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit)** — original Linux Kernel Rootkit Research PoC
* **RVBBIT Arsenal** — offensive & defensive Linux kernel security research environment

---

## License

Released under the **MIT License**.

See [LICENSE](LICENSE) for details.
