#pragma once

#include <dv-processing/data/cvector.hpp>

namespace dv {

template<class T>
class cvectorConstProxy {
public:
	// Container traits.
	using value_type       = typename cvector<T>::value_type;
	using const_value_type = typename cvector<T>::const_value_type;
	using pointer          = typename cvector<T>::pointer;
	using const_pointer    = typename cvector<T>::const_pointer;
	using reference        = typename cvector<T>::reference;
	using const_reference  = typename cvector<T>::const_reference;
	using size_type        = typename cvector<T>::size_type;
	using difference_type  = typename cvector<T>::difference_type;

	static constexpr size_type npos{cvector<T>::npos};

protected:
	cvector<T> *mVectorPtr;

private:
	static inline cvector<T> EMPTY_VECTOR{};

public:
	// C++ has no const constructors, so take care to restore const-ness call-side.
	cvectorConstProxy(const cvector<T> *vec) :
		mVectorPtr((vec == nullptr) ? (&EMPTY_VECTOR) : (const_cast<cvector<T> *>(vec))) {
	}

	~cvectorConstProxy() = default;

	// Copy constructor. Share proxying with supplied vec.
	cvectorConstProxy(const cvectorConstProxy &vec) : mVectorPtr(vec.mVectorPtr) {
	}

	// No move constructor, relatively useless here for proxy.
	cvectorConstProxy(cvectorConstProxy &&vec) = delete;

	// No copy or move assignment operators, as you should not be able to assign to this.
	cvectorConstProxy &operator=(const cvectorConstProxy &rhs) = delete;
	cvectorConstProxy &operator=(cvectorConstProxy &&rhs)      = delete;

	// Comparison operators.
	bool operator==(const cvectorConstProxy &rhs) const noexcept {
		return mVectorPtr->operator==(*rhs.mVectorPtr);
	}

	bool operator!=(const cvectorConstProxy &rhs) const noexcept {
		return (!operator==(rhs));
	}

	bool operator<(const cvectorConstProxy &rhs) const noexcept {
		return mVectorPtr->operator<(*rhs.mVectorPtr);
	}

	bool operator>(const cvectorConstProxy &rhs) const noexcept {
		return (rhs.operator<(*this));
	}

	bool operator<=(const cvectorConstProxy &rhs) const noexcept {
		return (!operator>(rhs));
	}

	bool operator>=(const cvectorConstProxy &rhs) const noexcept {
		return (!operator<(rhs));
	}

	bool operator==(const cvector<value_type> &rhs) const noexcept {
		return mVectorPtr->operator==(rhs);
	}

	bool operator!=(const cvector<value_type> &rhs) const noexcept {
		return (!operator==(rhs));
	}

	bool operator<(const cvector<value_type> &rhs) const noexcept {
		return mVectorPtr->operator<(rhs);
	}

	bool operator>(const cvector<value_type> &rhs) const noexcept {
		return (rhs.operator<(*this));
	}

	bool operator<=(const cvector<value_type> &rhs) const noexcept {
		return (!operator>(rhs));
	}

	bool operator>=(const cvector<value_type> &rhs) const noexcept {
		return (!operator<(rhs));
	}

	friend bool operator==(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator==(lhs);
	}

	friend bool operator!=(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator!=(lhs);
	}

	friend bool operator<(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator<(lhs);
	}

	friend bool operator>(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator>(lhs);
	}

	friend bool operator<=(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator<=(lhs);
	}

	friend bool operator>=(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator>=(lhs);
	}

	bool operator==(const std::vector<value_type> &rhs) const noexcept {
		return mVectorPtr->operator==(rhs);
	}

	bool operator!=(const std::vector<value_type> &rhs) const noexcept {
		return (!operator==(rhs));
	}

	bool operator<(const std::vector<value_type> &rhs) const noexcept {
		return mVectorPtr->operator<(rhs);
	}

	bool operator>(const std::vector<value_type> &rhs) const noexcept {
		return (rhs.operator<(*this));
	}

	bool operator<=(const std::vector<value_type> &rhs) const noexcept {
		return (!operator>(rhs));
	}

	bool operator>=(const std::vector<value_type> &rhs) const noexcept {
		return (!operator<(rhs));
	}

	friend bool operator==(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator==(lhs);
	}

	friend bool operator!=(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator!=(lhs);
	}

	friend bool operator<(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator<(lhs);
	}

	friend bool operator>(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator>(lhs);
	}

