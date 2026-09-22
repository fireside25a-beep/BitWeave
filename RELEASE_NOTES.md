# BitWeave 1.4.0

This release narrows BitWeave to a forward-only native authoring workflow.

## Changes

- direct `.bwx` → Linux `ET_EXEC`, Linux PIE, and Windows PE32+ build commands;
- direct `.bwx` → canonical `.bits` build command;
- unresolved labels now fail on the final encoding pass;
- optional AI command now asks a local model to author a new `.bwx` program from a brief;
- command surface simplified around authoring, build, verification, hashes, manifests, and ABI references;
- GitHub visual atlas rebuilt to show the forward build flow only;
- tests rewritten around forward-authored source and generated native outputs.

## Repository hygiene

The source repository no longer carries local compiler/sanitizer logs, generated example executables, release binaries, or session-specific verification captures. GitHub Actions now performs the build/test matrix from the tracked source and tests.
