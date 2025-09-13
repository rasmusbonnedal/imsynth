#pragma once

#include <memory>

class AudioEngine {
   public:
    virtual ~AudioEngine() {}
    virtual int init() = 0;
    virtual float* freq() = 0;
    static std::unique_ptr<AudioEngine> create();

   private:
};