	friend bool operator<=(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator<=(lhs);
	}

	friend bool operator>=(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) noexcept {
		return rhs.operator>=(lhs);
	}

	[[nodiscard]] const_pointer data() const noexcept {
		return mVectorPtr->data();
	}

	[[nodiscard]] size_type size() const noexcept {
		return mVectorPtr->size();
	}

	[[nodiscard]] size_type capacity() const noexcept {
		return mVectorPtr->capacity();
	}

	[[nodiscard]] size_type max_size() const noexcept {
		return mVectorPtr->max_size();
	}

	[[nodiscard]] bool empty() const noexcept {
		return mVectorPtr->empty();
	}

	template<typename INT>
	[[nodiscard]] const_reference operator[](const INT index) const {
		return at(index);
	}

	template<typename INT>
	[[nodiscard]] const_reference at(const INT index) const {
		return mVectorPtr->at(index);
	}

	[[nodiscard]] explicit operator std::vector<value_type>() const {
		return std::vector<value_type>{cbegin(), cend()};
	}

	[[nodiscard]] const_reference front() const {
		return at(0);
	}

	[[nodiscard]] const_reference back() const {
		return at(-1);
	}

	// Convenience functions.
	template<typename U>
	[[nodiscard]] bool contains(const U &item) const {
		return mVectorPtr->contains(item);
	}

	template<typename Pred>
	[[nodiscard]] bool containsIf(Pred predicate) const {
		return mVectorPtr->containsIf(predicate);
	}

	// Iterator support.
	using iterator               = typename cvector<T>::iterator;
	using const_iterator         = typename cvector<T>::const_iterator;
	using reverse_iterator       = typename cvector<T>::reverse_iterator;
	using const_reverse_iterator = typename cvector<T>::const_reverse_iterator;

	[[nodiscard]] const_iterator begin() const noexcept {
		return cbegin();
	}

	[[nodiscard]] const_iterator end() const noexcept {
		return cend();
	}

	[[nodiscard]] const_iterator cbegin() const noexcept {
		return mVectorPtr->cbegin();
	}

	[[nodiscard]] const_iterator cend() const noexcept {
		return mVectorPtr->cend();
	}

	[[nodiscard]] const_reverse_iterator rbegin() const noexcept {
		return crbegin();
	}

	[[nodiscard]] const_reverse_iterator rend() const noexcept {
		return crend();
	}

	[[nodiscard]] const_reverse_iterator crbegin() const noexcept {
		return mVectorPtr->crbegin();
	}

	[[nodiscard]] const_reverse_iterator crend() const noexcept {
		return mVectorPtr->crend();
	}

	cvector<value_type> operator+(const cvectorConstProxy &rhs) const {
		return mVectorPtr->operator+(*rhs.mVectorPtr);
	}

	cvector<value_type> operator+(const cvector<value_type> &rhs) const {
		return mVectorPtr->operator+(rhs);
	}

	friend cvector<value_type> operator+(const cvector<value_type> &lhs, const cvectorConstProxy &rhs) {
		return rhs.operator+(lhs);
	}

	cvector<value_type> operator+(const std::vector<value_type> &rhs) const {
		return mVectorPtr->operator+(rhs);
	}

	friend cvector<value_type> operator+(const std::vector<value_type> &lhs, const cvectorConstProxy &rhs) {
		return rhs.operator+(lhs);
	}

	cvector<value_type> operator+(const_reference value) const {
		return mVectorPtr->operator+(value);
	}

	friend cvector<value_type> operator+(const_reference value, const cvectorConstProxy &rhs) {
		return rhs.operator+(value);
	}

	cvector<value_type> operator+(std::initializer_list<value_type> rhs_list) const {
		return mVectorPtr->operator+(rhs_list);
	}

	friend cvector<value_type> operator+(std::initializer_list<value_type> lhs_list, const cvectorConstProxy &rhs) {
		return rhs.operator+(lhs_list);
	}
};

template<class T>
class cvectorProxy : public cvectorConstProxy<T> {
public:
	// Container traits.
	using value_type       = typename cvector<T>::value_type;
	using const_value_type = typename cvector<T>::const_value_type;
	using pointer          = typename cvector<T>::pointer;
	using const_pointer    = typename cvector<T>::const_pointer;
	using reference        = typename cvector<T>::reference;
	using const_reference  = typename cvector<T>::const_reference;
	using size_type        = typename cvector<T>::size_type;
	using difference_type  = typename cvector<T>::difference_type;

