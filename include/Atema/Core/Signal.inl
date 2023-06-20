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
		{
			std::shared_lock readLock(m_slotMutex);

			m_deleteLater = true;

			for (auto& slotData : m_slotDatas)
				slotData->callback(std::forward<Args>(args)...);

			m_deleteLater = false;
		}

		deletePendingConnections();
	}

	template <typename ... Args>
	Connection Signal<Args...>::connect(const Callback& callback)
	{
		std::unique_lock writeLock(m_slotMutex);
		
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
		disconnect(&connection, 1);
	}

	template <typename ... Args>
	void Signal<Args...>::disconnect(const std::vector<Connection>& connection)
	{
		disconnect(connection.data(), connection.size());
	}

	template <typename ... Args>
	Signal<Args...>& Signal<Args...>::operator=(const Signal& signal)
	{
		return *this;
	}

	template <typename ... Args>
	Signal<Args...>& Signal<Args...>::operator=(Signal&& signal) noexcept
	{
		std::unique_lock writeLock(m_slotMutex);
		std::unique_lock otherWriteLock(signal.m_slotMutex);

		m_slotDatas = std::move(signal.m_slotDatas);

		for (auto& slotData : m_slotDatas)
			*(slotData->signal) = this;

		m_pendingConnections = std::move(signal.m_pendingConnections);

		m_deleteLater = signal.m_deleteLater;

		return *this;
	}

	template <typename ... Args>
	void Signal<Args...>::deletePendingConnections()
	{
		if (m_deleteLater)
			return;

		bool clearPending = false;

		{
			std::shared_lock readLock(m_pendingMutex);

			clearPending = !m_pendingConnections.empty();
		}

		if (clearPending)
		{
			std::vector<Connection> pendingConnections;
			
			{
				std::unique_lock writeLock(m_pendingMutex);

				std::swap(pendingConnections, m_pendingConnections);
			}
			
			disconnect(pendingConnections);
		}
	}

	template<typename ...Args>
	void Signal<Args...>::disconnect(const Connection* connections, size_t size)
	{
		if (m_deleteLater)
		{
			std::unique_lock writeLock(m_pendingMutex);

			for (size_t i = 0; i < size; i++)
			{
				const auto& connection = connections[i];

				if (!connection.isConnected())
					continue;

				m_pendingConnections.emplace_back(connection);
			}
		}
		else
		{
			std::unique_lock writeLock(m_slotMutex);
			
			for (size_t i = 0; i < size; i++)
			{
				const auto& connection = connections[i];

				if (!connection.isConnected())
					continue;

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
	}

	// ConnectionGuard
	template <typename ... SignalArgs, typename ... ConnectionArgs>
	Connection& ConnectionGuard::connect(Signal<SignalArgs...>& signal, ConnectionArgs&&... args)
	{
		return addConnection(signal.connect(std::forward<ConnectionArgs>(args)...));
	}
}

#endif
