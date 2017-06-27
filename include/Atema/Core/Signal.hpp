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

#ifndef ATEMA_CORE_SIGNAL_HPP
#define ATEMA_CORE_SIGNAL_HPP

#include <Atema/Core/Config.hpp>
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>

namespace at
{
	namespace detail
	{
		template <typename...Args>
		class AbstractSignal;
	}

	class ATEMA_CORE_API Connection
	{
		template <typename...Args>
		friend class detail::AbstractSignal;

	public:
		virtual ~Connection();

		void disconnect();

		bool isConnected() const noexcept;

	private:
		Connection() = default;

		struct Holder
		{
			Holder() : isConnected(true) {}
			
			virtual ~Holder() = default;

			virtual void disconnect() = 0;

			bool isConnected;
		};

		std::shared_ptr<Holder> m_holder;
	};

	namespace detail
	{
		template <typename...Args>
		class AbstractSignal
		{
			friend class Connection;

		public:
			AbstractSignal();
			virtual ~AbstractSignal();

			void operator()(Args...);

		protected:
			template <typename O, typename R>
			Connection doConnectEmpty(O *object, R(O::*method)());
			template <typename O, typename R>
			Connection doConnect(O *object, R(O::*method)(Args...));
			template <typename R>
			Connection doConnectEmpty(R(*function)());
			template <typename R>
			Connection doConnect(R(*function)(Args...));
			template <typename R>
			Connection doConnectEmpty(std::function<R()>& function);
			template <typename R>
			Connection doConnect(std::function<R(Args...)>& function);

		private:
			struct Slot;

			Connection doConnect(Slot *slot);

			struct Slot
			{
				Slot() = default;
				virtual ~Slot() = default;

				virtual void operator()(Args...) = 0;
			};

			struct Holder : Connection::Holder
			{
				Holder(const std::shared_ptr<std::vector<Slot*>>& slotList, Slot *slot) :
					m_slots(slotList), m_slot(slot)
				{

				}

				~Holder() override
				{
					Holder::disconnect();
				}

				void disconnect() override
				{
					if (!m_slots.expired())
					{
						std::shared_ptr<std::vector<Slot*>> s = m_slots.lock();

						auto it = std::find(s->begin(), s->end(), m_slot);

						if (it != s->end())
						{
							s->erase(it);
						}

						isConnected = false;
					}
				}

				std::weak_ptr<std::vector<Slot*>> m_slots;

				Slot *m_slot;
			};

			template <typename O, typename R>
			struct ClassSlot : Slot
			{
				O *object;
				R(O::*method)(Args...);

				void operator()(Args...args) override
				{
					(object->*method)(args...);
				}
			};

			template <typename O, typename R>
			struct ClassEmptySlot : Slot
			{
				O *object;
				R(O::*method)();

				void operator()(Args...) override
				{
					(object->*method)();
				}
			};

			template <typename R>
			struct FunctionSlot : Slot
			{
				std::function<R(Args...)> function;

				void operator()(Args...args) override
				{
					function(args...);
				}
			};

			template <typename R>
			struct FunctionEmptySlot : Slot
			{
				std::function<R()> function;

				void operator()(Args...args) override
				{
					function();
				}
			};

			std::shared_ptr<std::vector<Slot*>> m_slots;
		};

		template <>
		class AbstractSignal<void> : public AbstractSignal<>
		{
			
		};
	}

	template <typename...Args>
	class Signal : public detail::AbstractSignal<Args...>
	{
	public:
		Signal() = default;
		virtual ~Signal() = default;

		template <typename O, typename R>
		Connection connect(O *object, R(O::*method)());
		template <typename O, typename R>
		Connection connect(O *object, R(O::*method)(Args...));
		template <typename O, typename R>
		Connection connect(O& object, R(O::*method)());
		template <typename O, typename R>
		Connection connect(O& object, R(O::*method)(Args...));
		template <typename R>
		Connection connect(R(*function)());
		template <typename R>
		Connection connect(R(*function)(Args...));
		template <typename R>
		Connection connect(std::function<R()>& function);
		template <typename R>
		Connection connect(std::function<R(Args...)>& function);
	};

	template <>
	class Signal<> : public detail::AbstractSignal<>
	{
	public:
		Signal() = default;
		virtual ~Signal() = default;

		template <typename O, typename R>
		Connection connect(O *object, R(O::*method)());
		template <typename R>
		Connection connect(R(*function)());
		template <typename R>
		Connection connect(std::function<R()>& function);
	};

	template <>
	class Signal<void> : public Signal<>
	{
		
	};
}

#include <Atema/Core/Signal.inl>

#endif