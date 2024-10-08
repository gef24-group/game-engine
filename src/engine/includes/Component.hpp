
#include <string>

class Component {
  private:
    std::string type;

  public:
    virtual ~Component() = default;
    virtual void Update() = 0;
};