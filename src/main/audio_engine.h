#pragma once

#include "audio_graph.h"

#include <memory>

class AudioEngine {
   public:
    virtual ~AudioEngine() {}
    virtual int init() = 0;
    virtual void setGraph(AuNodeGraphPtr graph) = 0;
    virtual AuNodeGraphPtr getGraph() = 0;
    virtual float getDb() const = 0;
    virtual const std::vector<float>& getHistory() const = 0;
    virtual size_t getHistoryPos() const = 0;

    static std::unique_ptr<AudioEngine> create();

   private:
};
