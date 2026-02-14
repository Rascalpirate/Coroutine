// 这个文件用于“教学近似”：展示协程函数被编译器改写后的 FSM 思路。
// 注意：并非真实编译器输出，而是为了理解 co_await 的状态跳转。

#include <iostream>

struct EchoSessionFSM
{
    enum State
    {
        kStart,
        kAfterRecv,
        kDone,
    } state {kStart};

    int  conn {-1};
    char buf[1024] {};
    long recv_n {0};

    bool resume()
    {
        switch (state)
        {
        case kStart:
            // 等价于: recv_n = co_await async_recv(conn, buf, 1024);
            state = kAfterRecv;
            return false; // 挂起，等待 async_recv 完成后再 resume

        case kAfterRecv:
            if (recv_n <= 0)
            {
                state = kDone;
                return true;
            }
            // 等价于: send(conn, buf, recv_n);
            std::cout << "echo once, bytes=" << recv_n << "\n";
            state = kStart;
            return false; // 继续下一轮 recv，挂起

        case kDone:
            return true;
        }
        return true;
    }
};

int main()
{
    EchoSessionFSM fsm;
    // 模拟调度器多次恢复：
    fsm.resume();   // start -> suspend after scheduling recv
    fsm.recv_n = 5; // 模拟异步 recv 完成
    fsm.resume();   // afterRecv -> send -> start
    fsm.resume();   // start -> suspend
    fsm.recv_n = 0; // 模拟连接关闭
    bool done  = fsm.resume();
    std::cout << "done=" << done << "\n";
}
