#pragma once

namespace widget {

class Widget {
   public:
    virtual ~Widget() = default;

    virtual void compose() = 0;

   private:
};

}  // namespace widget
