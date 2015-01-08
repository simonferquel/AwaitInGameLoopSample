#pragma once
#include <memory>
#include <experimental\resumable>

// This is the promise that handles the functions needed by the await facility
// - await_suspend is called when a fiber is being suspended until the coroutine has finished. the resumable_handle object
// must be called back to trigger the resume of the fiber
// - await_resume is called when the suspended fiber has resumed. if the coroutine has produced a result, it must be returned by this method.
// if an exception must be propagated, it should also be done at this point 
// - await_ready is called by the fiber to check if the coroutine has already reached a completion state before suspending
template<typename T>
class GameAwaitableSharedPromise
{
private:
	class SharedState {
	private:
		T _value;
		bool _isReady;
		std::experimental::resumable_handle<> _resumeCB;
	public:
		SharedState() : _isReady(false), _resumeCB(nullptr){}
		explicit SharedState(const T& value) : _isReady(true), _value(value) {}

		T value() const {
			return _value;
		}
		bool ready() const {
			return _isReady;
		}

		void setCallback(const std::experimental::resumable_handle<>& callback) {
			_resumeCB = callback;
		}
		template<typename U>
		void setResult(U&& value) {
			_value = std::forward<U>(value);
			if (_resumeCB) {
				_resumeCB();
			}
		}
	};
	std::shared_ptr<SharedState> _state;
public:
	GameAwaitableSharedPromise() : _state(std::make_shared<SharedState>()) {}
	explicit GameAwaitableSharedPromise(const T& value) : _state(std::make_shared<SharedState>(value)) {}


	bool await_ready() const {
		return _state->ready();
	}
	void await_suspend(std::experimental::resumable_handle<> _ResumeCb) {
		_state->setCallback(_ResumeCb);
	}
	T await_resume() const {
		return _state->value();
	}

	template<typename U>
	void setResult(U&& value) {
		_state->setResult(std::forward<U>(value));
	}
};

// specialization for a coroutine producing no result
template<>
class GameAwaitableSharedPromise<void>
{
private:
	class SharedState {
	private:
		bool _isReady;
		std::experimental::resumable_handle<> _resumeCB;
	public:
		explicit SharedState(bool ready) : _isReady(ready), _resumeCB(nullptr) {}

		bool ready() const {
			return _isReady;
		}

		void setCallback(const std::experimental::resumable_handle<>& callback) {
			_resumeCB = callback;
		}

		void setResult() {
			if (_resumeCB) {
				_resumeCB();
			}
		}


	};
	std::shared_ptr<SharedState> _state;
public:
	GameAwaitableSharedPromise() : _state(std::make_shared<SharedState>(false)) {}
	explicit GameAwaitableSharedPromise(bool ready) : _state(std::make_shared<SharedState>(ready)) {}

	void setResult() {
		_state->setResult();
	}


	bool await_ready() const {
		return _state->ready();
	}
	void await_suspend(std::experimental::resumable_handle<> _ResumeCb) {
		_state->setCallback(_ResumeCb);
	}
	void await_resume() const {
	}
};

template<typename T>
struct NoPromise {

};

namespace std {
	namespace experimental {
		
		template <class TResult, class... _Whatever>
		struct resumable_traits<GameAwaitableSharedPromise<TResult>, _Whatever...> {
			struct promise_type {
				GameAwaitableSharedPromise<TResult> _MyPromise;

				GameAwaitableSharedPromise<TResult> get_return_object()
				{
					return _MyPromise;
				}

				suspend_never initial_suspend()
				{
					return{};
				}
				suspend_never final_suspend()
				{
					return{};
				}

				template <class _Ut = TResult>
				enable_if_t<is_same<_Ut, void>::value>
					set_result()
				{
					_MyPromise.setResult();
				}

				template <class _Ut>
				enable_if_t<!is_same<TResult, void>::value && !is_same<_Ut, void>::value>
					set_result(_Ut&& _Value)
				{
					_MyPromise.setResult(std::forward<_Ut>(_Value));
				}

				

				bool cancellation_requested() const
				{
					return false;
				}

				void set_exception(std::exception_ptr exc) {
					// this promise does not support exception
				}
			};
		};
	}
}


class coroutine_abandoned : public std::exception {};

template<typename T>
class GameAwaitableUniquePromise
{
private:
	T _value;
	bool _isReady;
	std::experimental::resumable_handle<> _resumeCB;
	bool _ranToCompletion;

public:
	GameAwaitableUniquePromise(const GameAwaitableUniquePromise<T>&) = delete;
	GameAwaitableUniquePromise& operator=(const GameAwaitableUniquePromise<T>&) = delete;
	GameAwaitableUniquePromise(GameAwaitableUniquePromise<T>&&) = delete;
	GameAwaitableUniquePromise& operator=(GameAwaitableUniquePromise<T>&&) = delete;
	GameAwaitableUniquePromise() : _isReady(false), _ranToCompletion(false), _resumeCB(nullptr) {}
	explicit GameAwaitableUniquePromise(const T& value) : _isReady(true), _ranToCompletion(true), _value(value), _resumeCB(nullptr) {}
	~GameAwaitableUniquePromise() {
		if (!_ranToCompletion) {
			_isReady = true;
			if (_resumeCB) {
				_resumeCB();
			}
		}
	}

	bool await_ready() {
		return _isReady;
	}
	void await_suspend(std::experimental::resumable_handle<> _ResumeCb) {
		_resumeCB = _ResumeCb;
	}
	T await_resume() {
		if (!_ranToCompletion) {
			throw coroutine_abandoned();
		}
		return _value;
	}
	void setResult(const T& value) {
		_value = value;
		_ranToCompletion = true;
		_isReady = true;
		if (_resumeCB) {

			_resumeCB();
		}
	}
};


template<>
class GameAwaitableUniquePromise<void>
{
private:
	bool _isReady;
	std::experimental::resumable_handle<> _resumeCB;
	bool _ranToCompletion;

public:
	GameAwaitableUniquePromise() : _isReady(false), _ranToCompletion(false), _resumeCB(nullptr)
	{
	}
	explicit GameAwaitableUniquePromise(bool ready) : _isReady(ready), _ranToCompletion(ready), _resumeCB(nullptr)
	{
	}

	~GameAwaitableUniquePromise() {
		if (!_ranToCompletion) {
			_isReady = true;
			if (_resumeCB) {
				_resumeCB();
			}
		}
	}

	GameAwaitableUniquePromise(const GameAwaitableUniquePromise<void>&) = delete;
	GameAwaitableUniquePromise& operator=(const GameAwaitableUniquePromise<void>&) = delete;
	GameAwaitableUniquePromise(GameAwaitableUniquePromise<void>&&) = delete;
	GameAwaitableUniquePromise& operator=(GameAwaitableUniquePromise<void>&&) = delete;
	bool await_ready() {
		return _isReady;
	}
	void await_suspend(std::experimental::resumable_handle<> _ResumeCb) {
		_resumeCB = _ResumeCb;
	}
	void await_resume() {
		if (!_ranToCompletion) {
			throw coroutine_abandoned();
		}
	}
	void setResult() {
		_ranToCompletion = true;
		_isReady = true;
		if (_resumeCB) {
			_resumeCB();
		}
	}
};


template<typename T>
bool await_ready(GameAwaitableUniquePromise<T>* p) {
	return p->await_ready();
}


template<typename T>
void await_suspend(GameAwaitableUniquePromise<T>* p, std::experimental::resumable_handle<> _ResumeCb) {
	return p->await_suspend(_ResumeCb);
}


template<typename T>
T await_resume(GameAwaitableUniquePromise<T>* p) {
	return p->await_resume();
}
