#pragma once

#include <compare>

#if !(__cplusplus > 201703L && defined(__cpp_lib_three_way_comparison) && __cpp_lib_three_way_comparison >= 201907L) \
	&& !defined(_MSC_VER)

#	include <concepts>
#	include <functional>
#	include <iterator>

namespace std {

// Provide compatibility implementation of std::lexicographical_compare_three_way() from libc++ bug tracker.
template<class _Tp, class _Cat>
concept __compares_as = std::same_as<std::common_comparison_category_t<_Tp, _Cat>, _Cat>;

template<class _Tp, class _Cat = std::partial_ordering>
concept three_way_comparable = __weakly_equality_comparable_with<_Tp, _Tp> && __partially_ordered_with<_Tp, _Tp>
							&& requires(__make_const_lvalue_ref<_Tp> __a, __make_const_lvalue_ref<_Tp> __b) {
								   { __a <=> __b } -> __compares_as<_Cat>;
							   };

template<class _Tp, class _Up, class _Cat = std::partial_ordering>
concept three_way_comparable_with
	= three_way_comparable<_Tp, _Cat> && three_way_comparable<_Up, _Cat>
   && std::common_reference_with<__make_const_lvalue_ref<_Tp>, __make_const_lvalue_ref<_Up>>
   && three_way_comparable<std::common_reference_t<__make_const_lvalue_ref<_Tp>, __make_const_lvalue_ref<_Up>>, _Cat>
   && __weakly_equality_comparable_with<_Tp, _Up> && __partially_ordered_with<_Tp, _Up>
   && requires(__make_const_lvalue_ref<_Tp> __t, __make_const_lvalue_ref<_Up> __u) {
		  { __t <=> __u } -> __compares_as<_Cat>;
		  { __u <=> __t } -> __compares_as<_Cat>;
	  };

struct _LIBCPP_TEMPLATE_VIS compare_three_way {
	template<class _T1, class _T2>
	requires three_way_comparable_with<_T1, _T2>
	constexpr _LIBCPP_HIDE_FROM_ABI auto operator()(_T1 &&__t, _T2 &&__u) const
		noexcept(noexcept(std::forward<_T1>(__t) <=> std::forward<_T2>(__u))) {
		return std::forward<_T1>(__t) <=> std::forward<_T2>(__u);
	}

	using is_transparent = void;
};

template<class _InputIterator1, class _InputIterator2, class _Cmp>
_LIBCPP_HIDE_FROM_ABI constexpr auto lexicographical_compare_three_way(
	_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _InputIterator2 __last2, _Cmp __comp)
	-> decltype(__comp(*__first1, *__first2)) {
	using return_type_t = decltype(__comp(*__first1, *__first2));
	static_assert(
		std::disjunction_v<std::is_same<return_type_t, std::strong_ordering>,
			std::is_same<return_type_t, std::weak_ordering>, std::is_same<return_type_t, std::partial_ordering>>,
		"The return type must be a comparison category type.");

	if constexpr (__is_cpp17_random_access_iterator<_InputIterator1>::value
				  && __is_cpp17_random_access_iterator<_InputIterator2>::value) {
		// Fast path for random access iterators which computes the number of loop iterations up-front and
		// then skips the iterator comparisons inside the loop.
		static_assert(std::is_integral_v<typename std::iterator_traits<_InputIterator1>::difference_type>,
			"Using a non-integral difference_type is undefined behavior");
		static_assert(std::is_integral_v<typename std::iterator_traits<_InputIterator2>::difference_type>,
			"Using a non-integral difference_type is undefined behavior");

		auto __len1    = __last1 - __first1;
		auto __len2    = __last2 - __first2;
		auto __min_len = __len1 < __len2 ? __len1 : __len2;

		for (decltype(__min_len) __i = 0; __i < __min_len; ++__i) {
			auto __c = __comp(*__first1, *__first2);
			if (__c != 0) {
				return __c;
			}
			++__first1;
			++__first2;
		}

		return __len1 <=> __len2;
	}
	else {
		// Unoptimized implementation which compares the iterators against the end in every loop iteration
		while (true) {
			bool __exhausted1 = __first1 == __last1;
			bool __exhausted2 = __first2 == __last2;

			if (__exhausted1 || __exhausted2) {
				if (!__exhausted1)
					return strong_ordering::greater;
				if (!__exhausted2)
					return strong_ordering::less;
				return strong_ordering::equal;
			}

			auto __c = __comp(*__first1, *__first2);
			if (__c != 0) {
				return __c;
			}

			++__first1;
			++__first2;
		}
	}
}

template<class _InputIterator1, class _InputIterator2>
_LIBCPP_HIDE_FROM_ABI constexpr auto lexicographical_compare_three_way(
	_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _InputIterator2 __last2) {
	return std::lexicographical_compare_three_way(__first1, __last1, __first2, __last2, std::compare_three_way{});
}

} // namespace std

#endif
