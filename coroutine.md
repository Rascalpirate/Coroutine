<!--
 * @Author: error: git config user.name & please set dead value or install git
 * @Date: 2026-02-14 13:40:01
 * @LastEditors: error: git config user.name & please set dead value or install git
 * @LastEditTime: 2026-02-14 15:54:17
 * @FilePath: /MyCoder/CodeX/Coroutine/coroutine.md
 * @Description:
-->

准备一个演示项目，以网络编程中的 server 和 client app 为例，演示一下 coroutine 的演化过程：
1. 基于同步阻塞风格的网络编程，用 socket 开发 server 和 client app;
2. 基于同步非阻塞风格的网络编程，用 epoll 开发 server 和 client app;
3. 基于异步回调风格的网络编程，用 boost asio 开发 server 和 client app;
4. 基于future/promise 风格的网络编程，开发 server 和 client app;
5. 基于Continue Pass Style 的网络编程，开发 server 和 client app;
6. 基于Coroutine 风格的网络编程，开发 server 和 client app;
    6.1. 我们已经讨论过， c++ 20 提供的 coroutine 是 FSM 和 CPS 编程风格的语法糖，实际会生成对应的FSM. 还是以开发 server 和 client app 为例，将 c++ 编译器会生成的 FSM 和 CPS 代码给出来，同时将 std::coroutine 提供的控制 coroutine 的 promise_type 也详细举例说明一下。


diff --git a/README.md b/README.md
index 06b42f25096b0e3ef1d170416ba9cc00ca3e99be..c2b06f61e32c977205c8f1ccfa5e9560c7da2d61 100644
--- a/README.md
+++ b/README.md
@@ -1 +1,119 @@
-# Coroutine
\ No newline at end of file
+# Coroutine 演化演示项目
+
+这个仓库提供一个“同一业务、六种编程风格”的网络编程示例：统一使用 **TCP Echo**（服务端收到消息后原样返回）。
+
+> 目标：从阻塞式网络编程一路演化到 C++20 Coroutine，帮助理解为什么 Coroutine 能在保留同步写法可读性的同时获得异步执行能力。
+
+## 目录结构
+
+- `demo/01_blocking/`：同步阻塞（`socket` + `accept` + `recv/send`）
+- `demo/02_epoll/`：同步非阻塞（`epoll` 事件循环）
+- `demo/03_asio_callback/`：异步回调（Boost.Asio callback chain）
+- `demo/04_future_promise/`：future/promise 风格
+- `demo/05_cps/`：Continuation Passing Style（手写 continuation）
+- `demo/06_coroutine/`：C++20 Coroutine 风格 + FSM/CPS 展开示例 + promise_type 说明
+
+---
+
+## 1) 同步阻塞：`demo/01_blocking`
+
+**核心特点**
+- 代码直观，控制流符合“人脑顺序”。
+- 一个连接通常占用一个线程（或串行处理，吞吐低）。
+- 阻塞在 `accept/recv` 等系统调用上。
+
+**适合**
+- 教学、原型、小规模连接数。
+
+---
+
+## 2) 同步非阻塞 + epoll：`demo/02_epoll`
+
+**核心特点**
+- 文件描述符设置为 `O_NONBLOCK`。
+- 通过 `epoll_wait` 等待事件并分发处理。
+- 单线程可管理大量连接（I/O 多路复用）。
+
+**代价**
+- 业务逻辑被“事件分发 + 状态管理”拆碎，可读性下降。
+
+---
+
+## 3) 异步回调 + Boost.Asio：`demo/03_asio_callback`
+
+**核心特点**
+- `async_accept` / `async_read_some` / `async_write`。
+- 通过回调串起控制流。
+
+**代价**
+- 回调嵌套（callback hell）和生命周期管理复杂度提升。
+
+---
+
+## 4) future/promise 风格：`demo/04_future_promise`
+
+**核心特点**
+- 用 `std::promise/std::future` 表达“稍后可得的结果”。
+- 可组合（`wait/get`），相比裸回调更结构化。
+
+**代价**
+- 仍需要显式管理线程、同步点，组合能力不如原生协程 `co_await` 自然。
+
+---
+
+## 5) CPS（Continuation Passing Style）：`demo/05_cps`
+
+**核心特点**
+- 函数不“返回结果”，而是“接收 continuation 并在完成后调用它”。
+- 控制权显式传递，天然适配异步。
+
+**代价**
+- continuation 层层传递，业务代码变形明显。
+
+---
+
+## 6) Coroutine 风格：`demo/06_coroutine`
+
+**核心特点**
+- 使用 `co_await/co_return` 将异步逻辑写成同步样式。
+- 编译器会把协程函数改写为：
+  - **状态机（FSM）**：记录暂停点、局部变量、恢复逻辑。
+  - **continuation 链接（可理解为 CPS 语义）**：在 `await_suspend` 时安排后续恢复。
+
+### 6.1 C++20 coroutine = FSM + CPS 的语法糖（示例）
+
+见：
+- `demo/06_coroutine/fsm_expansion.cpp`：手写“近似编译器改写”的状态机版本。
+- `demo/06_coroutine/coroutine_server.cpp`：`co_await` 版本的 echo server。
+- `demo/06_coroutine/promise_type_explained.cpp`：完整 `promise_type` 控制点示例（`initial_suspend/final_suspend/return_void/unhandled_exception/get_return_object` 等）。
+
+可对照理解：
+1. 协程函数进入后创建 coroutine frame（堆/优化后栈上）。
+2. `promise_type` 参与返回对象构建和生命周期管理。
+3. 每个 `co_await` 被编译成 awaiter 协议调用：`await_ready/await_suspend/await_resume`。
+4. 挂起时保存当前状态编号（FSM state），恢复时 `switch(state)` 跳转。
+
+---
+
+## 构建方式（CMake）
+
+本仓库已使用 CMake 组织代码，示例按 Linux/POSIX 假设编写。
+
+- 需要 C++20 编译器（GCC 11+/Clang 14+ 建议）
+- 默认尝试构建 `03_asio_callback`，若环境没有 Boost，会自动跳过并给出提示
+- 也可显式关闭 Asio 示例：`-DBUILD_ASIO_DEMOS=OFF`
+
+### 配置与构建
+
+```bash
+cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
+cmake --build build -j
+```
+
+### 仅在无 Boost 环境构建其余示例
+
+```bash
+cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_ASIO_DEMOS=OFF
+cmake --build build -j
+```
+
