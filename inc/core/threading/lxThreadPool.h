#ifndef INCLUDED_LUX_THREAD_POOL_H
#define INCLUDED_LUX_THREAD_POOL_H
#include <mutex>
#include <thread>
#include <future>
#include "core/lxDeque.h"
#include "core/lxArray.h"

namespace lux
{
namespace core
{

class ThreadPool : public core::Uncopyable
{
private:
	class Function
	{
	public:
		virtual ~Function() {}
		virtual void Call() = 0;
	};

	template <typename FunctorT>
	class FunctionFunctor : public Function
	{
	public:
		using RetT = decltype(std::declval<FunctorT>()());
		using PromiseT = std::promise<RetT>;
		using FutureT = std::future<RetT>;

		FunctorT m_Functor;
		PromiseT m_Promise;
		FunctionFunctor(FunctorT&& f) :
			m_Functor(f)
		{
		}

		template<typename RetT>
		struct Executor { void operator()(FunctorT& f, std::promise<RetT>& p) { p.set_value(f()); } };
		template<>
		struct Executor<void> { void operator()(FunctorT& f, std::promise<RetT>& p) { f(); p.set_value(); } };

		void Call() override
		{
			Executor<RetT> exec;
			exec(m_Functor, m_Promise);
		}

		FutureT GetFuture() { return m_Promise.get_future(); }
	};

	class FunctionQueue
	{
	public:
		void Push(Function* ptr)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Pointers.PushBack(ptr);
		}
		Function* Pop()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			if(m_Pointers.IsEmpty())
				return nullptr;
			auto outPtr = m_Pointers.Front();
			m_Pointers.PopFront();
			return outPtr;
		}
		bool IsEmpty()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			return m_Pointers.IsEmpty();
		}

	private:
		core::Deque<Function*> m_Pointers;
		std::mutex m_Mutex;
	};

public:
	explicit ThreadPool(int nThreads) :
		m_NumWaiting(0),
		m_FlagKill(false)
	{
		m_Threads.Resize(nThreads);
		for(int i = 0; i < nThreads; ++i)
			m_Threads[i] = new std::thread([this, i]() {ThreadFunction(*this, i); });
	}
	~ThreadPool()
	{
		WaitFor();
		m_FlagKill = true;
		WakeThreads();
		ClearThreads();
	}

	template <typename FunctionT>
	std::future<decltype(std::declval<FunctionT>()())> Push(FunctionT&& f)
	{
		auto function = new FunctionFunctor<FunctionT>(std::forward<FunctionT>(f));
		auto future = function->GetFuture();
		m_Queue.Push(function);
		{
			std::unique_lock<std::mutex> lock(m_ThreadSignalMutex);
			m_ThreadSignal.notify_one();
		}
		return future;
	}
	
	template <typename FunctionT, typename... ArgTs>
	std::future<decltype(std::declval<FunctionT>()(std::declval<ArgTs>()...))> PushWithArgs(FunctionT&& f, ArgTs... args)
	{
		return Push([&f, &args...]() {f(std::forward<ArgTs>(args)...); });
	}

	int GetThreadCount() const { return m_Threads.Size(); }
	int GetIdleCount() const { return m_NumWaiting; }

	// Flush all remaining jobs currently in the queue.
	void Flush()
	{
		ClearQueue();
		WakeThreads();
		WaitFor();
	}

	// Wait until all jobs in the queue are finished.
	void WaitFor()
	{
		if(m_NumWaiting == GetThreadCount() && m_Queue.IsEmpty())
			return;
		std::unique_lock<std::mutex> lock(m_ThreadWaitSignalMutex);
		m_ThreadWaitSignal.wait(lock, [this]() { return m_NumWaiting == GetThreadCount() && m_Queue.IsEmpty(); });
	}

private:
	void WakeThreads()
	{
		std::unique_lock<std::mutex> lock(m_ThreadSignalMutex);
		m_ThreadSignal.notify_all();
	}

	void ClearQueue()
	{
		while(auto f = m_Queue.Pop())
			delete f;
	}

	void ClearThreads()
	{
		for(auto& th : m_Threads) {
			if(th->joinable())
				th->join();
			delete th;
		}
	}

	static void ThreadFunction(ThreadPool& pool, const int id)
	{
		LUX_UNUSED(id);
		Function* function = pool.m_Queue.Pop();
		do {
			while(function) {
				function->Call();
				delete function;
				if(pool.m_FlagKill)
					return;
				function = pool.m_Queue.Pop();
			}
			// Queue is empty.

			++pool.m_NumWaiting;
			{
				std::unique_lock<std::mutex> lock(pool.m_ThreadWaitSignalMutex);
				pool.m_ThreadWaitSignal.notify_all();
			}
			{
				std::unique_lock<std::mutex> lock(pool.m_ThreadSignalMutex);
				pool.m_ThreadSignal.wait(lock, [&pool, &function]() {
					function = pool.m_Queue.Pop();
					return function || pool.m_FlagKill;
				});
			}
			--pool.m_NumWaiting;
		} while(!pool.m_FlagKill);
	}

private:
	core::Array<std::thread*> m_Threads;

	// Should all the threads stop waiting.
	// Should all the threads end.
	std::atomic<bool> m_FlagKill;
	std::atomic<int> m_NumWaiting;

	FunctionQueue m_Queue;

	std::mutex m_ThreadSignalMutex;
	std::condition_variable m_ThreadSignal;

	std::mutex m_ThreadWaitSignalMutex;
	std::condition_variable m_ThreadWaitSignal;
};

}
}

#endif // #ifndef INCLUDED_LUX_THREAD_POOL_H
