#pragma once
#include "opt.h"
#include "typedefs.h"
#include "vec.h"

/*** ROOTS ***/

template <typename Cur, typename End>
struct StdIter {
	using Item = std::reference_wrapper<typename std::iterator_traits<Cur>::value_type>;

	StdIter() = default;
	StdIter(Cur cur, End end) : cur(cur), end(end) {}

	Opt<Item> next() {
		if (cur != end) {
			return Some(std::reference_wrapper(*cur++));
		} else {
			return None();
		}
	}

private:
	Cur cur;
	End end;
};

template <typename T>
struct OwnedIter {
	using Item = typename T::value_type;

	explicit OwnedIter(T&& source) : source(std::move(source)) {
		iter = StdIter(source.begin(), source.end());
	}
	OwnedIter(OwnedIter&& other) : source(std::move(other.source)) {
		iter = StdIter(source.begin(), source.end());
	}

	Opt<Item> next() {
		return iter.next().map([](auto&& x) { return std::move(x.get()); });
	}

private:
	StdIter<typename T::iterator, typename T::iterator> iter;
	T source;
};

template <typename R, typename T>
R iter(T&& source);

template <typename T>
OwnedIter<T> iter(T&& source) {
	return OwnedIter(std::move(source));
}

template <typename T>
StdIter<typename T::iterator, typename T::iterator> iter(T& source) {
	return StdIter(source.begin(), source.end());
}

/*** OPERATORS ***/

template <typename I>
struct EnumerateIter {
	using Item = std::pair<usize, typename I::Item>;

	EnumerateIter(I&& iter) : iter(std::move(iter)) {}

	Opt<Item> next() {
		if (auto value = iter.next()) {
			return Some(std::pair{idx++, *value});
		} else {
			return None();
		}
	}

private:
	I iter;
	usize idx = 0;
};

struct Enumerate {
	Enumerate() {}
};

template <typename I>
EnumerateIter<I> operator|(I iter, Enumerate proxy) {
	return EnumerateIter(std::move(iter));
}

template <typename I, typename F>
struct TransformIter {
	using Item = decltype(std::declval<F>()(std::declval<typename I::Item>()));

	TransformIter(I&& iter, F func) : iter(std::move(iter)), func(func) {}

	Opt<Item> next() {
		if (auto value = iter.next()) {
			return Some(func(*value));
		} else {
			return None();
		}
	}

private:
	I iter;
	F func;
};

template <typename F>
struct Transform {
	F func;

	explicit Transform(F func) : func(func) {}
};

template <typename I, typename F>
TransformIter<I, F> operator|(I iter, Transform<F> proxy) {
	return TransformIter(std::move(iter), proxy.func);
}

template <typename I, typename F>
struct FilterIter {
	using Item = typename I::Item;

	FilterIter(I&& iter, F func) : iter(std::move(iter)), func(func) {}

	Opt<Item> next() {
		while (auto value = iter.next()) {
			if (func(*value)) {
				return Some(*value);
			}
		}

		return None();
	}

private:
	I iter;
	F func;
};

template <typename F>
struct Filter {
	F func;

	explicit Filter(F func) : func(func) {}
};

template <typename I, typename F>
FilterIter<I, F> operator|(I iter, Filter<F> proxy) {
	return FilterIter(std::move(iter), proxy.func);
}

/*** COLLECTORS ***/

struct ToVec {};

template <typename I>
Vec<typename I::Item> operator|(I iter, ToVec proxy) {
	Vec<typename I::Item> vec;
	while (auto&& item = iter.next()) {
		vec.push(std::move(item).unwrap());
	}
	return vec;
}

struct Count {};

template <typename I>
usize operator|(I iter, Count proxy) {
	usize count = 0;
	while (iter.next()) {
		count++;
	}
	return count;
}

struct First {};

template <typename I>
Opt<typename I::Item> operator|(I iter, First proxy) {
	return iter.next();
}
