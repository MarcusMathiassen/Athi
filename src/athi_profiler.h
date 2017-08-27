#pragma once

#ifdef DEBUG
#define PROFILE_TIME(x) x
#endif

struct Frame {
  int num;
};

class Profiler {
 private:
  std::vector<Frame> frames;

 public:
};