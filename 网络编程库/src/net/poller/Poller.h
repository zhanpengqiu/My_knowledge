#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"
#include "Channel.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Poller:noncopyable{
public:
    using ChannelList=std::vector<Channel*>;
};


#endif // POLLER_H