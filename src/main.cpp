#include <iostream>

#include "winapi_thread_pool.h"

static int multiplication(int a, int b) {
    return a * b;
}

static void addition(int &result, int a, int b) {
    result = a + b;
	throw std::logic_error("Something horrible happened");
}

int main() {
    thread_pool Pool;
    int addition_result = 0;

    auto multiplication_result = Pool.submit(multiplication, 5, 10);
    auto addition_result_is_ready = Pool.submit(addition, std::ref(addition_result), 5, 10);

    std::cout << multiplication_result.get() << std::endl;

    try
	{
		addition_result_is_ready.get();
	} catch (const std::exception &e)
    {
		std::cout << "From main, I catch exception: " << e.what() << std::endl;
	}
    std::cout << addition_result << std::endl;

	return 0;
}