	static constexpr size_type npos{cvector<T>::npos};

	cvectorProxy(cvector<T> *vec) : cvectorConstProxy<T>(vec) {
		if (vec == nullptr) {
			throw std::invalid_argument("NULL vectors not allowed for modifiable cvector proxy!");
		}
	}

	~cvectorProxy() = default;

	void reassign(cvector<T> *vec) {
		if (vec == nullptr) {
			throw std::invalid_argument("NULL vectors not allowed for modifiable cvector proxy!");
		}

		this->mVectorPtr = vec;
	}

	// Copy constructor.
	cvectorProxy(const cvectorProxy &vec) : cvectorConstProxy<T>(vec.mVectorPtr) {
	}

	// No move constructor or assignment, relatively useless here for proxy.
	cvectorProxy(cvectorProxy &&vec)            = delete;
	cvectorProxy &operator=(cvectorProxy &&rhs) = delete;

	// Copy assignment.
	cvectorProxy &operator=(const cvectorProxy &rhs) {
		return assign(rhs);
	}

	cvectorProxy &operator=(const cvector<value_type> &rhs) {
		return assign(rhs);
	}

	// Extra assignment operators.
	cvectorProxy &operator=(const std::vector<value_type> &rhs) {
		return assign(rhs);
	}

	cvectorProxy &operator=(const_reference value) {
		return assign(value);
	}

	cvectorProxy &operator=(std::initializer_list<value_type> rhs_list) {
		return assign(rhs_list);
	}

	cvectorProxy &assign(const cvectorProxy &vec, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->assign(*vec.mVectorPtr, pos, count);
		return *this;
	}

	cvectorProxy &assign(const cvector<value_type> &vec, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->assign(vec, pos, count);
		return *this;
	}

	cvectorProxy &assign(const std::vector<value_type> &vec, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->assign(vec, pos, count);
		return *this;
	}

	// Lowest common denominator: a ptr and sizes. Most assignments call this.
	cvectorProxy &assign(
		const_pointer vec, const size_type vecLength, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->assign(vec, vecLength, pos, count);
		return *this;
	}

	// Replace vector with one value.
	cvectorProxy &assign(const_reference value) {
		this->mVectorPtr->assign(value);
		return *this;
	}

	// Replace vector with N copies of given value.
	cvectorProxy &assign(const size_type count, const_reference value) {
		this->mVectorPtr->assign(count, value);
		return *this;
	}

	// Replace vector with elements from range.
	template<typename InputIt,
		std::enable_if_t<
			std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, bool>
		= true>
	cvectorProxy &assign(InputIt first, InputIt last) {
		this->mVectorPtr->assign(first, last);
		return *this;
	}

	// Replace vector via initializer list {x, y, z}.
	cvectorProxy &assign(std::initializer_list<value_type> init_list) {
		this->mVectorPtr->assign(init_list);
		return *this;
	}

	[[nodiscard]] pointer data() noexcept {
		return this->mVectorPtr->data();
	}

	void resize(const size_type newSize) {
		return this->mVectorPtr->resize(newSize);
	}

	void resize(const size_type newSize, const_reference value) {
		return this->mVectorPtr->resize(newSize, value);
	}

	void reserve(const size_type minCapacity) {
		return this->mVectorPtr->reserve(minCapacity);
	}

	void shrink_to_fit() {
		return this->mVectorPtr->shrink_to_fit();
	}

	template<typename INT>
	[[nodiscard]] reference operator[](const INT index) {
		return at(index);
	}

	template<typename INT>
	[[nodiscard]] reference at(const INT index) {
		return this->mVectorPtr->at(index);
	}

	[[nodiscard]] reference front() {
		return at(0);
	}

	[[nodiscard]] reference back() {
		return at(-1);
	}

	void push_back(const_reference value) {
		this->mVectorPtr->push_back(value);
	}

	void push_back(value_type &&value) {
		this->mVectorPtr->push_back(std::move(value));
	}

	template<class... Args>
	reference emplace_back(Args &&...args) {
		return this->mVectorPtr->emplace_back(std::forward<Args>(args)...);
	}

	void pop_back() {
		this->mVectorPtr->pop_back();
	}

	void clear() noexcept {
		this->mVectorPtr->clear();
	}

	void swap(cvectorProxy &rhs) noexcept {
		this->mVectorPtr->swap(*rhs.mVectorPtr);
	}

