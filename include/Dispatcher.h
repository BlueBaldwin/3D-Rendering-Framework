#pragma once

#include <map>
#include <list>
#include <functional>
#include <typeinfo>
#include <typeindex>

#include "Observer.h"
#include "Event.h"

// Create a typedefine for std::list<observer*> objects to improve code readability
typedef std::list<Observer*> ObserverList;

class Dispatcher
{
public:
	static Dispatcher* GetInstance() { return m_instance; }
	static Dispatcher* CreateInstance()		
	{
		if (m_instance == nullptr)		// If we dont have an instance then create one and return it
		{
			m_instance = new Dispatcher();
		}
		return m_instance;				// Otherwise just return it
	}
	static void DestroyInstance()
	{
		if (m_instance)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}
#pragma region Subscription
	// Subscription function to subscribe observers to an event, member function pointer implementation
	template< typename T, typename ConcreteEvent>
	void Subscribe(T* a_instance, void(T::* memberFunction)(ConcreteEvent*))
	{
		// Get a list of observers from the subscribers map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		// If observers is null there are no observers for this event yet
		if (observers == nullptr)
		{
			// Create new list for event type and add this into the subscribers map
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
			//! Push a new member observer into the observers list from the subscribers map
			observers->push_back(new MemberObserver<T, ConcreteEvent>(a_instance, memberFunction));
		}
	}
	// Subscribe method for global functions to become event subscribers
	template<typename ConcreteEvent>
	void Subscribe(void(*Function)(ConcreteEvent*))
	{
		// Get a list of observers from the subscribers map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr)
		{
			//! create new list for event type and add this into the subscribers map
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		// Push a new member observer into the observers list from the subscribers map
		observers->push_back(new GlobalObserver<ConcreteEvent>(Function));
	}
#pragma endregion Subscription

#pragma region Publishing
	// Function to publish an event and notify any subscribers...
	// It will find the event type in the dictionary and then iterate through the 
	// list of observers and call them to handle the event. If one of the observers marks 
	// the event as handled then no other observers in the list will receive the event
	template <typename ConcreteEvent>
	void Publish(ConcreteEvent* e, bool cleanup = false)
	{
		// Get the list of observers from the map
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr) { return; }
		//! for each observer notify them that the event has occured
		for (auto& handler : *observers)
		{
			handler->exec(e);
			// If an event has been handled by a subscriber then we do not need to keep notifying other subscribers
			if (static_cast<Event*>(e)->Ishandled())
			{
				break;
			}
		}
		// as we could pass through "new ConcreteEvent(" we should call delete if needed
		if (cleanup) { delete e; }
	}
#pragma endregion Publishing

protected:
	// Keep the constructors protected and use this dispatcher class as a singleton object
	Dispatcher() {};
	~Dispatcher()
	{
		// Better clean up the subscriber map
		// We need to go through our subscriber dictionary list it contains and delete each item 
		for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
		{
			ObserverList* obs = it->second;
			for (auto o = obs->begin(); o != obs->end(); ++o)
			{
				delete (*o);
				(*o) = nullptr;
			}
			delete obs;
		}
	};

private:
	static Dispatcher* m_instance;
	// A hash map of observers uses typeid of Event class as an index into the map.
	std::map<std::type_index, ObserverList*> m_subscribers;
};
