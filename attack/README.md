# RVBBIT Arsenal — Offensive Research

This directory contains the offensive research side of **RVBBIT Arsenal**.

The code originated from experiments developed around the original [Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit) and is preserved here to study how kernel-level manipulation can affect system visibility.

The objective is not to provide an operational malware framework.

The objective is to provide a controlled reference implementation that can be compared directly with the defensive research under [`../defense/`](../defense/).

---

## Research purpose

The offensive side exists to answer questions such as:

> What kernel state must change for an object to disappear from a normal observation path?

> What does userspace stop seeing?

> What remains observable somewhere else?

> Which artifacts can be used by a defender?

The research workflow is therefore:

```text
Technique
   |
   v
Kernel modification
   |
   v
Visibility change
   |
   v
Remaining artifact
   |
   v
Defensive investigation
```

The offensive implementation is only the first half of that process.

---

## Research areas

The code explores concepts associated with Linux kernel rootkits and kernel-level concealment.

Current areas include:

| Area                      | Research focus                                                         |
| ------------------------- | ---------------------------------------------------------------------- |
| **DKOM**                  | Manipulation of kernel objects and relationships                       |
| **Process visibility**    | Differences between process existence and enumeration                  |
| **Module visibility**     | Changes to normal kernel-module enumeration                            |
| **Syscall interception**  | How redirected interfaces affect userspace observations                |
| **Filesystem visibility** | Filtering information returned during directory enumeration            |
| **Network visibility**    | Differences between networking state and selected exposed views        |
| **eBPF interaction**      | Experiments involving interference with selected BPF observation paths |
| **Persistence artifacts** | Studying traces created by persistence mechanisms                      |

These are established areas of rootkit research.

RVBBIT does not present them as previously unknown techniques or as universally effective methods for evading modern endpoint security.

---

## Relationship to the original RVBBIT

The original repository:

**[Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit)**

was created primarily to understand kernel-level concealment.

Its central question was:

> **How can kernel manipulation change what userspace sees?**

RVBBIT Arsenal extends that work by pairing offensive experiments with defensive investigation.

The question here becomes:

> **After visibility has been manipulated, what evidence is still available?**

---

## Relationship to RvbbitSafe

The defensive side of this repository lives under:

[`../defense/`](../defense/)

Rather than treating attack and defense as unrelated components, RVBBIT Arsenal uses the offensive experiments as known test cases for defensive research.

The intended relationship is:

```text
attack/
   |
   | produces
   v
Known manipulation
   |
   | leaves
   v
Artifacts / inconsistencies
   |
   | investigated by
   v
defense/
```

This makes it possible to study both the concealment mechanism and the detection assumptions around it.

---

## A note on historical components

Earlier RVBBIT experiments considered additional behavior associated with cryptocurrency mining and propagation.

Those ideas are **not the research focus of the public Arsenal project**.

The repository should be treated as source material for understanding kernel behavior and defensive visibility rather than as an instruction set for operational deployment.

The research value lies in the interaction between:

**kernel state → reported state → observable inconsistency**

not in monetization or propagation functionality.

---

## What this project does not claim

The offensive side does not claim to:

* provide reliable stealth against modern EDR products;
* defeat current hardened Linux environments;
* bypass modern hypervisor-based integrity monitoring;
* introduce previously unknown rootkit techniques;
* provide reliable compatibility across arbitrary kernel versions;
* represent production-quality malware;
* provide an operational offensive toolkit.

Several mechanisms are intentionally based on established techniques because their assumptions and detection opportunities are the subject of the research.

---

## Laboratory use

Kernel experiments can destabilize or crash a system.

Use a disposable virtual machine or another isolated laboratory environment.

Do not test the project on production systems.

Do not use the offensive components against systems or infrastructure without explicit authorization.

---

## Research direction

Future work around the offensive side should increasingly document each experiment using the following structure:

```text
Technique
  |
  +-- Mechanism
  |
  +-- Modified state
  |
  +-- Lost visibility
  |
  +-- Remaining artifacts
  |
  +-- Detection opportunities
  |
  `-- Limitations
```

This is intended to make the offensive code useful not only for understanding rootkit behavior but also for building and validating defensive research.

---

## Related resources

### RVBBIT Arsenal

[Repository root](../README.md)

### Defensive research

[RvbbitSafe](../defense/)

### Original project

[Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit)

### Research documentation

[Documentation](../docs/)

---

## Responsible use

This code is provided for cybersecurity education, controlled experimentation, and authorized security research.

Use it only:

* on systems you own;
* in isolated research environments; or
* where you have explicit authorization to conduct security testing.

The offensive component should not be deployed against third-party systems or infrastructure without permission.
