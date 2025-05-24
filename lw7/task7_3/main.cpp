#include <iostream>
#include <coroutine>
#include <optional>

struct MyAwaiter {
    int x;
    int y;

    // выяснить что нужно сделать чтобы сразу вернулось значение без остановки
    bool await_ready() const noexcept { return false; }

    // выяснить что можно указать в качестве возвращаемого типа
    // зачем noexcept
    void await_suspend(std::coroutine_handle<> handle) const noexcept {
        // Сохраняем handle для последующего возобновления
        handle.resume(); // Возобновляем сразу после приостановки
    }

    int await_resume() const noexcept {
        return x + y;
    }
};

class MyTask {
public:
    // что нужно сделать чтобы MyTask можно было использовать в качестве результата co_await
    // что нужно сделать чтоб особые методы не были видны клиентам напрямую
    struct promise_type {

        MyTask get_return_object() {
            return MyTask(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        std::suspend_always initial_suspend() noexcept { return {}; } // Приостанавливаем при старте
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    explicit MyTask(std::coroutine_handle<promise_type> handle) : handle(handle) {}

    ~MyTask() {
        if (handle) handle.destroy();
    }

    void Resume() {
        if (handle && !handle.done()) {
            handle.resume();
        }
    }

private:
    std::coroutine_handle<promise_type> handle;
};

MyTask CoroutineWithAwait(int x, int y) {
    std::cout << "Before await\n";
    int result = co_await MyAwaiter{x, y};
    std::cout << result << "\n";
    std::cout << "After await\n";
}

int main() {
    auto task = CoroutineWithAwait(30, 12);
    std::cout << "Before resume\n";
    task.Resume();
    std::cout << "After resume\n";
    CoroutineWithAwait(5, 10).Resume();
    std::cout << "End of main\n";
}