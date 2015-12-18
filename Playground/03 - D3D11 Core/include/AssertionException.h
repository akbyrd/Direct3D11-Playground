#pragma once

#define throw_assert(x) do { if ( !(x) ) { /**/ } } while (0)

class AssertionException : public std::exception
{
};