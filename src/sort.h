/*
 * algorithm.h
 *
 *  Created on: Oct 7, 2014
 *      Author: nbingham
 */

#pragma once

#include "slice.h"
#include "array.h"

namespace core
{

template <class type>
void swap(type &t1, type &t2)
{
	type temp = t1;
	t1 = t2;
	t2 = temp;
}

template <class container>
int sort_order_fast(const container &c)
{
	int result = 0;
	for (typename container::const_iterator i = c.begin(); i && i != c.rbegin(); i++) {
		if (*(i+1) < *i) {
			if (result == 0)
				result = -1;
			else if (result == 1)
				return 0;
		} else if (*(i+1) > *i) {
			if (result == 0)
				result = 1;
			else if (result == -1)
				return 0;
		}
	}

	return result;
}

template <class container>
float sort_order(const container &c)
{
	int f = 0;
	for (typename container::const_iterator i = c.begin(); i && i != c.rbegin(); i++)
		f += (*i < *(i+1)) ? 1 : -1;

	if (c.size() > 0)
		return (float)f/(float)c.size();
	else
		return 0.0;
}

template <class container>
bool is_sorted(const container &c)
{
	for (typename container::const_iterator i = c.begin(); i && i != c.rbegin(); i++)
		if (*(i+1) < *i)
			return false;

	return true;
}

template <class container>
bool is_rsorted(const container &c)
{
	for (typename container::const_iterator i = c.begin(); i && i != c.rbegin(); i++)
		if (*(i+1) > *i)
			return false;

	return true;
}

template <class container>
container reverse(container c)
{
	for (typename container::iterator i = c.begin(), j = c.rbegin(); i != j && i != j+1; i++, j--)
		i.swap(j);
	return c;
}

template <class container>
container &reverse_inplace(container &c)
{
	for (typename container::iterator i = c.begin(), j = c.rbegin(); i != j && i != j+1; i++, j--)
		i.swap(j);
	return c;
}

template <class type>
type median_iterator(type t1, type t2, type t3)
{
	if (*t1 < *t2)
	{
		if (*t2 < *t3)
			return t2;
		else if (*t3 < *t1)
			return t1;
		else
			return t3;
	}
	else
	{
		if (*t1 < *t3)
			return t1;
		else if (*t3 < *t2)
			return t2;
		else
			return t3;
	}
}

// Sorting Algorithms
template <class container>
container &sort_insert_inplace(container &c)
{
	for (typename container::iterator i = c.begin()+1; i; i++)
		for (typename container::iterator j = i; j && j != c.begin(); j--)
			if (*j < *(j-1))
				j.swap(j-1);

	return c;
}

template <class container>
container sort_insert(container c)
{
	return sort_insert_inplace(c);
}

template <class container>
container &sort_selection_inplace(container &c)
{
	for (typename container::iterator i = c.begin(); i != c.end(); i++)
	{
		typename container::iterator max_j = i;
		for (typename container::iterator j = i+1; j != c.end(); j++)
			if (*j < *max_j)
				max_j = j;

		i.swap(max_j);
	}

	return c;
}

template <class container>
container sort_selection(container c)
{
	return sort_selection_inplace(c);
}

template <class container>
container &sort_quick_inplace(container &c)
{
	static array<slice<range<typename container::iterator> > > stack;
	stack.push_back(c.sub());

	while (stack.size() > 0)
	{
		slice<range<typename container::iterator> > elem = stack.back();
		stack.drop_back();

		int sz = elem.size();
		if (sz < 10)
			sort_insert_inplace(elem);
		else
		{
			int order = sort_order_fast(elem);
			if (order == -1)
				reverse_inplace(elem);
			else if (order == 0)
			{
				typename container::iterator store = elem.begin();
				while (store && *store < *elem.rbegin())
					store++;
				
				for (typename container::iterator i = store+1; i && i != elem.rbegin(); i++)
					if (*i < *elem.rbegin())
					{
						i.swap(store);
						store++;
					}

				store.swap(elem.rbegin());
				slice<range<typename container::iterator> > left, right;
				left = container::sub(elem.begin(), store);
				right = container::sub(store+1, elem.end());
				if (left.size() < right.size())
				{
					elem = right;
					right = left;
					left = elem;
				}

				if (left.size() > 1)
					stack.push_back(left);
				if (right.size() > 1)
					stack.push_back(right);
			}
		}
	}

	return c;
}

template <class container>
container sort_quick(container c)
{
	return sort_quick_inplace(c);
}

template <class container1, class container2>
container1 sort_merge(const container1 &c1, const container2 &c2)
{
	container1 result;
	typename container1::const_iterator i = c1.begin();
	typename container2::const_iterator j = c2.begin();
	while (i != c1.end() && j != c2.end())
	{
		if (*i < *j)
		{
			result.push_back(*i);
			i++;
		}
		else if (*j < *i)
		{
			result.push_back(*j);
			j++;
		}
		else
		{
			result.push_back(*i);
			result.push_back(*j);
			i++;
			j++;
		}
	}

	result.append_back(i.sub());
	result.append_back(j.sub());
	return result;
}

template <class container1, class container2>
container1 &sort_merge_inplace(container1 &c1, const container2 &c2)
{
	typename container1::iterator i = c1.begin();
	typename container2::const_iterator j = c2.begin();
	while (i != c1.end() && j != c2.end())
	{
		if (*i < *j)
			i++;
		else if (*j < *i)
		{
			i.push(*j);
			j++;
		}
		else
		{
			i++;
			i.push(*j);
			j++;
		}
	}

	c1.append_back(j.sub());
	return c1;
}

template <class container>
container &sort_inplace(container &c)
{
	float o = sort_order(c);
	if (o < 0.0) {
		reverse_inplace(c);
		o = -o;
	}

	if (o > 0.8)
		sort_insert_inplace(c);
	else
		sort_quick_inplace(c);

	return c;
}

template <class container>
container sort(container c)
{
	return sort_inplace(c);
}

template <class container, class element>
typename container::iterator sorted_insert(container &c1, const element &c2)
{
	typename container::iterator result = lower_bound(c1, c2);
	result.push(c2);
	return result-1;
}

}

