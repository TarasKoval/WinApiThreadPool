#include <iostream>
#include <process.h>

#include "winapi_thread_pool.h"

thread_pool::thread_pool() : m_shutdown(0)
{
    SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	m_number_of_threads = sysinfo.dwNumberOfProcessors;
    
    InitializeSRWLock(&m_lock);

    m_event_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_event_handle == NULL)
    {
        throw std::logic_error("CreateEvent failed");
    }

    m_threads_handle = (HANDLE*) malloc(m_number_of_threads * sizeof(HANDLE));

    for(DWORD i = 0; i < m_number_of_threads; ++i)
    {
        m_threads_handle[i] = (HANDLE)_beginthreadex(0, 0, &thread_pool::start_worker_thread, this, 0, 0);
        if(m_threads_handle[i] == 0)
        {
            //need to safely finish the program
            if (i == 0)
            {
            	throw std::logic_error("Cannot create any worker thread");
            }
            else
            {
                InterlockedIncrement(&m_shutdown);

                for(DWORD j = 0; j < i; ++j)
                    SetEvent(m_event_handle);

                for(DWORD j = 0; j < i; ++j)
                {
                    DWORD ExitCode;
                    GetExitCodeThread(m_threads_handle[j], &ExitCode);
                    if (ExitCode == STILL_ACTIVE)
                        WaitForSingleObject(m_threads_handle[j], INFINITE);
                    CloseHandle(m_threads_handle[j]);
                }

                free(m_threads_handle);

                throw std::logic_error("Cannot create a worker thread");
            }
        }
    }
}

thread_pool::~thread_pool()
{
    InterlockedIncrement(&m_shutdown);

    for(DWORD i = 0; i < m_number_of_threads; ++i)
        SetEvent(m_event_handle);

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
    while (0 == InterlockedExchangeAdd(&m_shutdown, 0)) {
		AcquireSRWLockExclusive(&m_lock);
		if (!m_task_queue.empty()) {
            auto task = m_task_queue.front();
            m_task_queue.pop();
            ReleaseSRWLockExclusive(&m_lock);

            //if an exception will be thrown during execution of this task, 
            //it will be stored in std::future and thrown when std::future::get() is called
            //but packaged_task::operator() can throw exception by itself
            try
            {
                task();
            }
            catch(const std::exception& e)
            {
                std::cerr << "Exception while executing task on the thread pool: " << e.what() << '\n';
            }
            catch(...)
            {
                std::cerr << "Unknown exception while executing task on the thread pool" << '\n';
            }
        } else {
			ReleaseSRWLockExclusive(&m_lock);
            WaitForSingleObject(m_event_handle, INFINITE);
        }
    }

    return 0;
}
