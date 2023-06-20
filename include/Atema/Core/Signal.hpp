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

#ifndef ATEMA_CORE_SIGNAL_HPP
#define ATEMA_CORE_SIGNAL_HPP

#include <Atema/Core/Config.hpp>
#include <Atema/Core/Pointer.hpp>

#include <functional>
#include <shared_mutex>
#include <atomic>

namespace at
{
	class Connection;

	class ATEMA_CORE_API AbstractSignal
	{
	public:
		AbstractSignal();
		AbstractSignal(const AbstractSignal& other) = delete;
		AbstractSignal(AbstractSignal&& other) noexcept = delete;
		virtual ~AbstractSignal();

		virtual void disconnect(const Connection& connection) = 0;

		AbstractSignal& operator=(const AbstractSignal& other) = delete;
		AbstractSignal& operator=(AbstractSignal&& other) noexcept = delete;
	};

	template <typename ... Args>
	class Signal : public AbstractSignal
	{
	public:
		using Callback = std::function<void(Args...)>;

		Signal();
		Signal(const Signal& signal);
		Signal(Signal&& signal) noexcept;
		virtual ~Signal();

		void operator()(Args ... args);

		Connection connect(const Callback& callback);
		template <typename O>
		Connection connect(O& object, void(O::* method)(Args...));
		template <typename O>
		Connection connect(const O& object, void(O::* method)(Args...) const);

		void disconnect(const Connection& connection) override;
		void disconnect(const std::vector<Connection>& connection);

		Signal& operator=(const Signal& signal);
		Signal& operator=(Signal&& signal) noexcept;
		
	private:
		void deletePendingConnections();
		void disconnect(const Connection* connections, size_t size);

		struct SlotData
		{
			Ptr<AbstractSignal*> signal;
			Ptr<size_t> index;
			Callback callback;
		};

		std::shared_mutex m_slotMutex;
		std::vector<Ptr<SlotData>> m_slotDatas;
		std::shared_mutex m_pendingMutex;
		std::vector<Connection> m_pendingConnections;

		std::atomic_bool m_deleteLater;
	};

	class ATEMA_CORE_API Connection final
	{
		template <typename ... Args>
		friend class Signal;

	public:
		Connection() = default;
		Connection(const Connection& other) = default;
		Connection(Connection&& other) noexcept = default;
		~Connection() = default;

		bool isConnected() const noexcept;

		void disconnect();

		Connection& operator=(const Connection& other) = default;
		Connection& operator=(Connection&& other) noexcept = default;

	private:
		Connection(Ptr<AbstractSignal*> signal, Ptr<size_t> index);

		WPtr<AbstractSignal*> m_signal;
		WPtr<size_t> m_index;
	};

	class ATEMA_CORE_API ConnectionGuard
	{
	public:
		ConnectionGuard() = default;
		explicit ConnectionGuard(const Connection& connection);
		explicit ConnectionGuard(Connection&& connection);
		ConnectionGuard(const ConnectionGuard& other) = default;
		ConnectionGuard(ConnectionGuard&& other) noexcept = default;
		~ConnectionGuard();

		template <typename ... SignalArgs, typename ... ConnectionArgs>
		Connection& connect(Signal<SignalArgs...>& signal, ConnectionArgs&&... args);

		Connection& addConnection(const Connection& connection);
		Connection& addConnection(Connection&& connection);

		void disconnect();

		ConnectionGuard& operator=(const ConnectionGuard& other);
		ConnectionGuard& operator=(ConnectionGuard&& other) noexcept;

	private:
		std::vector<Connection> m_connections;
	};
}

#include <Atema/Core/Signal.inl>

#endif
