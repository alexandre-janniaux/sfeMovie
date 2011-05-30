
#include "Condition.h"
#include <SFML/Config.hpp>

#ifdef SFML_WINDOWS
#include "Win32/ConditionImpl.h"
#else
#include "Unix/ConditionImpl.h"
#endif


Condition::Condition(int& var) :
m_impl(NULL)
{
	m_impl = new ConditionImpl(var);
}

Condition::~Condition(void)
{
	delete m_impl;
}

void Condition::waitAndRetain(int value)
{
	m_impl->waitAndRetain(value);
}

void Condition::release(void)
{
	m_impl->release();
}

void Condition::signal(void)
{
	m_impl->signal();
}