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
#include <algorithm>
#include "TypeId.h"

template<typename T = class ErRuntimeError>
struct DeriveError : T
{
	using T::T;

	static std::set<size_t> GetBaseIds()
	{
		std::set<size_t> Bases = { GetTypeId<T>() };

		std::set<size_t> OtherBases = T::GetBaseIds();

		std::set<size_t> Result;

		std::merge(Bases.begin(), OtherBases.end(),
			Bases.begin(), OtherBases.end(),
			std::inserter(Result, Result.begin()));

		return Result;
	}
};


struct ErRuntimeError
{

	static std::set<size_t> GetBaseIds()
	{
		return { GetTypeId<ErRuntimeError>() };
	}

	ErRuntimeError()
	{
	}

	ErRuntimeError(const std::string& InMessage)
	{
		Message = InMessage;
	}

	std::string What() const
	{
		if (Message.empty())
			return GetErrorType();
		return GetErrorType() + ": " + Message;
	}

	virtual std::string GetErrorType() const
	{
		return "RuntimeError";
	}

	virtual ~ErRuntimeError() = default;

protected:

	std::string Message;
};

inline std::string error_to_str(const ErRuntimeError& Error)
{
	return Error.What();
}

#define DEFINE_SIMPLE_RUNTIME_ERROR(Error, Parent) \
	struct Error : DeriveError<Parent> \
	{ \
		using ParentType = DeriveError<Parent>; \
		using ParentType::ParentType; \
		virtual std::string GetErrorType() const override \
		{ \
			return #Error; \
		} \
	} \

DEFINE_SIMPLE_RUNTIME_ERROR(ErMathError, ErRuntimeError);
DEFINE_SIMPLE_RUNTIME_ERROR(ErZeroDivisionError, ErMathError);
DEFINE_SIMPLE_RUNTIME_ERROR(ErValueError, ErRuntimeError);
