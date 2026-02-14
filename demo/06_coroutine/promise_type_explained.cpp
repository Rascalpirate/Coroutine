#include <coroutine>
#include <exception>
#include <iostream>

struct DemoTask
{
    struct promise_type
    {
        int value_ {0};

        DemoTask get_return_object()
        {
            return DemoTask {
                std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // initial_suspend: 控制“创建后是否立即执行”
        std::suspend_always initial_suspend() noexcept
        {
            std::cout << "initial_suspend\n";
            return {};
        }

        // final_suspend: 控制“结束后是否挂起等待外部销毁”
        std::suspend_always final_suspend() noexcept
        {
            std::cout << "final_suspend\n";
            return {};
        }

        void unhandled_exception()
        {
            std::cout << "unhandled_exception\n";
            std::terminate();
        }

        void return_value(int v)
        {
            value_ = v;
        }
    };

    std::coroutine_handle<promise_type> h_;

    explicit DemoTask(std::coroutine_handle<promise_type> h) : h_(h)
    {
    }

    DemoTask(DemoTask &&other) noexcept : h_(other.h_)
    {
        other.h_ = {};
    }

    ~DemoTask()
    {
        if (h_)
            h_.destroy();
    }

    void resume()
    {
        if (h_ && !h_.done())
            h_.resume();
    }

    int result() const
    {
        return h_.promise().value_;
    }
};

DemoTask simple_coroutine()
{
    std::cout << "coroutine body begin\n";
    co_return 42;
}

int main()
{
    auto task = simple_coroutine(); // 只创建 frame, 因 initial_suspend 挂起
    std::cout << "after create\n";
    task.resume(); // 真正执行 body
    std::cout << "result=" << task.result() << "\n";
}
