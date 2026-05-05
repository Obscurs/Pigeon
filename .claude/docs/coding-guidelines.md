# Coding Guidelines

## Language and Style

- C++17. No exceptions in hot paths.
- Prefer `entt::registry` views for system queries.
- Avoid raw owning pointers. Use `const` wherever applicable. Avoid unnecessary copies of entt components.

## Naming Conventions

| Type | Convention | Example |
|---|---|---|
| Component | `*Component` | `StatsComponent`, `CurrentActionComponent` |
| System | `*System` | `CandidateScoreSystem`, `StatsUpdateSystem` |
| Event | `*Event` | `RequestActionEvent`, `ClockChangedEvent` |
| File | matches class name | `StatsUpdateSystem.cpp` / `.h` |

## Comments

Only add a comment when the **WHY** is non-obvious — a hidden constraint, a subtle invariant, a workaround for a specific bug. Do not describe what the code does; well-named identifiers already do that.

---

## Components

Plain structs only — no methods, no logic. Constructors are allowed.

One system owns writes to a component; any system may read it. The system that attaches a component to an entity may differ from the system that later modifies its values — but each responsibility belongs to exactly one system.

**A component must always have at least one data member.** EnTT does not support empty types as components and they will not compile. If a component is purely a tag (carries no data), add `bool m_Dummy = true;` as a placeholder.

```cpp
// Good — pure data, no logic
struct PausedStateComponent
{
    PausedStateComponent() = default;
    PausedStateComponent(PausedStateComponent&&) = default;
    PausedStateComponent& operator=(PausedStateComponent&&) = default;

    entt::entity m_LoadButtonEntity  = entt::null;
    entt::entity m_SaveButtonEntity  = entt::null;
    entt::entity m_PlayButtonEntity  = entt::null;
    entt::entity m_ExitButtonEntity  = entt::null;
};

// Tag component — no meaningful data, so use m_Dummy to satisfy the compiler
struct GameLoadedComponent
{
    bool m_Dummy = true;
};
```

---

## Events

Declare in a dedicated header (`*Event.h`). Dispatch via `entt::dispatcher`. Only **one** system may dispatch a specific event type. Every subscribing system must process and **clear its queue every frame**.

Events follow the same rule as tag components: they must have at least one member. Use `bool m_Dummy = true;` when the event carries no payload.

```cpp
// GameLoadRequestEvent.h — no payload needed, m_Dummy satisfies the compiler
struct GameLoadRequestEvent
{
    bool m_Dummy = true;
};
```

Subscribe in the system constructor:

```cpp
pig::World::GetDispatcher()
    .sink<ds::state::GameLoadRequestEvent>()
    .connect<&GameStateChangeSystem::QueueLoadGameEvent>(*this);
```

---

## Systems

A system reads components and events as input, and writes to components and/or events as output.

### Hard rules

1. **Single responsibility.** A system does one thing. If it needs to do more, split it into additional systems.
2. **No data members except event queues.** The only allowed member variables are the vectors/lists used to batch incoming events. No other fields, no helper methods on the class.
3. **One writer per component.** Only one system modifies a given component type.
4. **One dispatcher per event.** Only one system dispatches a given event type.
5. **Clear queues every frame.** Every system must clear every event queue it holds at the end of `Update()` — even if nothing was processed.

### System header — allowed shape

```cpp
// GameStateChangeSystem.h
class GameStateChangeSystem : public pig::System
{
public:
    GameStateChangeSystem();
    ~GameStateChangeSystem() = default;
    void Update(const pig::Timestep& ts) override;

    // Public queue methods — one per subscribed event type
    void QueueLoadGameEvent(const GameLoadRequestEvent& event) { m_LoadEvents.push_back(event); }
    void QueueSaveGameEvent(const GameSaveRequestEvent& event) { m_SaveEvents.push_back(event); }
    void QueueRunGameEvent (const GameRunRequestEvent&  event) { m_RunEvents.push_back(event);  }
    void QueuePauseGameEvent(const GamePauseRequestEvent& event) { m_PauseEvents.push_back(event); }

private:
    // ONLY allowed members: event queues
    std::vector<GameLoadRequestEvent>  m_LoadEvents;
    std::vector<GameSaveRequestEvent>  m_SaveEvents;
    std::vector<GameRunRequestEvent>   m_RunEvents;
    std::vector<GamePauseRequestEvent> m_PauseEvents;
};
```

### System implementation — required queue-clearing pattern

```cpp
void GameStateChangeSystem::Update(const pig::Timestep& /*ts*/)
{
    entt::registry& registry = pig::World::GetRegistry();

    for (const GameLoadRequestEvent& event : m_LoadEvents)
    {
        // handle event...
    }
    for (const GameRunRequestEvent& event : m_RunEvents)
    {
        // handle event...
    }

    // Always clear at the end of Update — never skip this
    m_LoadEvents.clear();
    m_RunEvents.clear();
    m_SaveEvents.clear();
    m_PauseEvents.clear();
}
```

---

## New System Checklist

Before implementing a new system:

1. Read `Documentation/<ModuleName>.info` — understand what this system does and how it relates to others.
2. Inherit from `pig::System` and implement `Update(const pig::Timestep& ts)`.
3. Implement required components (pure data structs, no logic, at least one member each).
