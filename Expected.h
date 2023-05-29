// Copyright (c) 2023, Artem Selivanov
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <set>
#include <coroutine>
#include <optional>
#include "TypeId.h"
#include "Error.h"

template<typename ErrorType>
struct Unexpected;


struct ExpectedBase
{
	ExpectedBase()
	{
		ErrHolder = nullptr;
	}

	ExpectedBase(const ExpectedBase& Other)
	{
		if (Other.ErrHolder)
			ErrHolder = Other.ErrHolder->Copy();
		else
			ErrHolder = nullptr;
	}

	template<typename ErrorType>
	ExpectedBase(Unexpected<ErrorType> Unexpected)
	{
		SetError(Unexpected.Error);
	}

	bool HasError() const
	{
		return ErrHolder != nullptr;
	}

	void Unwrap() const
	{
		if (ErrHolder)
		{
			const std::string ErrorMessage = ErrHolder->Handle();
			printf("Unxpected: %s", ErrorMessage.c_str());
		}
	}

	template<typename ErrorType>
	void SetError(ErrorType Error)
	{
		ErrHolder = std::make_unique<TErrorHolder<ErrorType>>(Error);

		if constexpr (std::is_base_of_v<ErRuntimeError, ErrorType>)
		{
			std::set<size_t> Bases = ErrorType::GetBaseIds();
			Bases.insert(GetTypeId<ErrorType>());
			ErrHolder->SetBases(Bases);
		}
	}

	template<typename ErrorType>
	const ErrorType* GetError() const
	{
		const size_t ErrorTypeId = TypeIdGetTypeId<std::decay_t<ErrorType>>();
		return (const ErrorType*)ErrHolder->RetrieveErrorExact(ErrorTypeId);
	}

	template<typename ErrorType>
	typename std::enable_if<
		std::is_base_of_v<ErRuntimeError, ErrorType>,
		const ErrorType*>::type
		Catch()
	{
		const size_t ErrorTypeId = GetTypeId<std::decay_t<ErrorType>>();
		return (const ErrorType*)ErrHolder->CatchError(ErrorTypeId);
	}

	
	struct ErrorHolderBase
	{
		virtual std::unique_ptr<ErrorHolderBase> Copy() const = 0;

		virtual std::string Handle() const = 0;

		virtual void* RetrieveErrorExact(size_t ErrorTypeId) const = 0;

		virtual void* GetErrorPtr() const = 0;

		virtual ~ErrorHolderBase() {};

		void SetBases(const std::set<size_t>& InBases)
		{
			Bases = InBases;
		}

		void* CatchError(size_t ErrorTypeId) const
		{
			if (Bases.contains(ErrorTypeId))
				return GetErrorPtr();
			return nullptr;
		}

		std::set<size_t> Bases;
	};


	template<typename ErrType>
	struct TErrorHolder : public ErrorHolderBase
	{
		using ErrorType = std::decay_t<ErrType>;

		TErrorHolder(ErrorType InError)
		{
			Error = InError;
		}
		virtual std::unique_ptr<ErrorHolderBase> Copy() const override
		{
			auto Err = std::make_unique<TErrorHolder<ErrorType>>(Error);
			Err->SetBases(Bases);
			return Err;
		}
		virtual std::string Handle() const override
		{
			const std::string OutputString = error_to_str(Error);
			return OutputString;
		};
		virtual void* RetrieveErrorExact(size_t ErrorTypeId) const override
		{
			if (ErrorTypeId == GetTypeId<ErrorType>())
				return GetErrorPtr();
			return nullptr;
		}

		virtual void* GetErrorPtr() const override
		{
			return (void*)&Error;
		}

		ErrorType Error;
	};

	std::unique_ptr<ErrorHolderBase> ErrHolder;
};

template<typename T = void>
struct Expected;




template<>
struct Expected<void> : public ExpectedBase
{
	Expected()
	{
	}

	template<typename ErrorType>
	Expected(Unexpected<ErrorType> Unexpected)
		: ExpectedBase(Unexpected)
	{
	}

	bool IsSucceeded() const
	{
		return !HasError();
	}
};


template<typename T>
struct Expected : public ExpectedBase
{
	struct promise_type
	{
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void unhandled_exception() noexcept {}
		Expected<T> get_return_object()
		{
			return (Expected<T>)(Expected<T>::HandleType::from_promise(*this));
		}

		Expected<T>* Exp;


		void return_value(Expected<T> InExpected)
		{
			if (InExpected.HasValue())
				Exp->SetValue(InExpected.Value.value());
			else if (InExpected.HasError())
				Exp->ErrHolder = InExpected.ErrHolder->Copy();
		}
	};
	using HandleType = std::coroutine_handle<promise_type>;

	bool await_ready()
	{
		return !HasError();
	}

	void await_suspend(std::coroutine_handle<promise_type> Handle)
	{
		Handle.promise().Exp->ErrHolder = ErrHolder->Copy();
		Handle.destroy();
	}

	Expected<T> await_resume()
	{
		return *this;
	}

	Expected<T> operator=(const Expected<T>& Other)
	{
		if (Other.HasError())
			ErrHolder = Other.ErrHolder->Copy();
		if (Other.HasValue())
			SetValue(Other.Unwrap());
		return *this;
	}

	HandleType Handle;

	Expected(std::coroutine_handle<promise_type> InHandle)
	{
		Handle = InHandle;
		Handle.promise().Exp = this;
	}

	Expected(T InValue)
	{
		Value = InValue;
	}

	Expected(const Expected& Other)
	{
		if (Other.ErrHolder)
			ErrHolder = Other.ErrHolder->Copy();
		Value = Other.Value;
	}

	template<typename ErrorType>
	Expected(Unexpected<ErrorType> Unexpected)
		: ExpectedBase(Unexpected)
	{
		Value.reset();
	}


	void SetValue(T V)
	{
		Value = V;
	}

	T Unwrap() const
	{
		ExpectedBase::Unwrap();
		return *Value;
	}

	T operator*() const
	{
		return Unwrap();
	}

	bool HasValue() const
	{
		return Value.has_value();
	}

	T ValueOr(const T Default)
	{
		if (Value.IsSet() && !HasError())
			return *Value;
		return Default;
	}

	operator T()
	{
		return Unwrap();
	}

	std::optional<T> Value;
};

template<typename T>
Expected(T Value) -> Expected<T>;
Expected()->Expected<>;


template<typename ErrorType>
struct Unexpected
{
	Unexpected(ErrorType InError)
	{
		Error = InError;
	}

	ErrorType Error;
};