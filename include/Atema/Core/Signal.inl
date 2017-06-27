/*
	Copyright 2017 Jordi SUBIRANA

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
	Software, and to permit persons to whom the Software is furnished to do so, subject
	to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ATEMA_CORE_SIGNAL_INL
#define ATEMA_CORE_SIGNAL_INL

#include <Atema/Core/Config.hpp>
#include <Atema/Core/Signal.hpp>

#include <Atema/Core/Object.hpp>

namespace at
{
	// ABSTRACT SIGNAL
	namespace detail
	{
		template <typename ... Args>
		AbstractSignal<Args...>::AbstractSignal()
		{
			m_slots = std::make_shared<std::vector<Slot*>>();
		}

		template <typename ... Args>
		AbstractSignal<Args...>::~AbstractSignal()
		{
		}

		template <typename ... Args>
		template <typename O, typename R>
		Connection AbstractSignal<Args...>::doConnectEmpty(O* object, R(O::*method)())
		{
			auto s = new ClassEmptySlot<O, R>();
			s->object = object;
			s->method = method;

			auto c = doConnect(s);

			if (std::is_base_of<Object, O>::value)
				static_cast<Object*>(object)->m_connections.push_back(c);

			return c;
		}

		template <typename ... Args>
		template <typename O, typename R>
		Connection AbstractSignal<Args...>::doConnect(O* object, R(O::*method)(Args...))
		{
			auto s = new ClassSlot<O, R>();
			s->object = object;
			s->method = method;

			auto c = doConnect(s);

			if (std::is_base_of<Object, O>::value)
				static_cast<Object*>(object)->m_connections.push_back(c);

			return c;
		}

		template <typename ... Args>
		template <typename R>
		Connection AbstractSignal<Args...>::doConnectEmpty(R(*function)())
		{
			auto s = new FunctionEmptySlot<R>();
			s->function = function;

			return doConnect(s);
		}

		template <typename ... Args>
		template <typename R>
		Connection AbstractSignal<Args...>::doConnect(R(*function)(Args...))
		{
			auto s = new FunctionSlot<R>();
			s->function = function;

			return doConnect(s);
		}

		template <typename ... Args>
		template <typename R>
		Connection AbstractSignal<Args...>::doConnectEmpty(std::function<R()>& function)
		{
			auto s = new FunctionEmptySlot<R>();
			s->function = function;

			return doConnect(s);
		}

		template <typename ... Args>
		template <typename R>
		Connection AbstractSignal<Args...>::doConnect(std::function<R(Args...)>& function)
		{
			auto s = new FunctionSlot<R>();
			s->function = function;

			return doConnect(s);
		}

		template <typename ... Args>
		void AbstractSignal<Args...>::operator()(Args...args)
		{
			std::vector<Slot*>& slotList = *m_slots;

			for (auto s : slotList)
				s->operator()(args...);
		}

		template <typename ... Args>
		Connection AbstractSignal<Args...>::doConnect(Slot* s)
		{
			m_slots->push_back(s);

			Connection c;
			Holder *h = new Holder(m_slots, s);

			c.m_holder.reset(h);

			return c;
		}
	}

	// SIGNAL
	template <typename ... Args>
	template <typename O, typename R>
	Connection Signal<Args...>::connect(O* object, R(O::* method)())
	{
		return doConnectEmpty(object, method);
	}

	template <typename ... Args>
	template <typename O, typename R>
	Connection Signal<Args...>::connect(O* object, R(O::* method)(Args...))
	{
		return doConnect(object, method);
	}

	template <typename ... Args>
	template <typename O, typename R>
	Connection Signal<Args...>::connect(O& object, R( O::* method)())
	{
		return doConnectEmpty(&object, method);
	}

	template <typename ... Args>
	template <typename O, typename R>
	Connection Signal<Args...>::connect(O& object, R( O::* method)(Args...))
	{
		return doConnect(&object, method);
	}

	template <typename ... Args>
	template <typename R>
	Connection Signal<Args...>::connect(R(*function)())
	{
		return doConnectEmpty(function);
	}

	template <typename ... Args>
	template <typename R>
	Connection Signal<Args...>::connect(R(*function)(Args...))
	{
		return doConnect(function);
	}

	template <typename ... Args>
	template <typename R>
	Connection Signal<Args...>::connect(std::function<R()>& function)
	{
		return doConnectEmpty(function);
	}

	template <typename ... Args>
	template <typename R>
	Connection Signal<Args...>::connect(std::function<R(Args...)>& function)
	{
		return doConnect(function);
	}

	// SIGNAL (specialized for empty Args)
	template <typename O, typename R>
	Connection Signal<>::connect(O* object, R( O::* method)())
	{
		return doConnect(object, method);
	}

	template <typename R>
	Connection Signal<>::connect(R(* function)())
	{
		return doConnect(function);
	}

	template <typename R>
	Connection Signal<>::connect(std::function<R()>& function)
	{
		return doConnect(function);
	}
}

#endif