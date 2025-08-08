#pragma once

namespace style {

inline bool RightToLeft() { return false; }
inline int Scale() { return 1; }
inline int CheckScale(int scale) { return scale; }
constexpr int kScaleAuto = 0;

} // namespace style
