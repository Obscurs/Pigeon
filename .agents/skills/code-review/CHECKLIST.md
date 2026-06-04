# Review Checklists

## Application / Engine Code

### Architecture
- [ ] Code belongs in the right project (PigeonLib vs. SandboxApp)?
- [ ] Engine (`PigeonLib/`) was only modified if explicitly required by the task?
- [ ] New system registered in `SandboxApp/SystemRegister.cpp` in the correct position (writers before readers)?

### Components
- [ ] Pure data struct — no methods, no logic?
- [ ] Has at least one data member (use `bool m_Dummy = true;` for tags)?
- [ ] Only one system writes to this component?

### Systems
- [ ] Does one thing — no hidden second responsibility?
- [ ] No data members except event dispatch queues?
- [ ] Queues are cleared at the end of every `Update()`?

### Events
- [ ] Declared in a dedicated `*Event.h`?
- [ ] Only one system dispatches this event type?
- [ ] All subscribing systems clear their queues every frame?

### Naming & Files
- [ ] Follows `*Component`, `*System`, `*Event` naming?
- [ ] File names match class names?

### General
- [ ] No duplication — checked `EvalHelpers`, `StatHelpers`, and existing systems for overlap?
- [ ] Comments explain only non-obvious WHY — no what-comments?

---

## Test Code

- [ ] One test file per system, named `<SystemFileName>Test.cpp`?
- [ ] File is in the correct `UT/UT_<ModuleName>/` folder?
- [ ] Test file covers only the one system it is named after?
- [ ] Each test case uses `pg::World::Create()` (isolated world)?
- [ ] Only the system under test is registered — no other systems pulled in?
- [ ] Test name format: `"<ModuleName>.<SystemName>::<Description>"`?
- [ ] `UT/CMakeLists.txt` updated if a new module folder was added or removed?
- [ ] Coverage includes: happy path, guard conditions, event handling, edge cases?
