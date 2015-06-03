#ifndef gem_base_GEMMonitor_h
#define gem_base_GEMMonitor_h

#include <string>
#include <tr1/unordered_map>
#include <utility>
#include <vector>

#include "log4cplus/logger.h"

#include "toolbox/task/TimerListener.h"
#include "toolbox/task/TimerEvent.h"
#include "toolbox/TimeVal.h"

namespace toolbox {
  namespace task {
    class Timer;
  }
}

namespace xdaq {
  class Application;
}

namespace gem {
  namespace base {
    
    class GEMMonitor : public toolbox::task::TimerListener
      {
	
      };
  }
}
#endif
