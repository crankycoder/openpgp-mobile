# Claude Development Guidelines

## 🏗️ Repository Structure - Three Distinct Subprojects

This repository contains **three separate subprojects** that should be clearly distinguished in all documentation:

## 🤖 AUTO-BOOTSTRAP ON STARTUP

When Claude Code starts a new session, automatically:

1. Bootstrap all LLM prompts using `mcp__llm-prompts__startLlmPrompts`
2. Review the bootstrapped prompts for:
   - Code change planning instructions
   - Task completion instructions
   - Git commit standards
   - Git workflow best practices
   - LLM testing guidelines
   - MCP tool setup guidelines

This ensures all best practices are loaded without manual intervention.

# 🚨 CRITICAL: TOOL PRIORITY ORDER 🚨

## 🚀 Git Operations Priority

### For Git-Related Commands:

1. **FIRST TRY**: 1. Desktop Commander (`mcp__desktop-commander__execute_command`)

| Git Operation         | Priority 1: Desktop Commander |
| --------------------- | ----------------------------- |
| Create feature branch | `git checkout -b` via DC      |
| Create/Update PR      | `gh pr create` via DC         |
| Check PR status       | `gh pr checks` via DC         |
| Mark PR ready         | `gh pr ready` via DC          |
| Sync with upstream    | `git pull --rebase` via DC    |
| Complete feature      | Manual cleanup via DC         |

### For All Other Operations:

## ⚡ NEVER use the standard Read/Write/Bash tools ⚡

## ⚡ ALWAYS use Desktop Commander MCP equivalents ⚡

| ❌ NEVER USE | ✅ ALWAYS USE                                 |
| ------------ | --------------------------------------------- |
| Read         | `mcp__desktop-commander__read_file`           |
| Write        | `mcp__desktop-commander__write_file`          |
| Edit         | `mcp__desktop-commander__edit_block`          |
| **Bash**     | **`mcp__desktop-commander__execute_command`** |
| LS           | `mcp__desktop-commander__list_directory`      |
| Grep         | `mcp__desktop-commander__search_code`         |
| Glob         | `mcp__desktop-commander__search_files`        |

### 🔴 For non-git commands use Desktop Commander:

- `poetry run pytest` → Use Desktop Commander
- `npm install`, `yarn test` → Use Desktop Commander
- File operations → Use Desktop Commander
- ANY non-git shell command → Use Desktop Commander

**TOOL PRIORITY: Desktop Commander → NEVER standard tools**

---

## 📝 Prompt Journal Format

### Feature-Based Documentation Requirements

Document all development activity in feature-specific journals:

| Requirement  | Details                                                         |
| ------------ | --------------------------------------------------------------- |
| **Location** | `./docs/CLAUDE_WORKSPACE/journal/features/<branch-name>.md`     |
| **Creation** | Automatically created on first commit to feature branch         |
| **Content**  | Document all prompts, responses, and decisions for that feature |
| **Archival** | Move to `archive/YYYY-MM/` when feature merges to dev           |

### Directory Structure

```
docs/CLAUDE_WORKSPACE/journal/
├── features/           # Active feature journals
│   ├── user-auth.md   # One file per active feature
│   └── api-v2.md
└── archive/           # Completed features
    └── 2025-06/
        └── payment-integration.md
```

### Journal Contents

Each feature journal should include:

- **Feature Overview** - What the feature accomplishes
- **User Prompts** - All prompts related to this feature
- **Implementation Details** - Key decisions and approaches
- **Errors & Resolutions** - Problems encountered and solutions
- **Lessons Learned** - Insights for future development

## 🔧 Development Practices

### Core Requirements

- ✅ Always remember to use the prompt journal format above
- ✅ Run tests locally before creating PRs
- ✅ Keep PRs small and focused (one model/feature at a time)
- ✅ Follow the git workflow: `feature branch → dev → main`
- ✅ Document decisions and rationale in the journal
- ✅ When asked to review/update plans, check for architectural changes
- ✅ Create design documents when implementation diverges from plan

