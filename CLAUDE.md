# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

rmw (ReMove to Waste) is a C command-line trashcan/recycle bin utility (version 0.10.0-dev). It implements the [FreeDesktop.org Trash specification](https://specifications.freedesktop.org/trash-spec/trashspec-latest.html) and adds features like timed auto-purging and a ncurses-based restore menu.

## Platform scope

**POSIX only — Windows is no longer a target.** There are no plans for rmw to run on Windows. Over time, existing Windows references should be removed as they're encountered (e.g. the `// TODO: make it compatible with Windows` in `config_rmw.c`, any `$UID`/path-separator hedging that only exists for Windows). Don't add new Windows-portability code or caveats. When touching code that carries a Windows note, removing it is in scope; don't go on a dedicated sweep unless asked.

## Guidelines for AI-assisted work

These apply to anyone using Claude (or a similar assistant) in this repo —
maintainer or contributor. (For the maintainer they mirror his user-level
instructions, which are canonical if the two ever disagree.)

### Conduct and voice

- **No sycophancy.** Don't agree with the user just because they're the user.
  When you disagree with a proposed course of action, debate it first — offer
  the contrary view before going along. Capitulating without push-back is
  unhelpful.
- Don't oversell. Mark speculation as speculation; state trade-offs plainly.
- Match response length to the question; one-line questions get one-line answers.
- Avoid the word "drop" (ambiguous). Use "remove"/"delete" when subtracting,
  "add"/"create"/"put" when adding.

### Posting publicly as an AI (issues, PR/commit bodies, comments)

- Start with a short blockquote preface identifying yourself as Claude, an LLM
  made by Anthropic, with model version, noting the content was posted at the
  user's direction.
- Write from the AI's perspective: first person for what the AI did, the
  human's handle (e.g. `@andy5995`) for what the human did. Never "we" when it
  hides who did what.
- Sound like an LLM, not a human: no anthropomorphic phrases ("happy to",
  "excited", "glad to help"), no colloquialisms or slang. Neutral, factual
  wording ("available for follow-up", "the analysis showed").
- Mark anything unverified as unverified.
- One-line commit subjects need no preface (the `Co-Authored-By: Claude …`
  trailer attributes them); a commit message **body** does need the preface.
- Never put a third party's contact info (email, etc.) in any repo artifact —
  even publicly-known addresses. Your own (the contributor's) email is fine.

### Code and comments

- **Comments are rare and short.** Default to none; add one only when the
  *why* is non-obvious (hidden constraint, prior bug, surprising invariant).
  Public `.h` docblocks are exempt — contract documentation can be thorough.
- No error handling/validation for cases that can't happen; validate only at
  real boundaries (user input, external APIs).
- No features, refactors, or abstractions beyond the task.
- Don't change existing code style mid-edit; match the surrounding lines.

### Git discipline

- Only commit when the user asks. Stage and commit are separate steps: stage,
  show what's staged, pause for a clear go-ahead, then commit. A clarification
  or tweak is not a go-ahead.
- Map git verbs 1:1 to the user's words: "amend" ≠ push, "push" ≠ amend/rebase.
  Never push unless explicitly asked; never force-push the main branch.
- Commit messages: one-line subject by default; body only for a non-obvious
  *why*; the message must never be longer than the diff. After committing,
  report the sha and stop — no "want me to push?", no PR instructions.
- Use `rmw` instead of `rm` for deletions outside git's reach; `git rm` is
  fine for tracked files (history is the safety net).

### Environment

- Don't install packages on the host (any package manager, any scope); name
  the package and let the user install it. Throwaway containers are exempt.
- Wrap long builds in `nice -n 19 ionice -c 3`; background long jobs and tee
  to `/tmp/<name>.log`, and surface the log path.
- Run `meson fmt -eir` from the project root after editing any `meson.build`.

## Build System

rmw uses **Meson + Ninja**. Use **`_build-debug`** as the build directory (not `builddir`).

```sh
# First-time setup
meson setup _build-debug

# Build
ninja -C _build-debug

# Run all tests
meson test -C _build-debug

# Run a single test by name
meson test -C _build-debug test_strings_rmw

# View/change build options
meson configure _build-debug
```

Key build options (`meson_options.txt`):
- `-Dbuild_tests=false` — skip building tests
- `-Dwant_ficlone=false` — disable btrfs/bcachefs/xfs reflink support (skips linux-headers requirement)
- `-Dnls=false` — English-only, no gettext dependency
- `-Dwithout-curses=true` — build without ncurses restore menu

Dependencies: `glib-2.0 >= 2.52`, `gio-2.0 >= 2.52`, `gio-unix-2.0`, `ncursesw` + menu lib, `gettext`. The `canfigger` config-parsing library is a subproject (falls back to `subprojects/canfigger/`).

## Coding Style

GNU Coding Standards style, but **braces are not indented**:

```c
if (cli_user_options.list)
{
  list_waste_folders(st_config_data.st_waste_folder_props_head);
  return 0;
}
```

Auto-format with: `indent -ci2 -bl -bli0 -nut -npcs`

## Architecture

### Entry point and flow

`src/main.c` → parses CLI options (`parse_cli_options`) → reads config (`parse_config_file`) → dispatches to restore, purge, or move operations.

### Key structs

| Struct | Defined in | Purpose |
|--------|-----------|---------|
| `st_waste` | `trashinfo.h` | Linked list node for a waste directory; holds `parent`, `info`, `files` subdirs, `dev_num`, `removable`, `is_ficlone_fs` |
| `rmw_target` | `trashinfo.h` | A file being rmw'd: `real_path`, `base_name`, `waste_dest_name`, `is_duplicate` |
| `st_config` | `config_rmw.h` | Parsed config: `st_waste_folder_props_head` list, `expire_age`, `force_required`, `uid` |
| `rmw_options` | `parse_cli_options.h` | All CLI flags (restore, purge, list, dry-run, etc.) |
| `st_time` | `time_rmw.h` | Time state: `now`, `deletion_date`, format strings |
| `st_mount_trash` | `topdir_trash.h` | Node for a discovered mount + its $topdir trash paths (iss-525 work) |

### Module responsibilities

- **`config_rmw.c`** — reads `~/.config/rmwrc`, builds the `st_waste` linked list
- **`trashinfo.c`** — creates and parses `.trashinfo` files (FreeDesktop spec)
- **`restore.c`** — restores files; includes ncurses selection menu (`restore_select`) and undo-last-rmw
- **`purging.c`** — deletes files older than `expire_age` days; orphan maintenance (`.trashinfo` with no matching file)
- **`ficlone.c`** — Linux-only: reflink cloning across btrfs/bcachefs/xfs subvolumes via `FICLONE` ioctl. No plain byte-copy fallback: if reflink isn't possible, the move fails with EXDEV
- **`topdir_trash.c`** (iss-525 branch) — FreeDesktop `$topdir` trash discovery: enumerates mounts via GLib (`build_mount_trash_list`, with pseudo/network/fuse/readonly filtering in `mount_is_eligible`), and resolves a file's mount for the per-file fallback (`find_topdir_trash`)
- **`src/bsdutils/`** — vendored BSD `rm` implementation used by the purge path
- **`strings_rmw.c`**, **`utils.c`**, **`messages.c`**, **`time_rmw.c`**, **`globals.c`** — utilities and shared state

### Waste directory layout

Each waste folder has two subdirectories following the FreeDesktop spec:
- `files/` — deleted files live here
- `info/` — corresponding `.trashinfo` files (stores original path + deletion date)

### Gotchas (hard-won)

- **`rename(2)` returns EXDEV across *mount* boundaries even when `st_dev` is identical** (bind mounts). Matching a waste folder by device number alone does not guarantee `rename` can reach it. `remove_to_waste` handles this: on EXDEV it skips the waste so the `$topdir` fallback can run, capped at one fallback attempt per file (`fb_attempted_arg`) to prevent retry loops.
- **Don't use `g_unix_mount_for()` to find a file's mount.** It walks up the path until `st_dev` changes, which walks straight past bind mounts to the parent mount. `find_topdir_trash` instead takes the longest mount-table-path prefix of the file's `realpath` (what `findmnt -T` does). Same blind spot exists in `stat -c %m` — use `mountpoint -q` in tests.
- **`trim_char('/', …)` collapses an all-slash argument to `""`.** The `rmw /` easter egg depends on the empty-after-trim check sitting immediately after `trim_char` in `remove_to_waste`; a check on `argv` or after basename runs too late (basename of `""` is `"."`, which the dotdir guard swallows). This regressed once (#450 fix) — `test_trim_char` in `utils.c` covers the collapse.
- **Write-error checking convention:** don't check `fprintf`/`fputs` return values on buffered streams; `close_file()` (which checks `fclose`) is the single write-error gate.
- **The ficlone move is copy-then-delete, not `rename`, for cross-filesystem/reflink cases** (`rename(2)` can't cross filesystems or mount boundaries). The single-file path: `clone_file()` clones to the waste (no source removal); `do_ficlone()` calls it then unlinks the source, rolling back (unlinks the dest) if the source unlink fails — so a file is never left only in the waste. The directory path (`do_ficlone_dir()`) uses the same copy-then-delete discipline as coreutils `mv` across filesystems: `clone_tree()` copies the whole tree first **without touching the source**; only if that fully succeeds does `remove_tree()` delete the source. A clone failure rolls back cleanly (the partial dst is removed, source untouched); a source-removal failure keeps the copy in the waste and warns (data is never lost — but a copy may remain in both places, like `mv`). `test_ficlone_safeguards.sh` (sourced by the btrfs/bcachefs tests) covers both failure paths.
- **`-o` / orphan handling never permanently deletes user data.** `-o` alone repairs a `files/` entry that has no `.trashinfo` by *creating* one. The opposite orphan — a `.trashinfo` with no data file — is removed only during a purge at force level 2 (`rmw -offg` = `-o -f -f -g`), and that deletes just the dangling metadata. The core invariant holds: rmw won't permanently delete a `files/` entry without first finding its `.trashinfo`.

### Testing

C unit tests (`test_strings_rmw`, `test_utils`, `test_restore`) are compiled with `-DTEST_LIB`, which exposes internal functions via `#ifdef TEST_LIB` guards in the source files. `test_topdir_trash` (iss-525) includes its translation unit directly to reach static functions.

Shell integration tests run against the built `rmw` binary using `RMW_FAKE_HOME` (set to `_build-debug/test/rmw-tests-home/`) to avoid touching the real home directory. `RMW_FAKE_HOME` also suppresses topdir discovery (it would scan the host's real mounts); a test that needs discovery sets `RMW_CHECK_DISCOVERY` to re-enable it against mounts the test controls.

- `test_basic.sh`, `test_purging.sh`, `test_restore.sh` — core flows, no special requirements
- `test_waste_negation.sh` (iss-525) — the `no-add` WASTE attribute is skipped for removals but listed/restored/purged
- `test_media_root.sh` — needs `/tmp` to be a top-level mount on its own device; skips otherwise
- `test_exdev_fallback.sh` (iss-525) — bind-mount EXDEV regression test; builds a 2 MiB tmpfs ramdisk with a bind mount inside it. Needs passwordless `sudo`; SKIPs otherwise
- `test_discovery.sh` (iss-525) — discovery's excluded-fs rule, run inside an unprivileged mount namespace (`unshare --mount --map-root-user`, no sudo); SKIPs if unprivileged userns is unavailable
- `test_discovery_loopback.sh` (iss-525) — discovery of an eligible fs on its own device (candidate listing + fresh-trash auto-create); generates an ext4 loop image at run time. Needs passwordless `sudo` + `mkfs.ext4`; SKIPs otherwise
- `test_btrfs_clone.sh`, `test_bcachefs.sh` — reflink tests against prebuilt loopback images (`test/*.img`, not in the repo; CI downloads them, sha256sums are committed). Need the image + passwordless `sudo`; SKIP otherwise

To test Year 2038 (epochalypse) behavior when `faketime` is installed:
```sh
meson test -C _build-debug --setup epochalypse
```

## Known deferred items

- `--empty` is currently blocked entirely when `expire_age = 0`; intended behavior is `--empty -f` overrides (task tracked)
- A failed `fclose` in `create_trashinfo` leaves a partial `.trashinfo` on disk (task tracked)
- Scattered commented-out debug lines (`main.c`, `config_rmw.c`, `time_rmw.c`) — optional sweep, low priority
