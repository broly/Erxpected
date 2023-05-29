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

#include "Expected.h"

// Список аргументов функции
template<typename...>
struct TFunctionTraits_Args;

// Одна из специлизаций (только для первого аргумента)
template<typename T0>
struct TFunctionTraits_Args<T0>
{
	using Type0 = T0;
};


// Шаблонная мета-функция, выдающая информацию о функции
template <typename T>
struct TFunctionTraits : public TFunctionTraits<decltype(&T::operator())>
{};

template <typename ClassType, typename R, typename... Args>
struct TFunctionTraits<R(ClassType::*)(Args...) const> : public TFunctionTraits_Args<Args...>
{
	using ReturnType = R;
	static constexpr auto Arity = sizeof...(Args);
};


template<typename TryCallable, typename... CatchCallables>
auto Try(TryCallable&& InTryCallable, CatchCallables&&... InCatchCallables)
{
	auto expected = InTryCallable();

	bool Caught = false;


	auto Handler = [&] <typename T> (auto && CatchCallable)
	{
		if (!Caught)
		{
			if (auto V = expected.Catch<std::decay_t<T>>())
			{
				Caught = true;
				expected = CatchCallable(*V);
			}
		}
	};

	(Handler.operator() < typename TFunctionTraits<CatchCallables>::Type0 > (InCatchCallables), ...);

	return expected;
}