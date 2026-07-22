# BugItPlus — Design Spec

**Date:** 2026-07-22
**Status:** Approved for planning

## Purpose

Improve the engine's native `BugIt`/`BugItGo` workflow. Primary goal is a workflow
improvement for daily use; secondary goal is a portfolio piece demonstrating clean
Runtime/Editor module separation in an Unreal Engine plugin.

`BugIt`/`BugItGo` are exec functions on `APlayerController`. They capture the current
camera location/rotation plus a screenshot, and let you paste the location string back
in to teleport to that exact spot. The plugin adds two specific improvements without
touching or requiring changes to the native commands:

1. The generated "go back" string is copied straight to the clipboard instead of
   requiring a trip into the `Saved/Bugit` text file.
2. The "go back" string also works across maps — if you paste it while a different
   map is open, the tool switches maps first, then applies the saved location.

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

Native `BugIt`/`BugItGo` are exec functions on `APlayerController`. Cleanly overriding
them would require every consuming project to use a custom PlayerController subclass,
which breaks "just enable the plugin" portability. Instead, this plugin registers two
new, independent console commands via `IConsoleManager`:

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
   - **PIE/packaged, pawn exists** — `PlayerController->PlayerCameraManager` for
     location/rotation, `GetWorld()->GetMapName()` for the map. Same source vanilla
     `BugIt` uses.
   - **Editor, not playing** — call `EditorCaptureDelegate.Execute()`; the Editor
     module's bound implementation reads the active level-viewport camera transform
     and the currently open level's name.
   - **Neither available** — log a warning and abort. No partial/silent report.
3. Capture a screenshot, reusing vanilla's screenshot request path as-is.
4. Build the go-string: `"MapName X Y Z Pitch Yaw Roll"`.
5. Write the report to `Saved/Bugit/` (see Storage below).
6. Copy the go-string to the clipboard via `FPlatformApplicationMisc::ClipboardCopy()`
   (an `ApplicationCore` API, available in Runtime, no Editor dependency).
7. Log confirmation to screen/output log.

### `BugItGoPlus "<go-string>"`

1. Handler in the Runtime module parses the string into `MapName` + `FTransform`.
2. Compare `MapName` to the currently loaded map, then branch:

   | | Same map | Different map |
   |---|---|---|
   | **PIE/packaged, pawn exists** | Teleport pawn directly (`SetActorLocationAndRotation`) — same as vanilla `BugItGo` | `UGameplayStatics::OpenLevel(...)`, then a one-shot binding on `FCoreUObjectDelegates::PostLoadMapWithWorld` applies the saved transform to the new pawn once the map finishes loading, then unbinds itself |
   | **Editor, not playing** | `EditorJumpDelegate.Execute(MapName, Transform)` — Editor-side handler just moves the viewport camera | Same delegate call — the Editor-side handler detects the map mismatch itself and calls `LevelEditorSubsystem->LoadLevel()` before moving the camera |

3. Log confirmation ("Jumped to X" / "Loading X...").

**Known risk area:** the "different map, PIE/packaged" cell depends on `OpenLevel`'s
travel being asynchronous — the teleport must wait for `PostLoadMapWithWorld` rather
than happening inline. This is the most fragile part of v1 and the most likely to need
iteration during implementation.

## Storage format

Deliberately unambitious for v1 — no new convention to learn on top of vanilla:

- **Location:** `Saved/Bugit/`, same folder vanilla `BugIt` already uses.
- **Per-report layout:** one subfolder per report, named from description + timestamp
  (matching vanilla's existing convention so old and new reports sit side-by-side
  consistently), containing:
  - the screenshot (`.png`)
  - a small text file with: description, map name, and the `BugItGoPlus` string (a
    durable backup of what's already on the clipboard, not the only way to retrieve it)
- **No manifest/index file in v1.** Each report is self-contained. A central
  JSON/CSV index becomes worth building once a v2 Slate panel needs to enumerate
  reports — building it now with no consumer would be speculative.

**Implementation note:** the exact vanilla folder/file naming convention above is from
memory, not verified. The first implementation step should read the engine's
`PlayerController.cpp` source to confirm the precise format before matching it.

## Testing considerations

- Verify `BugItPlus`/`BugItGoPlus` are reachable from all three console contexts (PIE,
  packaged non-shipping, Editor Cmd line).
- Verify graceful degradation when the Editor module is absent (packaged build):
  delegates unbound, no crash, PIE/packaged path still works.
- Verify same-map and different-map jump behavior in both PIE and Editor-only contexts.
- Verify clipboard content matches the go-string written to the report file.