### Tool Usage

- 🖥️ **DESKTOP COMMANDER MCP IS MANDATORY** for ALL file and command operations
- 🌐 **ALWAYS USE browsermcp** for web operations (browser navigation, clicking, typing)
- 🔍 **ALWAYS USE mcp**sequentialthinking\*\*\*\* for complex problem solving and multi-step reasoning
- 📝 Use **TodoWrite** tool to track task progress
- 🧠 Use **mcp**memory\*\*\*\* tools for knowledge management

**REMINDER: The standard Bash/Read/Write/Edit tools are FORBIDDEN. Only Desktop Commander.**

### Git Commit Standards

## 🔴 COMMIT FREQUENCY RULES - CRITICAL 🔴

**COMMIT IMMEDIATELY WHEN:**

1. ✅ Any test goes from failing to passing
2. ✅ A new test is written and passes
3. ✅ A feature/function is implemented and its test passes
4. ✅ Any significant code change that moves toward the goal
5. ✅ **After creating new files** - Commit directory structure and initial files
6. ✅ **After each logical unit of work** - Even if tests aren't written yet

**DO NOT:**

- ❌ Wait until everything is done to commit
- ❌ Bundle multiple features in one commit
- ❌ Skip commits because "it's not finished yet"

**EXAMPLE WORKFLOW:**

```
1. Write test → Run test (fails) → Implement code → Test passes → COMMIT
2. Add another test → Test passes → COMMIT
3. Refactor code → Tests still pass → COMMIT
4. Fix lint issue → COMMIT
```

**COMMIT MESSAGE REQUIREMENTS:**

- Commit messages MUST describe:
  - What new tests were added
  - Any tests that were deleted
- Always ensure lefthook precommit hooks are setup and installed
- **Every time tests are run, commit code so we have history**
- **Every commit should update the top level PR message if a PR has been created for the current branch**

### Testing Requirements

Run coverage analysis locally:

```bash
poetry run pytest --cov=src --cov-report=term-missing
```

## 📊 Coverage Analysis

### Running Coverage Reports

```bash
# Run tests with coverage
uv run pytest --cov=src --cov-report=term-missing --cov-report=html

# View HTML report
open htmlcov/index.html
```

### Coverage Standards

- **Target**: >80% coverage on all new code
- **Reports**: Check both terminal and HTML outputs
- **Enforcement**: Verify before creating PRs

## 📋 Pull Request Standards

### PR Description Template

**ALWAYS** use the standardized PR template when creating or updating PRs:

- **Template location**: `/docs/CLAUDE_WORKSPACE/pr-template.md`
- **Required sections**: Summary, Test Results, Test Changes, Changes
- **Auto-include**: Test statistics and coverage data
- **See also**: `/docs/CLAUDE_WORKSPACE/workflow-optimization-strategies/pr-template-compliance.md`

### Creating PRs with Template

```bash
# Use the template for PR body
gh pr create --title "feat: description" --body-file /docs/CLAUDE_WORKSPACE/pr-template.md
```

## 🚀 Quick Reference

| Command                       | Purpose                       |
| ----------------------------- | ----------------------------- |
| `uv run pytest --cov=src`     | Run tests with coverage       |
| `open htmlcov/index.html`     | View detailed coverage report |
| `git checkout -b feature/...` | Create feature branch         |
| `uv run pytest`               | Run tests without coverage    |
| `gh pr edit --body "..."`     | Update PR description         |

## 📌 Important Reminders

1. **Desktop Commander MCP is mandatory** for file and process operations
2. **Document everything** in the daily prompt journal
3. **Test before PR** - always run coverage analysis
4. **Small PRs win** - one feature at a time
5. **Commit messages matter** - describe test changes clearly
6. **Commit after every test run** - maintain complete history
7. **Update PR descriptions** - keep them current with each commit
8. **Use worktrees for parallel work** - never work directly on dev/main
9. **Check for duplicate files** - ensure no duplicate models/tests from copy-paste
10. **Always update prompt journal** - at end of each session
11. Always use the Makefile to build flatbuffer C bindings using: `make flatbuffer_c`

