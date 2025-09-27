#pragma once

#include "audio_graph.h"

#include <memory>

class AudioEngine {
   public:
    virtual ~AudioEngine() {}
    virtual int init() = 0;
    virtual void setGraph(AuNodeGraphPtr graph) = 0;
    virtual float* amp() = 0;
    static std::unique_ptr<AudioEngine> create();

   private:
};
