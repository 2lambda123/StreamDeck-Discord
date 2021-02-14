/*
 * Copyright (c) 2018-present, Frederick Emmott.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the LICENSE file
 * in the root directory of this source tree.
 */

#include <asio.hpp>

template<typename T>
struct AwaitablePromiseBase {
  public:
    AwaitablePromiseBase(
      asio::io_context& ctx
    ): mTimer(
      new asio::steady_timer(
        ctx,
        std::chrono::steady_clock::time_point::max()
      )
    ) {
    }

    asio::awaitable<void> async_wait() {
      asio::error_code ec;
      co_await mTimer->async_wait(asio::redirect_error(asio::use_awaitable, ec));
      assert(ec == asio::error::operation_aborted);
    }

  protected:
    void resolve() noexcept {
      mTimer->cancel();
    }

  private:
    std::shared_ptr<asio::steady_timer> mTimer;
};

template<class T>
struct AwaitablePromise : public AwaitablePromiseBase<T> {
  AwaitablePromise(
    asio::io_context& ctx
  ) : AwaitablePromiseBase<T>(ctx), mData(std::make_shared<T>()) {
  }

  void resolve(T data) noexcept {
    *mData = data;
    AwaitablePromiseBase<T>::resolve();
  }

  T result() const {
    return *mData;
  }

  asio::awaitable<T> async_wait() {
    co_await AwaitablePromiseBase<T>::async_wait();
    co_return result();
  }
  private:
    std::shared_ptr<T> mData;
};

template<>
struct AwaitablePromise<void> : public AwaitablePromiseBase<void> {
  using AwaitablePromiseBase<void>::AwaitablePromiseBase;
  using AwaitablePromiseBase<void>::resolve;
};
