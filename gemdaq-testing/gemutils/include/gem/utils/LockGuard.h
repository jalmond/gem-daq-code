// copied from tcds/utils/include/tcds/utils/LockGuard.h
// would like to just include their library, but it's not in our xdaq release
#ifndef gem_utils_LockGuard_h
#define gem_utils_LockGuard_h

namespace gem {
  namespace utils {

    template <class L>
    class LockGuard
    {
    public:
      LockGuard(L& lock);
      ~LockGuard();

    private:
      L& lock_;

      // Prevent copying.
      LockGuard(LockGuard const&);
      LockGuard& operator=(LockGuard const&);
    };

  } // namespace utils
} // namespace gem

template <class L>
gem::utils::LockGuard<L>::LockGuard(L& lock) :
  lock_(lock)
{
  lock_.lock();
}

template <class L>
gem::utils::LockGuard<L>::~LockGuard()
{
  lock_.unlock();
}

#endif // _gem_utils_LockGuard_h_
