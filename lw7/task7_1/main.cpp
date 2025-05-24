#include <iostream>
#include <coroutine>
#include <string>
#include "_libs/_exceptionHandler.h"
#include "_libs/_logger.h"

class MyTask;

struct Promise
{
    std::string value;
    std::exception_ptr exception;

    MyTask get_return_object();

    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_value(std::string v) { value = std::move(v); }
    void unhandled_exception() { exception = std::current_exception(); }
};

class MyTask
{
public:
    using promise_type = Promise;

    explicit MyTask(std::coroutine_handle<Promise> h)
            : handle(h) {}

    MyTask(const MyTask&) = delete;
    MyTask& operator=(const MyTask&) = delete;

    MyTask(MyTask&& other) noexcept: handle(other.handle)
    {
        other.handle = nullptr;
    }

    MyTask& operator=(MyTask&& other) noexcept
    {
        if (this != &other)
        {
            if (handle)
            {
                handle.destroy();
            }
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~MyTask()
    {
        if (handle)
        {
            handle.destroy();
        }
    }

    [[nodiscard]] std::string GetResult() const
    {
        if (handle.promise().exception)
        {
            std::rethrow_exception(handle.promise().exception);
        }
        return handle.promise().value;
    }

private:
    std::coroutine_handle<Promise> handle;
};

MyTask Promise::get_return_object()
{
    return MyTask{std::coroutine_handle<Promise>::from_promise(*this)};
}

MyTask SimpleCoroutine()
{
//    throw std::runtime_error("This is an error in the coroutine!");
    co_return std::string("Hello from coroutine!");
}

int main()
{
    ExceptionHandler exceptionHandler;
    exceptionHandler.Handle([]() {
        auto task = SimpleCoroutine();
        Logger::Println(task.GetResult());
    });
    if (exceptionHandler.WasExceptionCaught())
    {
        Logger::Error(exceptionHandler.GetErrorMessage());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
