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
class GameAwaitablePromise
{
private:
	class SharedState {
	private:
		T _value;
		bool _isReady;
		std::experimental::resumable_handle<> _resumeCB;
	public:
		SharedState() : _isReady(false) {}
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

		void setResult(const T& value) {
			_value = value;
			_resumeCB();
		}
	};
	std::shared_ptr<SharedState> _state;
public:
	GameAwaitablePromise() : _state(std::make_shared<SharedState>()) {}
	explicit GameAwaitablePromise(const T& value) : _state(std::make_shared<SharedState>(value)) {}


	bool await_ready() const {
		return _state->ready();
	}
	void await_suspend(std::experimental::resumable_handle<> _ResumeCb) {
		_state->setCallback(_ResumeCb);
	}
	T await_resume() const {
		return _state->value();
	}
};

// specialization for a coroutine producing no result
template<>
class GameAwaitablePromise<void>
{
private:
	class SharedState {
	private:
		bool _isReady;
		std::experimental::resumable_handle<> _resumeCB;
	public:
		explicit SharedState(bool ready) : _isReady(ready), _resumeCB(nullptr){}

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
	GameAwaitablePromise() : _state(std::make_shared<SharedState>(false)) {}
	explicit GameAwaitablePromise(bool ready) : _state(std::make_shared<SharedState>(ready)) {}

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
