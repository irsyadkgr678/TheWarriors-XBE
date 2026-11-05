# Contributing

## Ground rules

This project ships no game code and no game data. Do not commit:

- `assets/` contents: ISOs, `default.xbe`, `*.wad*`, `audio/`, `bik/`
- `generated/`: the recompiled game code, which is a derivative of the game's binary
- `TheWarriors.dll`, crash dumps, logs, saves, `settings.toml`

`.gitignore` already covers all of these. If a change needs a test with the real game, describe how you tested it in
the pull request instead of attaching files.

## Language

- Source code, comments, documentation and commit messages are written in **English**.
- Indonesian versions of player-facing documents use the `.id.md` suffix: `README.id.md`,
  `docs/INSTALL-FOR-TESTERS.id.md`. Keep them in sync with the English version when you change either.

## Commit messages

English, imperative mood, first line under 72 characters, no trailing period. A short body explains *why* when the
change is not obvious. Prefixes used in this repository:

```
feat:     new functionality visible to players
fix:      bug fix
docs:     documentation only
build:    build scripts, CMake, packaging
refactor: restructuring without behaviour change
```

Examples from the history:

```
fix: keep the APU sample counter committed for XACT
docs: link the v0.1.0-alpha release
build: package the archive without 7-Zip
```

Keep one logical change per commit. If a commit touches both a fix and a rewrite of the docs, split it.

## Reporting bugs

Use the bug report form (Issues → New issue). It asks for the build, how the game data is provided, your system, the
log tail, and the newest crash dump. Reports without a log are usually not actionable.

## Pull requests

- Explain what changed and how you verified it. "Boots to the menu on <GPU>, mission 1-2 played" is enough.
- Keep the SDK changes minimal and documented in `PORTING-STATUS.md`, under the section describing the fix.
- If you touch `sdk/runtime/kernel/`, re-read section 4 of `PORTING-STATUS.md` first: a newer upstream runtime
  regressed this title's boot, and the current files are pinned deliberately.
