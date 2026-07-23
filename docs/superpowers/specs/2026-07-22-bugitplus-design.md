# BugItPlus — Design Spec

**Date:** 2026-07-22
**Status:** Approved for planning

## Purpose

Improve the engine's native `BugIt`/`BugItGo` workflow. Primary goal is a workflow
improvement for daily use; secondary goal is a portfolio piece demonstrating clean
Runtime/Editor module separation in an Unreal Engine plugin.

Verified against the UE 5.7 engine source (`EditorServer.cpp`, `CheatManager.cpp`),
vanilla `BugIt`/`BugItGo` are actually **two separate implementations**, not one:

- **Editor-browsing path** — `UEditorEngine::HandleBugItCommand`/`HandleBugItGoCommand`
  (`UnrealEd` module). Only runs when not in PIE. Captures the level-viewport camera,
  builds `"BugItGo X Y Z Pitch Yaw Roll"`, and **already copies it to the clipboard**.
  No screenshot, no file, no map name, no description.
- **In-game path** — `UCheatManager::BugIt`/`BugItGo` (`Engine` module — runs in PIE and
  packaged non-shipping builds). Takes a screenshot, writes a `.txt` report to
  `Saved/BugIt/<PlatformName>/<desc>/` including the map name, and teleports the pawn
  on `BugItGo` — but does **not** copy anything to the clipboard, and `BugItGo` has no
  map-switch logic at all (it just teleports on whatever map is currently loaded).

The plugin adds two specific improvements without touching or requiring changes to
either native command:

1. **Clipboard copy for the in-game path.** The editor-browsing path already copies
   its go-string to the clipboard; the in-game path (PIE/packaged) is the one missing
   it today. `BugItPlus` closes that gap so both paths behave consistently.
2. **Cross-map jump, for both paths.** Neither native path switches maps today — if
   you paste a go-string while a different map is open, the tool switches maps first,
   then applies the saved location.

## Non-goals (v1)

Everything below is explicitly out of scope for this spec and deferred to a future
design:

- Slate dockable panel for browsing report history
- Hotkey + popup trigger (alternative to the console command)
- Richer metadata (engine changelist/version, level-streaming state)
- Structured storage / JSON or CSV index of reports
- Screenshot annotation
- Bug tracker integration (Jira, Trello, markdown export)
- Team / source-control sharing of reports
- Interface-based (dependency-inversion) rewrite of the Runtime/Editor hook —
  functionally equivalent to the delegate approach below, worth revisiting only as a
  separate portfolio exercise

## Command hookup

Native `BugIt`/`BugItGo` live in two different engine classes depending on context
(`UCheatManager` in-game, `UEditorEngine` while browsing in the editor — see Purpose
above), neither of which this plugin can cleanly subclass/override project-wide without
either requiring a custom `CheatManager` class per project (breaks "just enable the
plugin" portability) or patching engine-only editor exec dispatch (not possible from a
plugin at all). Instead, this plugin registers two new, independent console commands
via `IConsoleManager`:

- `BugItPlus "<description>"`
- `BugItGoPlus "<go-string>"`

Because `IConsoleManager` commands are globally reachable — the PIE viewport console,
a packaged build's console, and the Editor's own Cmd line at the bottom of the Output
Log — these commands work in all three contexts without extra plumbing, matching the
requirement that BugItPlus works everywhere vanilla BugIt works today.

## Module architecture

Two modules, following the project's existing Runtime/Editor split pattern:

```
Plugins/BugItPlus/
  Source/
    BugItPlus/            # Runtime module
    BugItPlusEditor/       # Editor module
```

**Constraint:** the console commands must work in PIE and packaged builds (Runtime),
but "editor, not playing" behavior needs `UnrealEd`/`LevelEditor` APIs, which a Runtime
module must never link against.

**Chosen approach — delegate hook:** the Runtime module registers both console
commands and owns everything that doesn't need editor APIs (capture, screenshot,
clipboard copy, PIE/packaged travel-then-teleport). It exposes two static delegate
slots that the Editor module binds to in `StartupModule()` and unbinds in
`ShutdownModule()`:

```cpp
// BugItPlus module (Runtime) — no UnrealEd dependency
DECLARE_DELEGATE_RetVal(bool, FBugItPlusEditorCaptureDelegate /* out MapName, out FTransform */);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FBugItPlusEditorJumpDelegate,
    const FString& /*MapName*/, const FTransform& /*Transform*/);

class FBugItPlusModule : public IModuleInterface
{
public:
    static FBugItPlusEditorCaptureDelegate EditorCaptureDelegate;
    static FBugItPlusEditorJumpDelegate EditorJumpDelegate;
};
```

The Editor module binds both via `BindStatic` (no captured object lifetime, avoids any
dangling-pointer risk on module unload/hot-reload) to implementations that use
`LevelEditorSubsystem` and the active viewport — the same pattern already used by the
CommonMaps plugin for map presets.

If the Editor module isn't loaded (e.g. a packaged build), the delegates are simply
unbound; `IsBound()` checks in the Runtime module's command handlers fall through to
the PIE/packaged code path instead. No crash, no missing symbols — it degrades
gracefully by construction.

