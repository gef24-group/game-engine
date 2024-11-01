
#include "Types.hpp"

class Event {
  public:
    EventType type;
    EventData data;
    int64_t timestamp;
    Priority priority;

    Event(EventType type, EventData data);

    void SetDelay(int64_t delay);
    void SetPriority(Priority priority);
};