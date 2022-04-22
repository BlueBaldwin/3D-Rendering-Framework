#pragma once

#include "Event.h"

// A brief An abstract base class for observers that allows concrete observers to derive from it.

class Observer
{
public:
	// Constructor
	Observer() = default;
	//  Destructor
	virtual ~Observer() = default;
	// Copy, Move, and Assignement operators have been disabled for this class.
	Observer(const Observer&) = delete;
	Observer(const Observer&&) = delete;
	Observer& operator=(const Observer&) = delete;

	// Function to perform a call to the abstract call function to post the @param event to the observer function //
	void exec(Event* e)
	{
		call(e);
	} 
	// Function to get an instance of the object/class the function pointer is bound to //
	virtual void* Instance() = 0;

private:
	// Abstract call function takes an event as a parameter to be implemented in derived classes //
	virtual void call(Event * e) = 0;
};

//	Template class MemberObserver an observer that is a class with a designated observation member function
//	- typename T:				Is a pointer to the class owning the function to be called
//	- typename ConcreteEvent:	Is the type of event that is to be observed
template<typename T, typename ConcreteEvent>
class MemberObserver : public Observer
{
public:
	//! typedefine for the pointer to the class member function/
	typedef void (T::* MemberFunction) (ConcreteEvent*);			// creating a type define for the function pointer that we will store in this class
	/*! Constructor */
	MemberObserver(T* a_instance, MemberFunction a_function)
		: m_instance(a_instance), m_memberFunction(a_function)
	{}
	/*! Destructor */
	~MemberObserver() { m_instance = nullptr; }

	/*! Function to return a pointer to the instance of the class the Member function belongs to */
	void* Instance() { return (void*)m_instance; }

private:
	/*! Implementation of abstract base class function for calling observer function */
	void call(Event* e)
	{
		//cast event to correct type
		(m_instance->*m_memberFunction)(static_cast<ConcreteEvent*>(e));
	}
private:
	// the member function in the class that we hold a pointer to
	MemberFunction m_memberFunction;
	// the class instance to call the function on
	T* m_instance;
};

// template class Global Observer, template argument ConcreteEvent is deduced by the compiler
template<typename ConcreteEvent>
class GlobalObserver : public Observer
{
public:
	// typedef of function pointer to non-member function that takes an concrete event as a parameter 
	typedef void (*Function)(ConcreteEvent*);
	// Constructor sets function pointer member to point to parameter function
	GlobalObserver(Function a_function)
		: m_function(a_function)
	{}
	/// Destructor
	~GlobalObserver() {}
	// Instance function implementation, global functions have no instance so returns nullptr
	void* Instance() { return nullptr; }

private:
	// Call function will call global function member variable with Event parameter
	void call(Event* e)
	{
		/*!cast event to correct type*/
		(*m_function) (static_cast<ConcreteEvent*>(e));
	}

private:
	// member variable pointer to global/static function
	Function m_function;
};