## 🛡️ Protected Files Policy

### Files You Must NEVER Edit

The following files are **READ-ONLY** during normal development:

| Protected File/Pattern                     | Reason                                    |
| ------------------------------------------ | ----------------------------------------- |
| `/docs/CLAUDE_WORKSPACE/plan.md`           | High-level project plan, human-controlled |
| `/docs/CLAUDE_WORKSPACE/architecture/*.md` | Technical designs, require review         |
| `/docs/book/**/*.md`                       | Published documentation                   |
| `/plan.md`                                 | Legacy plan location                      |
| `/README.md`                               | Project documentation                     |
| `/CONTRIBUTORS.md`                         | Contributor list                          |

### Files You May Edit

| File                                                  | Allowed Changes                                                                     |
| ----------------------------------------------------- | ----------------------------------------------------------------------------------- |
| `/docs/CLAUDE_WORKSPACE/tasks.md`                     | ✅ Check off completed tasks<br>~~Strike through~~ descoped items<br>Add PR numbers |
| `/docs/CLAUDE_WORKSPACE/journal/features/<branch>.md` | Add prompts, responses, decisions                                                   |
| Source code files                                     | Full editing as needed                                                              |

### If You Accidentally Modify Protected Files

1. **STOP** - Do not commit
2. **Revert** - Use `git checkout -- <file>` to undo changes
3. **Document** - Note the incident in your feature journal
4. **Alert** - Tell the user before proceeding

### Branch Protection Rules

- **NEVER** commit directly to `main` or `dev` branches
- **ALWAYS** work on feature branches (`feature/*`, `fix/*`, etc.)
- Branch protection is enforced by lefthook pre-commit hooks

## 🤖 Automatic Behaviors

These actions should be performed automatically without waiting to be asked:

1. **After creating branch**: Push immediately with `git push -u origin branch-name`
2. **After pushing new branch**: Create draft PR with `gh pr create --draft --body-file /docs/CLAUDE_WORKSPACE/pr-template.md`
3. **After every commit**: Push automatically with `git push`
4. **After creating/updating PR**: Monitor CI every 30 seconds until all checks pass
5. **When all CI checks pass**: Mark draft PR as ready with `gh pr ready`
6. **When PR is merged**: Perform complete branch cleanup
7. **When starting new feature**: Create feature journal in `journal/features/`
8. **When tests fail**: Commit the current state before fixing
9. **When code is edited**: Commit when any test goes from failing to passing
10. **When PR is ready**: Create workflow optimization analysis

## 🎯 Project-Specific Guidelines

### PR Creation Workflow

1. Create feature branch in worktree
2. **IMMEDIATELY PUSH BRANCH** - Use `git push -u origin branch-name`
3. **CREATE DRAFT PR IMMEDIATELY** - Use `gh pr create --draft --body-file /docs/CLAUDE_WORKSPACE/pr-template.md`
4. Implement with TDD approach
5. Run tests with coverage
6. Commit with detailed message
7. **AUTOMATICALLY PUSH EVERY COMMIT** - Use `git push`
8. Update PR description with each commit using template: `gh pr edit --body-file /docs/CLAUDE_WORKSPACE/pr-template.md`
9. **AUTO-READY PR** - When all tests pass and branch is fully pushed:
   - Check CI status with `gh pr checks <PR-NUMBER>`
   - If all checks pass, mark ready: `gh pr ready <PR-NUMBER>`
10. **MONITOR CI STATUS** - Check PR status every 30 seconds after push:

- Run `gh pr checks <PR-NUMBER>` to see status
- If any checks fail, investigate with `gh run view <RUN-ID> --log-failed`
- Fix issues and push updates immediately
- Continue monitoring until all checks pass

