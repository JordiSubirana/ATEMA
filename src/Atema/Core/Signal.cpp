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

#include <Atema/Core/Signal.hpp>

using namespace at;

// AbstractSignal
AbstractSignal::AbstractSignal()
{
}

AbstractSignal::~AbstractSignal()
{
}

// Connection
Connection::Connection()
{
}

Connection::Connection(Ptr<AbstractSignal*> signal, Ptr<size_t> index) :
	m_signal(signal),
	m_index(index)
{
}

Connection::Connection(Connection&& connection) noexcept :
	m_signal(std::move(connection.m_signal)),
	m_index(std::move(connection.m_index))
{
}

Connection::~Connection()
{
}

bool Connection::isConnected() const noexcept
{
	return !m_signal.expired();
}

void Connection::disconnect()
{
	if (isConnected())
	{
		(*m_signal.lock())->disconnect(*this);
	}
}

// ConnectionGuard
ConnectionGuard::ConnectionGuard()
{
}

ConnectionGuard::ConnectionGuard(const Connection& connection)
{
	addConnection(connection);
}

ConnectionGuard::ConnectionGuard(Connection&& connection)
{
	addConnection(std::move(connection));
}

ConnectionGuard::ConnectionGuard(ConnectionGuard&& connectionGuard) noexcept
{
	operator=(std::move(connectionGuard));
}

ConnectionGuard::~ConnectionGuard()
{
	disconnect();
}

void ConnectionGuard::addConnection(const Connection& connection)
{
	m_connections.emplace_back(connection);
}

void ConnectionGuard::addConnection(Connection&& connection)
{
	m_connections.emplace_back(std::move(connection));
}

void ConnectionGuard::disconnect()
{
	for (auto& connection : m_connections)
		connection.disconnect();

	m_connections.clear();
}

ConnectionGuard& ConnectionGuard::operator=(const Connection& connection)
{
	disconnect();

	addConnection(connection);

	return *this;
}

ConnectionGuard& ConnectionGuard::operator=(Connection&& connection)
{
	disconnect();

	addConnection(std::move(connection));

	return *this;
}

ConnectionGuard& ConnectionGuard::operator=(ConnectionGuard&& connectionGuard) noexcept
{
	if (&connectionGuard != this)
	{
		disconnect();
		m_connections = std::move(connectionGuard.m_connections);
	}

	return *this;
}
