#pragma once

#include <future>
#include <queue>

#include <windows.h>

class thread_pool {

    volatile DWORD      m_shutdown;

    DWORD               m_number_of_threads;
    SRWLOCK             m_lock;
    HANDLE              m_event_handle;
    HANDLE*             m_threads_handle;

    std::queue<std::function<void()>> m_task_queue;

    unsigned int worker_thread();

    static unsigned int __stdcall start_worker_thread(void *p_this)
    {
        try
        {
            thread_pool* p_object = static_cast<thread_pool*>(p_this);
            p_object->worker_thread();
        }
        catch(const std::exception& e)
        {
            std::cerr << "Exception in start_worker_thread(): " << e.what() << '\n';
        }
        catch(...)
        {
            std::cerr << "Unknown exception in start_worker_thread()" << '\n';
        }
        
        return 0;
    }


public:
    thread_pool();

    ~thread_pool();

    template<typename F, typename...Args>
    auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        // This allows passing a function with many parameters
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // Packaged_task wrapped in shared_ptr to be able to copy construct / assign
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(task);
        // Wrap into void function
        std::function<void()> wrapped_task = [task_ptr]() { (*task_ptr)(); };

        AcquireSRWLockExclusive(&m_lock);
            m_task_queue.push(wrapped_task);    
        ReleaseSRWLockExclusive(&m_lock);

        SetEvent(m_event_handle);

        return task_ptr->get_future();
    }
};