	void swap(cvector<value_type> &rhs) noexcept {
		this->mVectorPtr->swap(rhs);
	}

	// Convenience functions.
	void sortUnique() {
		this->mVectorPtr->sortUnique();
	}

	template<typename Compare>
	void sortUnique(Compare comp) {
		this->mVectorPtr->sortUnique(comp);
	}

	template<typename U>
	size_type remove(const U &item) {
		return this->mVectorPtr->remove(item);
	}

	template<typename Pred>
	size_type removeIf(Pred predicate) {
		return this->mVectorPtr->removeIf(predicate);
	}

	// Iterator support.
	using iterator               = typename cvector<T>::iterator;
	using const_iterator         = typename cvector<T>::const_iterator;
	using reverse_iterator       = typename cvector<T>::reverse_iterator;
	using const_reverse_iterator = typename cvector<T>::const_reverse_iterator;

	[[nodiscard]] iterator begin() noexcept {
		return this->mVectorPtr->begin();
	}

	[[nodiscard]] iterator end() noexcept {
		return this->mVectorPtr->end();
	}

	[[nodiscard]] reverse_iterator rbegin() noexcept {
		return this->mVectorPtr->rbegin();
	}

	[[nodiscard]] reverse_iterator rend() noexcept {
		return this->mVectorPtr->rend();
	}

	iterator insert(const_iterator pos, const_reference value) {
		return this->mVectorPtr->insert(pos, value);
	}

	iterator insert(const_iterator pos, value_type &&value) {
		return this->mVectorPtr->insert(pos, std::move(value));
	}

	iterator insert(const_iterator pos, const size_type count, const_reference value) {
		return this->mVectorPtr->insert(pos, count, value);
	}

	template<typename InputIt,
		std::enable_if_t<
			std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, bool>
		= true>
	iterator insert(const_iterator pos, InputIt first, InputIt last) {
		return this->mVectorPtr->insert(pos, first, last);
	}

	iterator insert(const_iterator pos, std::initializer_list<value_type> init_list) {
		return this->mVectorPtr->insert(pos, init_list);
	}

	template<class... Args>
	iterator emplace(const_iterator pos, Args &&...args) {
		return this->mVectorPtr->emplace(pos, std::forward<Args>(args)...);
	}

	iterator erase(const_iterator pos) {
		return this->mVectorPtr->erase(pos);
	}

	iterator erase(const_iterator first, const_iterator last) {
		return this->mVectorPtr->erase(first, last);
	}

	cvectorProxy &append(const cvectorProxy &vec, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->append(*vec.mVectorPtr, pos, count);
		return *this;
	}

	cvectorProxy &append(const cvector<value_type> &vec, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->append(vec, pos, count);
		return *this;
	}

	cvectorProxy &append(const std::vector<value_type> &vec, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->append(vec, pos, count);
		return *this;
	}

	// Lowest common denominator: a ptr and sizes.
	cvectorProxy &append(
		const_pointer vec, const size_type vecLength, const size_type pos = 0, const size_type count = npos) {
		this->mVectorPtr->append(vec, vecLength, pos, count);
		return *this;
	}

	// Enlarge vector with one value.
	cvectorProxy &append(const_reference value) {
		this->mVectorPtr->append(value);
		return *this;
	}

	// Enlarge vector with N copies of given value.
	cvectorProxy &append(const size_type count, const_reference value) {
		this->mVectorPtr->append(count, value);
		return *this;
	}

	// Enlarge vector with elements from range.
	template<typename InputIt,
		std::enable_if_t<
			std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<InputIt>::iterator_category>, bool>
		= true>
	cvectorProxy &append(InputIt first, InputIt last) {
		this->mVectorPtr->append(first, last);
		return *this;
	}

	// Enlarge vector via initializer list {x, y, z}.
	cvectorProxy &append(std::initializer_list<value_type> init_list) {
		this->mVectorPtr->append(init_list);
		return *this;
	}

	cvectorProxy &operator+=(const cvectorProxy &rhs) {
		return append(rhs);
	}

	cvectorProxy &operator+=(const cvector<value_type> &rhs) {
		return append(rhs);
	}

	cvectorProxy &operator+=(const std::vector<value_type> &rhs) {
		return append(rhs);
	}

	cvectorProxy &operator+=(const_reference value) {
		return append(value);
	}

	cvectorProxy &operator+=(std::initializer_list<value_type> rhs_list) {
		return append(rhs_list);
	}
};

} // namespace dv