**Rejected alternatives:**
- *Commands registered inside the Editor module* — an `Editor`-type module is never
  compiled into or loaded by a packaged build, so the commands would silently not
  exist outside the editor. Breaks the "works wherever vanilla BugIt works" requirement.
- *Interface class instead of a delegate* (`IBugItPlusEditorHandler` with a
  register/unregister singleton) — functionally equivalent to the delegate approach,
  slightly more ceremony (separate header, manual lifecycle management) for the same
  result. Valid alternative, not chosen for v1.

## Command flow

### `BugItPlus "<description>"`

1. Console command hits its handler in the Runtime module.
2. Resolve a location + map name, in order:
   - **PIE/packaged, pawn exists** — same source vanilla `UCheatManager::BugIt` uses:
     `PlayerController->GetPlayerViewPoint()`, overridden with the pawn's actor
     location when a pawn exists; map name from `GetWorld()->GetMapName()`.
   - **Editor, not playing** — call `EditorCaptureDelegate.Execute()`; the Editor
     module's bound implementation reads the active level-viewport camera transform
     and the currently open level's name (vanilla's editor-side `BugIt` doesn't
     capture a map name at all today — this is new).
   - **Neither available** — log a warning and abort. No partial/silent report.
3. Capture a screenshot. In the PIE/packaged case, reuse vanilla's mechanism exactly:
   `PlayerController->ConsoleCommand(TEXT("BUGSCREENSHOTWITHHUDINFO <path>"))`. In the
   editor-only case there's no equivalent to reuse (vanilla's editor path doesn't
   screenshot), so this step is skipped when there's no pawn/game world.
4. Build the go-string: `"MapName X Y Z Pitch Yaw Roll"`. Note this is a **new format**,
   not an extension of vanilla's — vanilla's go-string has no map name in it, so
   `BugItGoPlus` needs its own parser rather than reusing engine parsing helpers like
   `GetFVECTORSpaceDelimited`.
5. Write the report to `Saved/BugItPlus/<PlatformName>/<desc>/` (see Storage below).
6. Copy the go-string to the clipboard via `FPlatformApplicationMisc::ClipboardCopy()`
   (an `ApplicationCore` API, available in Runtime, no Editor dependency). This is the
   actual gap being closed — vanilla's in-game `BugIt` never does this.
7. Log confirmation to screen/output log.

### `BugItGoPlus "<go-string>"`

1. Handler in the Runtime module parses the string into `MapName` + `FTransform`.
2. Compare `MapName` to the currently loaded map, then branch:

   | | Same map | Different map |
   |---|---|---|
   | **PIE/packaged, pawn exists** | Teleport pawn using the same approach as vanilla `UCheatManager::BugItWorker`: toggle ghost mode, `Pawn->TeleportTo()` + `FaceRotation()`, `PlayerController->SetControlRotation()`, toggle ghost mode again | `UGameplayStatics::OpenLevel(...)`, then a one-shot binding on `FCoreUObjectDelegates::PostLoadMapWithWorld` applies the same teleport sequence to the new pawn once the map finishes loading, then unbinds itself |
   | **Editor, not playing** | `EditorJumpDelegate.Execute(MapName, Transform)` — Editor-side handler just moves the viewport camera | Same delegate call — the Editor-side handler detects the map mismatch itself and calls `LevelEditorSubsystem->LoadLevel()` before moving the camera |

3. Log confirmation ("Jumped to X" / "Loading X...").

**Known risk area:** the "different map, PIE/packaged" cell depends on `OpenLevel`'s
travel being asynchronous — the teleport must wait for `PostLoadMapWithWorld` rather
than happening inline. This is the most fragile part of v1 and the most likely to need
iteration during implementation.

## Storage format

Deliberately unambitious for v1 — mirrors vanilla's own convention
(`FPaths::BugItDir()` = `Saved/BugIt/<PlatformName>/`), just in a sibling folder rather
than mixed into vanilla's own output, since our go-string format isn't compatible with
vanilla's parser:

- **Location:** `Saved/BugItPlus/<PlatformName>/<desc>/`, mirroring vanilla's
  `Saved/BugIt/<PlatformName>/<desc>/` layout one level over.
- **Per-report layout:** one subfolder per report, named from description + a
  sequential suffix (matching `FFileHelper::GenerateNextBitmapFilename`'s convention,
  the same helper vanilla uses), containing:
  - the screenshot (`.png`), when one was captured (PIE/packaged only — see flow above)
  - a small text file with: description, map name, and the `BugItGoPlus` string (a
    durable backup of what's already on the clipboard, not the only way to retrieve it)
- **No manifest/index file in v1.** Each report is self-contained. A central
  JSON/CSV index becomes worth building once a v2 Slate panel needs to enumerate
  reports — building it now with no consumer would be speculative.

## Testing considerations

- Verify `BugItPlus`/`BugItGoPlus` are reachable from all three console contexts (PIE,
  packaged non-shipping, Editor Cmd line).
- Verify graceful degradation when the Editor module is absent (packaged build):
  delegates unbound, no crash, PIE/packaged path still works.
- Verify same-map and different-map jump behavior in both PIE and Editor-only contexts.
- Verify clipboard content matches the go-string written to the report file.
