// copied from tcds/utils/include/tcds/utils/Lock.h
// would like to just include their library, but it's not in our xdaq release
#ifndef gem_utils_Lock_h
#define gem_utils_Lock_h

#include "toolbox/BSem.h"

namespace gem {
  namespace utils {

    class Lock
    {
    public:
      Lock(toolbox::BSem::State state=toolbox::BSem::EMPTY, bool recursive=true);
      ~Lock();

      void lock();
      void unlock();

    private:
      toolbox::BSem semaphore_;

      // Prevent copying.
      Lock(Lock const&);
      Lock& operator=(Lock const&);
    };

  } // namespace utils
} // namespace gem

#endif // _gem_utils_Lock_h_
