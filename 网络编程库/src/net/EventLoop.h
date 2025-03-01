#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Timestamp.h"
#include "noncopyable.h"
#include "CurrentThread.h"

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>

class Channel;
class Poller;

class EventLoop:noncopyable{

};

#endif // EVENTLOOP_H