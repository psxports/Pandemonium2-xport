# Pandemonium 2

Resolve `[XPORT_ROOT]` from `xport-project.json` and read `[XPORT_ROOT]/AGENTS.md`. Keep only stable game-specific facts and task-routing links here; put detailed evidence under `status`. Never create, edit, move, delete or regenerate `README.md`.

## Project facts

- Native short name: `P2`; active game language: C++17; shared xport runtime remains C
- Solution: `src/platform/win/P2.sln`; Debug executable: `bin/P2_debug.exe`; Release executable: `bin/P2.exe`; working directory: `bin`; intermediates: `_build`
- Runtime data: `bin/DATA`; Red Book output when applicable: `bin/MUSIC`
- Before relying on them, record reviewed image identities, language decision, dummy scope, hooks/layouts, adapter contract and evidence links here

- Original inputs: `status/startup.json` (field `inventory`) and startup receipts retain hashes; `orig/SYSTEM.CNF` selects `SLUS_005.78` as the boot executable; `FMV.EXE` is a separate image
- Startup language decision: C, following the canonical scaffold; no original-language claim is made
- CUE: one MODE2/2352 data track, no Red Book AUDIO tracks; XA media stays game data
- GPU graph binding `0u` selects the shared native standard GPU. Direct DATA reads and typed native CD callbacks do not use guest CD address bindings; reviewed contract: `status/refactoring-compliance-execution/stage-08/native-cd-callback-boundary/changes.json`

- Reviewed GP: `SLUS_005.78` uses `0x8008A0C8` from `0x800112FC`; `FMV.EXE` uses `0x8011E794` from `0x801009A0`; both derive from original `lui/addiu` instructions
- Active exports: `orig/images-main-gp-v3`; `orig/images-gp-v2` retains the earlier corrected exports for both images; `orig/images` is superseded because segment reconstruction discarded the initial IDA GP assignment; preserve it and its receipts
- GP export verification: `status/gp-export-verification.json`; checked every exported GP-relative load/store against original bytes and actual IDA data references

- User-approved scope: analyze and translate `SLUS_005.78`; defer `FMV.EXE` and preserve its existing exports; replace confirmed FMV module launches with skip stubs when their owning callsites enter translation scope
- Reviewed entry boundary: `status/entry-boundary-review.json`; executable entry ends after BREAK at `0x80011324`; `0x80011328..0x80011338` is stack-table data; geometry starts independently at `0x80011338`

- Startup completed at supervisor revision 37: 937 main-image functions, 327 reviewed SDK SKIP, 1 entry WIP, 609 TODO; no runtime equivalence proof yet
- Native entry: private `GameEntry` retains original marker `0x8001127C`, configures BIOS interrupts, calls `RunGame` and retains the original final BREAK; game BSS/heap storage is native. Only GPU packet pools and ordering tables retain device RAM clearing. Contracts and verified stage result: `status/refactoring-compliance-execution/stage-07/bootstrap-contract.json` and `status/refactoring-compliance-execution/stage-07/changes.json`
- Historical startup entry compilation: `status/build/startup-entry-compile.log`; current Debug x86 and Release x86 acceptance is tracked by `status/refactoring-compliance-execution/progress.json`
- DuckStation roles use ports user=2439, lockstep=2440, audit=2441, trace=2442 with isolated data roots; root launchers are `DuckStation.bat` and `record.bat`

- Coverage follows the explicit user constraints: no builds; save sequential `src/batch_N.c` modules starting the next module only after the previous module already exceeds 2000 physical lines; keep each function whole; use standard xport `r_u*`, `r_s*` and `w_u*` guest-memory accessors without project-local replacements

- Active user-authorized goal: native C++ reconstruction and gradual native RAM migration; DRAM/scratchpad layout preservation is waived for this refactoring. Current state: `status/native-refactoring/progress.json`; original-address mapping: `status/native-refactoring/method-map.json`; recoverable C baseline: `status/native-refactoring/baseline-manifest.json`. Historical coverage constraints do not govern this phase
- Compliance execution: `status/refactoring-compliance-execution/plan.ru.txt` and `status/refactoring-compliance-execution/progress.json`; implementation within each stage has no checks, then the complete Debug x86 gate and Release x86 build run at its endpoint
