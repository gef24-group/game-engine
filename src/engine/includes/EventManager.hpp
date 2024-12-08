#pragma once

#include "EventHandler.hpp"
#include "Types.hpp"
#include <mutex>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ComparePriority {
    bool operator()(const Event &event_1, const Event &event_2) {
        if (event_1.timestamp == event_2.timestamp) {
            return event_1.priority > event_2.priority;
        }
        return event_1.timestamp > event_2.timestamp;
    }
};

class EventManager {
  public:
    static EventManager &GetInstance() {
        static EventManager instance;
        return instance;
    }

  private:
    EventManager() {}

  public:
    EventManager(EventManager const &) = delete;
    void operator=(EventManager const &) = delete;

  public:
    void Register(std::vector<EventType> event_types, EventHandler *handler);
    void Deregister(std::vector<EventType> event_types, EventHandler *handler);
    void ProcessEvents();

    void ProfileEventQueue();
    int64_t GetLastEventTimestamp();

    void RaiseInputEvent(InputEvent event);
    void RaiseCollisionEvent(CollisionEvent event);
    void RaiseDeathEvent(DeathEvent event);
    void RaiseSpawnEvent(SpawnEvent event);
    void RaiseMoveEvent(MoveEvent event, bool ignore_change = false);
    void RaiseSendUpdateEvent(SendUpdateEvent event);
    void RaiseJoinEvent(JoinEvent event);
    void RaiseDiscoverEvent(DiscoverEvent event);
    void RaiseLeaveEvent(LeaveEvent event);
    void RaiseStartRecordEvent(StartRecordEvent event);
    void RaiseStopRecordEvent(StopRecordEvent event);
    void RaiseStartReplayEvent(StartReplayEvent event);
    void RaiseStopReplayEvent(StopReplayEvent event);

    void Raise(Event event);

  private:
    std::mutex event_queue_mutex;
    std::mutex handlers_mutex;
    std::unordered_map<EventType, std::unordered_set<EventHandler *>> handlers;
    std::priority_queue<Event, std::vector<Event>, ComparePriority> event_queue;

    std::priority_queue<Event, std::vector<Event>, ComparePriority> GetEventQueue();
    std::unordered_map<EventType, std::unordered_set<EventHandler *>> GetHandlers();
    void HandleEvent(Event event);
    void HandleReplayedEvent(Event event);
    void PushEventQueue(Event event);
    void PopEventQueue();
    bool IsDeathOrSpawnInQueue();
};