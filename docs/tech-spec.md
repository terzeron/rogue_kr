# Tech Spec: Additional Message Localization

## Context
- Remaining English literals in the C sources still surface directly to players despite the new `msg_get` catalog support.
- Examples found: the compact status line format string in `io.c`, the terse direction prompt in `misc.c`, and the wizard-password prompt in `main.c`.
- There is also an English-only comparison in `monsters.c` (`"it"`) that breaks the apostrophe logic when monster names are translated.

## Requirements
1. All user-visible prompts for directions and wizard password must come from the message catalog so translators can adapt them.
2. The stats banner must rely on the existing `MSG_STATUS_FORMAT` template for both the `msg()` path and the `printw()` path so only one format string needs to be maintained.
3. Monster name comparisons must respect localized strings (no hard-coded `"it"`).
4. Message catalog files (`en.msg`, `ko.msg`) must define any new keys, packaged with appropriate Korean translations.

## Proposed Changes
- **Status line (`io.c`)**: replace the literal `printw("%s: %d ...")` call with `printw(msg_get("MSG_STATUS_FORMAT"), ...)` so both verbose and terse updates share the same localized template.
- **Wizard password prompt (`main.c`)**: add a catalog key (e.g., `MSG_PROMPT_WIZARD_PASSWORD`) and call `msg_get` before `md_getpass`.
- **Direction prompt (`misc.c`)**: add a localized terse string (e.g., `MSG_MISC_DIRECTION_TERSE`) and use it instead of the literal `"direction: "`.
- **Monster apostrophe logic (`monsters.c`)**: compare against `msg_get("MSG_IT")` instead of the English literal; this keeps Korean grammar from forcing an unnecessary apostrophe.
- **Message catalog**: add English/Korean text for the new wizard and direction prompts; reuse existing translations for other reused strings.

## Acceptance Criteria
- Running the game in Korean no longer shows English fragments when:
  - requesting the wizard password,
  - issuing direction-based commands in terse mode,
  - displaying the stats banner,
  - showing monster-confusion apostrophes.
- `en.msg`/`ko.msg` contain the new keys with reviewed translations.
- Existing behavior in English remains unchanged (status prints, prompts, apostrophe suppression).

## Testing
- No automated tests cover these prompts directly; perform a focused build/run (or unit harness if available) isn’t strictly necessary after string refactors, but compile-time checks (`make`) should still pass if run locally.
