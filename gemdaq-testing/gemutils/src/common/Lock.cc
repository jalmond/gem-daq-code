// copied from tcds/utils/src/common/Lock.cc
#include "gem/utils/Lock.h"

gem::utils::Lock::Lock(toolbox::BSem::State state, bool recursive) :
  semaphore_(state, recursive)
{

}

gem::utils::Lock::~Lock()
{
  unlock();
}

void
gem::utils::Lock::lock()
{
  semaphore_.take();
}

void
gem::utils::Lock::unlock()
{
  semaphore_.give();
}
