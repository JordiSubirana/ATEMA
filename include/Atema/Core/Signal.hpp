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
#include <Atema/Core/NonCopyable.hpp>

#include <functional>

namespace at
{
	class Connection;

	class ATEMA_CORE_API AbstractSignal
	{
	public:
		AbstractSignal();
		virtual ~AbstractSignal();

		virtual void disconnect(const Connection& connection) = 0;
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

		Signal& operator=(const Signal& signal);
		Signal& operator=(Signal&& signal) noexcept;
		
	private:
		void deletePendingConnections();

		struct SlotData
		{
			Ptr<AbstractSignal*> signal;
			Ptr<size_t> index;
			Callback callback;
		};

		std::vector<Ptr<SlotData>> m_slotDatas;
		std::vector<Connection> m_pendingConnections;
		bool m_deleteLater;
	};

	class ATEMA_CORE_API Connection final
	{
		template <typename ... Args>
		friend class Signal;

	public:
		Connection();
		Connection(const Connection& connection) = default;
		Connection(Connection&& connection) noexcept;
		~Connection();

		bool isConnected() const noexcept;

		void disconnect();

	private:
		Connection(Ptr<AbstractSignal*> signal, Ptr<size_t> index);

		WPtr<AbstractSignal*> m_signal;
		WPtr<size_t> m_index;
	};

	class ATEMA_CORE_API ConnectionGuard : public NonCopyable
	{
	public:
		ConnectionGuard();
		ConnectionGuard(const Connection& connection);
		ConnectionGuard(Connection&& connection);
		ConnectionGuard(ConnectionGuard&& connectionGuard) noexcept;
		~ConnectionGuard();

		template <typename ... SignalArgs, typename ... ConnectionArgs>
		void connect(Signal<SignalArgs...>& signal, ConnectionArgs&&... args);

		void addConnection(const Connection& connection);
		void addConnection(Connection&& connection);

		void disconnect();

		ConnectionGuard& operator=(const Connection& connection);
		ConnectionGuard& operator=(Connection&& connection);
		ConnectionGuard& operator=(ConnectionGuard&& connectionGuard) noexcept;

	private:
		std::vector<Connection> m_connections;
	};
}

#include <Atema/Core/Signal.inl>

#endif
