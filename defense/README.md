# RvbbitSafe

### Defensive Linux Kernel Security Research

RvbbitSafe is the defensive research component of **RVBBIT Arsenal**.

It was built to investigate an important question raised by the offensive RVBBIT experiments:

> **If one view of system state has been manipulated, can another independent source expose the inconsistency?**

RvbbitSafe is not intended to be a universal anti-rootkit or endpoint security product.

It is a research prototype used to explore detection and integrity-validation ideas against known behavior inside the RVBBIT laboratory environment.

---

## Why RvbbitSafe exists

The original RVBBIT experiments focused on concealment.

Processes could disappear from normal enumeration paths.

Kernel modules could become absent from expected listings.

System interfaces could provide a filtered view of underlying state.

Those experiments raised a natural defensive question:

> **What remains visible after something has been hidden?**

That became the starting point for RvbbitSafe.

---

## Current components

RvbbitSafe currently contains two main components.

### Kernel detector

`detector.c`

The kernel-side component explores techniques including:

* hidden-process discovery;
* comparison of different views of kernel state;
* integrity checks related to altered kernel references;
* detection of known manipulation associated with the RVBBIT laboratory environment.

### User-space component

`rvbbitsafe_daemon.c`

The user-space component supports the defensive experiments and can assist with:

* observing results from the detector;
* handling known artifacts associated with the test environment;
* persistence-artifact investigation;
* cleanup during controlled experiments.

These components should be understood as research implementations rather than production security software.

---

## Research model

RvbbitSafe follows the same model used throughout RVBBIT Arsenal:

```text
Offensive technique
        |
        v
Modified state
        |
        v
Visibility gap
        |
        v
Remaining evidence
        |
        v
Independent observation
        |
        v
Detection
```

The most important part is the transition from **visibility gap** to **remaining evidence**.

A concealment technique does not necessarily remove the underlying object.

It may only affect one path used to observe it.

---

## Cross-view analysis

Cross-view analysis is one of the main ideas explored by RvbbitSafe.

Consider two sources that should describe the same underlying system state.

If they disagree, that disagreement may itself become a useful signal.

Conceptually:

```text
Observer A
    |
    | reports
    v
 State X

        !=

Observer B
    |
    | reports
    v
 State Y
```

The goal is not to assume that either observer is automatically correct.

The goal is to investigate **why the views differ**.

This approach is especially relevant when studying kernel-level concealment because a compromised or altered observation path may continue returning internally consistent but incomplete information.

---

## Research areas

Current and planned defensive areas include:

| Area                      | Research focus                                                            |
| ------------------------- | ------------------------------------------------------------------------- |
| **Process detection**     | Identifying inconsistencies caused by hidden processes                    |
| **Kernel integrity**      | Checking selected kernel references for unexpected modification           |
| **Module visibility**     | Comparing normal enumeration with independent evidence                    |
| **Persistence artifacts** | Identifying changes associated with persistence experiments               |
| **Cross-view analysis**   | Comparing partially independent representations of state                  |
| **eBPF telemetry**        | Investigating independent event-based observability                       |
| **Validation**            | Testing whether detection survives variations of the offensive experiment |

---

## Limitations

RvbbitSafe is a research prototype.

It does not claim to:

* detect every Linux rootkit;
* provide complete kernel integrity monitoring;
* replace a modern EDR product;
* reliably identify unknown kernel threats;
* operate unchanged across arbitrary Linux kernel versions;
* guarantee complete remediation;
* provide production-ready endpoint protection.

The current defensive implementation is closely related to known RVBBIT behavior.

A detection that works against one controlled experiment should not automatically be generalized to unrelated rootkits.

That limitation is intentional and documented.

---

## Detection is not the same as proof

An unexpected inconsistency can indicate manipulation.

It can also have another explanation.

For that reason, a detection signal should not automatically be treated as proof of compromise.

Useful defensive research should eventually answer:

```text
What was observed?

Why is it suspicious?

What else could produce it?

How can the result be verified?

What are the false-positive conditions?
```

Documenting those questions is part of the future direction of RvbbitSafe.

---

## Laboratory environment

RvbbitSafe is intended to be evaluated alongside the offensive RVBBIT experiments in isolated systems.

Kernel-level testing can destabilize a machine.

Use disposable virtual machines or other controlled laboratory environments.

The project should not currently be treated as a security control for production infrastructure.

---

## Building

Build requirements depend on the Linux kernel and distribution used for testing.

A typical laboratory system will require:

```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

Compile the defensive component from this directory using the included build configuration.

Because kernel internals vary between versions, compilation and behavior may require adjustment for different environments.

---

## Validation

When running experiments, focus on the evidence rather than simply asking whether RvbbitSafe printed a detection message.

Useful observations include:

* which artifact triggered detection;
* which data sources were compared;
* whether the offensive technique changed both sources;
* whether the signal survives a modified implementation;
* whether legitimate system behavior can create the same result.

Those observations are more valuable than a simple "detected / not detected" result.

---

## Relationship to the offensive component

The offensive research environment is available under:

[`../attack/`](../attack/)

The two components should be considered part of the same experiment:

```text
attack/
   |
   v
Known manipulation
   |
   v
Observable effects
   |
   v
defense/
   |
   v
Detection experiment
```

The goal is not to demonstrate a perfect attack followed by a perfect cure.

The goal is to understand the boundary between **manipulation and observability**.

---

## Research direction

Future defensive work will focus increasingly on:

* stronger cross-view techniques;
* reproducible detection experiments;
* kernel integrity validation;
* eBPF-based telemetry;
* false-positive analysis;
* testing against modified offensive implementations;
* documenting which assumptions each detection relies on.

The desired format for individual experiments is:

```text
Technique
    |
Artifact
    |
Telemetry source
    |
Detection logic
    |
Validation
    |
False positives
    |
Limitations
```

---

## Related resources

### RVBBIT Arsenal

[Repository root](../README.md)

### Offensive research

[Attack component](../attack/)

### Original RVBBIT

[Project RVBBIT](https://github.com/buter-chkalova/project-rvbbit)

### Documentation

[Research documentation](../docs/)

---

## Responsible use

RvbbitSafe is provided for cybersecurity education and defensive security research.

Test it in controlled environments and validate its behavior independently before relying on any result.
