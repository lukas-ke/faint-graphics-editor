
// Helper to make static_assert:s dependent on T.
template<typename T>
struct TypeDependentFalse{
  static const bool value = false;
};
