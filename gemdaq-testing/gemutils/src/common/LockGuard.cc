// copied from tcds/utils/src/common/LockGuard.cc
#include "gem/utils/LockGuard.h"

template <class L>
gem::utils::LockGuard::LockGuard(L& lock) :
  lock_(lock)
{
  lock_.lock();
}

template <class L>
gem::utils::LockGuard::~LockGuard()
{
  lock_.unlock();
}
