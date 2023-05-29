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

#include "Erxpected.h"
#include "Expected.h"
#include "Try.h"

using namespace std;

#define expect *co_await


Expected<int> Ok()
{
	return 1;
}

Expected<int> Fail()
{
	co_return Unexpected(ErMathError("m. err"));
}

Expected<int> MaybeA()
{
	int A = co_await Fail();
	int B = co_await Ok();
	co_return A + B;
}

Expected<int> MaybeB()
{
	//int Res1 = co_await Ok();
	//int Res2 = co_await Ok();
	//int Res3 = co_await MaybeA();
	//int Res4 = co_await Ok();
	//co_return Res1 + Res2 + Res3;
	co_return
		expect Ok() +
		expect Ok() +
		expect MaybeA() +
		expect Ok();
}

Expected<int> MonadicWrong_Catch()
{
	int Res1 = co_await Ok();
	int Res2 = co_await Ok();
	auto Res3 = MaybeA();
	if (Res3.HasError())
	{
		std::cout << "Error occured!\n";
		co_await Res3;  // rethrow
	}
	int Res4 = co_await Ok();
	co_return Res1 + Res2 + Res3.Unwrap();
}


Expected<int> MonadicOk()
{
	int Res1 = co_await Ok();
	int Res2 = co_await Ok();
	co_return Res1 + Res2;
}

int main()
{
	int OkValue = Ok();

	Expected<int> WrongExpected = MaybeB();

	auto expected = Try(
		[&] () -> Expected<int> {
			return MaybeB();
		},
		[&](const ErMathError& Err)
		{
			return 12;
		},
		[&](const ErRuntimeError& Err)
		{
			return 13;
		});

	if (WrongExpected.HasValue())
	{
		std::cout << WrongExpected.Unwrap();
	}
	else if (auto Error = WrongExpected.Catch<ErRuntimeError>())
	{
		std::cout << "Error: " << Error->What();

	}

	// You will give error message and crash here
	// int WrongResult = WrongExpected;

	return 0;
}
