/*
	Copyright 2022 Jordi SUBIRANA

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

#include <Atema/Core/Signal.hpp>

namespace at
{
	// Signal
	template <typename ... Args>
	Signal<Args...>::Signal() :
		AbstractSignal(),
		m_deleteLater(false)
	{
	}

	template <typename ... Args>
	Signal<Args...>::Signal(const Signal& signal) :
		AbstractSignal(),
		m_deleteLater(false)
	{
	}

	template <typename ... Args>
	Signal<Args...>::Signal(Signal&& signal) noexcept
	{
		operator=(std::move(signal));
	}

	template <typename ... Args>
	Signal<Args...>::~Signal()
	{
	}

	template <typename ... Args>
	void Signal<Args...>::operator()(Args... args)
	{
		m_deleteLater = true;

		for (auto& slotData : m_slotDatas)
			slotData->callback(std::forward<Args>(args)...);

		m_deleteLater = false;

		deletePendingConnections();
	}

	template <typename ... Args>
	Connection Signal<Args...>::connect(const Callback& callback)
	{
		auto slotData = std::make_shared<SlotData>();
		slotData->signal = std::make_shared<AbstractSignal*>(this);
		slotData->index = std::make_shared<size_t>(m_slotDatas.size());
		slotData->callback = callback;

		m_slotDatas.emplace_back(slotData);

		return Connection(slotData->signal, slotData->index);
	}

	template <typename ... Args>
	template <typename O>
	Connection Signal<Args...>::connect(O& object, void(O::* method)(Args...))
	{
		return connect([&object, method](Args... args)
			{
				object.method(std::forward<Args>(args)...);
			});
	}

	template <typename ... Args>
	template <typename O>
	Connection Signal<Args...>::connect(const O& object, void(O::* method)(Args...) const)
	{
		return connect([&object, method](Args... args)
			{
				object.method(std::forward<Args>(args)...);
			});
	}

	template <typename ... Args>
	void Signal<Args...>::disconnect(const Connection& connection)
	{
		if (!connection.isConnected())
			return;

		if (m_deleteLater)
		{
			m_pendingConnections.emplace_back(connection);
		}
		else
		{
			size_t index = *connection.m_index.lock();

			if (index == 0 && m_slotDatas.size() == 1)
			{
				m_slotDatas.clear();
			}
			else if (index == m_slotDatas.size() - 1)
			{
				m_slotDatas.resize(m_slotDatas.size() - 1);
			}
			else
			{
				auto& lastSlot = m_slotDatas.back();
				*(lastSlot->index) = index;

				lastSlot.swap(m_slotDatas[index]);

				m_slotDatas.resize(m_slotDatas.size() - 1);
			}
		}
	}

	template <typename ... Args>
	Signal<Args...>& Signal<Args...>::operator=(const Signal& signal)
	{
		return *this;
	}

	template <typename ... Args>
	Signal<Args...>& Signal<Args...>::operator=(Signal&& signal) noexcept
	{
		m_slotDatas = std::move(signal.m_slotDatas);

		for (auto& slotData : m_slotDatas)
			*(slotData->signal) = this;

		m_pendingConnections = std::move(signal.m_pendingConnections);

		m_deleteLater = m_deleteLater;

		return *this;
	}

	template <typename ... Args>
	void Signal<Args...>::deletePendingConnections()
	{
		if (m_deleteLater)
			return;

		for (auto& connection : m_pendingConnections)
			disconnect(connection);

		m_pendingConnections.clear();
	}

	// ConnectionGuard
	template <typename ... SignalArgs, typename ... ConnectionArgs>
	void ConnectionGuard::connect(Signal<SignalArgs...>& signal, ConnectionArgs&&... args)
	{
		addConnection(signal.connect(std::forward<ConnectionArgs>(args)...));
	}
}

#endif
