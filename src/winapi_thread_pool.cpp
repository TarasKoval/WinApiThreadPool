#include <iostream>
#include <process.h>

#include "winapi_thread_pool.h"

thread_pool::thread_pool() : m_shutdown(0)
{
    SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	m_number_of_threads = sysinfo.dwNumberOfProcessors;
    
    InitializeSRWLock(&m_lock);
	InitializeConditionVariable(&m_condition);

    m_threads_handle = (HANDLE*) malloc(m_number_of_threads * sizeof(HANDLE));

    for(DWORD i = 0; i < m_number_of_threads; ++i)
    {
        m_threads_handle[i] = (HANDLE)_beginthreadex(0, 0, &thread_pool::start_worker_thread, this, 0, 0);
        if(m_threads_handle[i] == NULL)
        {
            std::cout << "Cannot create a thread, thread number: " << i << std::endl;
        }
    }
}

thread_pool::~thread_pool()
{
    AcquireSRWLockExclusive(&m_lock);
        InterlockedIncrement(&m_shutdown);
    ReleaseSRWLockExclusive(&m_lock);

    WakeAllConditionVariable(&m_condition);
    WaitForMultipleObjects(m_number_of_threads, m_threads_handle, TRUE, INFINITE);
    for(DWORD i = 0; i < m_number_of_threads; ++i)
    {
        DWORD exit_code;
        GetExitCodeThread(m_threads_handle[i], &exit_code);
        if (exit_code)
        {
            std::cout << "Thread exited with error code: " << exit_code << std::endl;
        }
        
        CloseHandle(m_threads_handle[i]);
    }

    free(m_threads_handle);
}

unsigned int thread_pool::worker_thread()
{
    AcquireSRWLockExclusive(&m_lock);

    while (m_shutdown == 0) {
        if (!m_task_queue.empty()) {
            auto task = m_task_queue.front();
            m_task_queue.pop();
            ReleaseSRWLockExclusive(&m_lock);

            //if an exception will be thrown during execution of this task, 
            //it will be stored in std::future and thrown when std::future::get() is called
            task();

            AcquireSRWLockExclusive(&m_lock);
        } else {
            SleepConditionVariableSRW(&m_condition, &m_lock, INFINITE, 0);
        }
    }

    ReleaseSRWLockExclusive(&m_lock);
    return 0;
}
