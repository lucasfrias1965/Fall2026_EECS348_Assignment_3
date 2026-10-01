# ADVICE 2 — Re-grade after ADVICE_1 fixes

Re-graded `src/human.cpp` against `Rubric 2.pdf` (same rubric used in ADVICE_1), checking specifically whether the two zero-point items from that session got fixed. Recompiled with `g++ -std=c++11 -Wall -Wextra` (clean, no warnings), and ran it against both `exec/DEMO.txt` and the exact sample input/output from `prompt_rubric/PROMPT.md` — output matches byte-for-byte except one trailing blank line (see note below, not scored).

`git diff --stat` confirms only `src/human.cpp` changed since the `ADVICE_1.md` session (24 insertions, 7 deletions) — `ANALYSIS.md` is untouched, so its scores carry over unchanged from ADVICE_1.

## Scorecard

| Criterion | Points possible | Score | Change | Why |
|---|---|---|---|---|
| C Program meets requirements | 30 | **30** | — | Builds clean, executes sample test correctly, OOP (`Email`/`MaxHeap`/`InboxProcessor`/`Application` classes), from-scratch vector-based MaxHeap, no `<queue>` or pre-existing heap calls |
| GenAI Program & Access | 4 | **4** | — | Claude Sonnet and Moonshot Kimi K3 both named with access method described in `ANALYSIS.md` |
| Prompt & Raw Code | 6 | **6** | — | Identical prompt confirmed, full raw code from both included |
| Code Analysis | 10 | **10** | — | Correctness/time/space/maintainability all covered with benchmarks |
| Selection & Justification | 5 | **5** | — | Sonnet selected, justified with data |
| Code Improvement | 10 | **10** | — | Vector + enum rewrite, before/after benchmarks |
| Prolog Comments | 7 | **7** | **+7** | All 7 items now present: name, description, input, output, collaborators, author, created+revised dates |
| In-Line Comments | 14 | **14** | — | Every line commented |
| Collaborators | 14 | **14** | **+14** | Prolog names Sonnet (base design), Opus 5.5 (vector/enum revision), and the StackOverflow citation with URL; inline `// --- revised with Claude Opus 5.5 ---` tags mark the exact two spots touched, satisfying the full-marks "what part of the code they collaborated on" bar |
| **Total** | **100** | **100** | **+21** | |

Both fixes from ADVICE_1 landed exactly as suggested and closed the full 21-point gap: **79 → 100**.

---

## Remaining nitpicks (not rubric-costing, but worth a look before submission)

1. **Stale filename in the prolog's Build/Run lines** (`src/human.cpp:17-18`): they still say `g++ ... -o ceo_inbox ceo_inbox.cpp` / `./ceo_inbox inbox.txt`, left over from before the file was renamed to `human.cpp`. The "Program" field correctly says `human.cpp`, so this is just an internal inconsistency — not one of the 7 required prolog items, so it doesn't cost points, but a grader copy-pasting the build line will get a "no such file" error.
2. **Trailing blank line**: `HandleCount()` always prints `"\n\n"`, so a `COUNT` as the last command in a file leaves one extra blank line at EOF that isn't in the prompt's sample output. Harmless for any grader comparing meaningful output, but worth knowing if the Grader's Test File is diffed byte-for-byte.
3. **`COPY.md` was not deleted.** ADVICE_1's housekeeping item #3 flagged this as the leftover *Assignment 2* write-up (opens "Make this program in C," references the old Luna/GPT comparison) sitting at the project root — it's still there (2346 lines, unchanged). It isn't rubric-scored, but it's the first file a grader alphabetically opens in the submission root and could easily be mistaken for part of this assignment, or just read as a sign the repo wasn't cleaned up. Worth a `git rm COPY.md` before submitting.
4. Typos from ADVICE_1 ("tstore," "sttring," "exicitng") are still present in comments — cosmetic only, doesn't affect the In-Line Comments score since intent is still clear everywhere.

None of the above change the score. The submission is at full marks on the rubric as it currently stands; items 1 and 3 are the only two worth a quick cleanup pass before handing it in.
