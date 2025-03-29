// event_system.h

#pragma once
#include <functional>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

// Forward declarations
class ISubscription;

//-----------------------------------
// Core Event System Components
//-----------------------------------
class ISubscription {
public:
    virtual ~ISubscription() = default;
};

template <typename... Args>
class Event {
public:
    // inherits from ISubscription to enable the EventsManager to treat all subscriptions polymorphically through the ISubscription interface.
    class Subscription : public ISubscription {
        friend class Event; // Grant Event access to private members
    public:
        // Destructor: Automatically unsubscribes when Subscription is destroyed
        ~Subscription() { if(event_) event_->unsubscribe(id_); }
        // Move constructor (transfer ownership)
        Subscription(Subscription&& other)  : 
            event_(other.event_), id_(other.id_) {
            other.event_ = nullptr; // Invalidate the moved-from object
        }
        
        // Move assignment operator
        Subscription& operator=(Subscription&& other)  {
            if(this != &other) {
                event_ = other.event_;
                id_ = other.id_;
                other.event_ = nullptr;
            }
            return *this;
        }

        // Disable copying (subscriptions are unique ownership)
        Subscription(const Subscription&) = delete;
        Subscription& operator=(const Subscription&) = delete;

    private:
        // Private constructor: Only Event can create Subscriptions
        Subscription(Event* event, int id) : 
            event_(event), id_(id) {}
        
        Event* event_ = nullptr; // Pointer to the associated Event    
        int id_; // Unique subscription ID
    };

    Subscription subscribe(std::function<void(Args...)> callback, const std::string& name) {
        int id = nextId_++;
		//The actual callback function, moved into the entry (to avoid copying)
		//std::move transfers ownership of the callback from the subscribe parameter to the CallbackEntry
		callbacks_.emplace_back(CallbackEntry{ id, std::move(callback), name });
        std::cout << "subscribe " << name.c_str() << ", id " << id << std::endl;
        return Subscription(this, id);
    }

    void trigger(Args... args) {
        for(const auto& entry : callbacks_) {
            entry.callback(args...);
        }
    }

private:
    struct CallbackEntry {
        int id;
        std::function<void(Args...)> callback;
        std::string name; //debug info
    };

    int nextId_ = 0;
    std::vector<CallbackEntry> callbacks_;

    void unsubscribe(int id) {
        auto it = std::find_if(callbacks_.begin(), callbacks_.end(),
            [id](const CallbackEntry& entry) { return entry.id == id; });
        if(it != callbacks_.end()) {
            std::wcout << "unsubscribe " << it->name.c_str() << ", id " << id << std::endl;
            callbacks_.erase(it);
        }
    }
};

class EventsManager {
public:
    template <typename EventType, typename Callback>
	void subscribe(EventType& event, Callback&& cb, std::string info) {
        auto sub = event.subscribe(std::forward<Callback>(cb), info);
        subscriptions_.emplace_back(
            std::make_unique<SubscriptionWrapper<EventType>>(std::move(sub))
        );
    }

private:
    template <typename EventType>
    struct SubscriptionWrapper : public ISubscription {
        typename EventType::Subscription sub;
        SubscriptionWrapper(typename EventType::Subscription&& s) : 
            sub(std::move(s)) {}
    };

    std::vector<std::unique_ptr<ISubscription>> subscriptions_;
};