9. **CLEAN UP AFTER MERGE** - When user mentions PR was merged:
   - Switch to dev branch: `git checkout dev`
   - Pull latest changes: `git pull origin dev`
   - Delete local feature branch: `git branch -d feature/branch-name`
   - Delete remote feature branch: `git push origin --delete feature/branch-name`
   - Archive feature journal: Move from `journal/features/` to `journal/archive/YYYY-MM/`
   - Check if worktree needs cleanup: `git worktree list`
10. **OPTIMIZE WORKFLOW** - When PR is ready for review:
    - Create workflow optimization analysis in `workflow-optimization-strategies/`
    - Review feature journal for repetitive tasks
    - Identify automation opportunities (hooks, scripts, aliases)
    - Document time savings and implementation priority

**IMPORTANT**: If the user mentions pushing changes or asks about a PR, ALWAYS create one using `gh pr create --body-file /docs/CLAUDE_WORKSPACE/pr-template.md`. Don't wait to be explicitly asked.

**CRITICAL**: After pushing or updating a PR, you MUST monitor CI status every 30 seconds until all checks pass. Do not assume CI will pass just because local tests pass.

**AUTOMATIC**: When user mentions a PR was merged, immediately perform branch cleanup without being asked.

## ⚠️ Common Pitfalls to Avoid

### Git Operations

- **Never push directly to dev/main** - always use feature branches
- **Check for rebasing needs** - use `git pull --rebase` when branches diverge
- **Avoid duplicate files** - check `git status` before staging
- **Clean up after PR merge** - remove worktrees and delete branches

### Testing

- **Don't skip tests** - even for "simple" documentation changes
- **Check for import errors** - ensure all dependencies are installed
- **Watch for mypy errors** - duplicate files often cause module conflicts
- **Commit after test runs** - maintain complete history

### Documentation

- **Update prompt journal immediately** - don't wait until end of session
- **Include architectural discoveries** - document when reality differs from plan
- **Clarify assumptions** - especially around interfaces (MCP vs CLI)

## 🛠️ QEMU VM Setup Scripts

### Script Usage Guidelines

- Always create shell scripts for command-line tasks in the QEMU_VM.md plan
- Save scripts to `./qemu_setup_scripts/` directory
- Maintain order of successful scripts in `./qemu_setup_scripts/SUCCESSFUL_SCRIPT_ORDER.md`
- Goal: Create a one-shot setup script from the successful script order
- **OS**: Use Linux Mint 22.1 xia for QEMU images (not Ubuntu)

### Last Used Commands

```bash
# Create qemu_setup_scripts directory
mkdir -p /home/vng/dev/chicken-dinner/qemu_setup_scripts
```

## 📝 Commit Message Format

Every commit must follow this template:

```
<type>: <brief description>

Test Results:
- Total: X tests
- Passed: Y tests
- Failed: Z tests

Failed Tests:
- test_name_1: <error message>
- test_name_2: <error message>

Test Changes:
- Added: <list of new test functions/classes added>
- Deleted: <list of test functions/classes removed>
- Modified: <list of test functions/classes changed>

Changes:
- <what was changed and why>
```

### Example Commit

```
fix: update authentication middleware to handle edge cases

Test Results:
- Total: 15 tests
- Passed: 12 tests
- Failed: 3 tests

Failed Tests:
- test_jwt_expired_token: Expected 401, got 500
- test_malformed_token: TypeError: Cannot read property 'id' of undefined
- test_missing_auth_header: AssertionError: Expected error message

Test Changes:
- Added: test_jwt_expired_token, test_malformed_token, test_missing_auth_header
- Deleted: None
- Modified: test_valid_token (added edge case assertions)

Changes:
- Added null checks in token parser
- Fixed error status codes for invalid tokens
- Added try-catch for malformed JWT parsing
```

- # Workflow rules
-
- Always use docs/CLAUDE_WORKSPACE/pr-template.md for the PR template.
- Always fill in as much detail as possible with the PR templates
- Always use test driven development.
- All tests must pass for a feature to be considered complete.
- Features with no tests must be explicitly marked by a human with '@skiptest('some reason here')' in the task plan.
- Don't use a TODO list in memory. use the markdown task list to organize work.
- Always update the plan markdown after tests are run.
