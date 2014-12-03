/**
 * TempoList.cpp
 * Copyright © 2014 kbinani
 *
 * This file is part of libvsq.
 *
 * libvsq is free software; you can redistribute it and/or
 * modify it under the terms of the BSD License.
 *
 * libvsq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "./TempoList.hpp"

VSQ_BEGIN_NAMESPACE

TempoList::Iterator::Iterator(const std::vector<Tempo>* list)
{
	this->list = list;
	this->iterator = list->begin();
}

bool TempoList::Iterator::hasNext()
{
	return iterator != this->list->end();
}

Tempo TempoList::Iterator::next()
{
	Tempo result = *iterator;
	++iterator;
	return result;
}

TempoList::Iterator TempoList::iterator() const
{
	return Iterator(&_array);
}

void TempoList::sort()
{
	std::stable_sort(_array.begin(), _array.end(), Tempo::compare);
}

void TempoList::push(const Tempo& value)
{
	_array.push_back(value);
}

int TempoList::size() const
{
	return _array.size();
}

Tempo TempoList::get(int index) const
{
	return _array[index];
}

void TempoList::set(int index, const Tempo& value)
{
	_array[index] = value;
}

double TempoList::getClockFromSec(double time) const
{
	// timeにおけるテンポを取得
	int tempo = TempoList::baseTempo;
	tick_t base_clock = 0;
	double base_time = 0.0;
	int c = _array.size();
	if (c == 0) {
		tempo = TempoList::baseTempo;
		base_clock = 0;
		base_time = 0.0;
	} else if (c == 1) {
		tempo = _array[0].tempo;
		base_clock = _array[0].clock;
		base_time = _array[0]._time;
	} else {
		for (int i = c - 1; i >= 0; i--) {
			Tempo item = _array[i];
			if (item._time < time) {
				return item.clock + (time - item._time) * TempoList::gatetimePerQuater * 1000000.0 / item.tempo;
			}
		}
	}
	double dt = time - base_time;
	return base_clock + dt * TempoList::gatetimePerQuater * 1000000.0 / tempo;
}

void TempoList::updateTempoInfo()
{
	int c = _array.size();
	if (c == 0) {
		_array.push_back(Tempo(0, TempoList::baseTempo));
	}
	std::stable_sort(_array.begin(), _array.end(), Tempo::compare);
	Tempo item0 = _array[0];
	if (item0.clock != 0) {
		item0._time = TempoList::baseTempo * item0.clock / (TempoList::gatetimePerQuater * 1000000.0);
	} else {
		item0._time = 0.0;
	}
	double prev_time = item0._time;
	tick_t prev_clock = item0.clock;
	int prev_tempo = item0.tempo;
	double inv_tpq_sec = 1.0 / (TempoList::gatetimePerQuater * 1000000.0);
	for (int i = 1; i < c; i++) {
		_array[i]._time = prev_time + prev_tempo * (_array[i].clock - prev_clock) * inv_tpq_sec;

		Tempo itemi = _array[i];
		prev_time = itemi._time;
		prev_tempo = itemi.tempo;
		prev_clock = itemi.clock;
	}
}

double TempoList::getSecFromClock(double clock) const
{
	int c = _array.size();
	for (int i = c - 1; i >= 0; i--) {
		Tempo item = _array[i];
		if (item.clock < clock) {
			double init = item.getTime();
			tick_t dclock = clock - item.clock;
			double sec_per_clock1 = item.tempo * 1e-6 / 480.0;
			return init + dclock * sec_per_clock1;
		}
	}

	double sec_per_clock = TempoList::baseTempo * 1e-6 / 480.0;
	return clock * sec_per_clock;
}

int TempoList::getTempoAt(int clock) const
{
	int index = 0;
	int c = size();
	for (int i = c - 1; i >= 0; i--) {
		index = i;
		if (_array[i].clock <= clock) {
			break;
		}
	}
	return _array[index].tempo;
}

void TempoList::clear()
{
	_array.clear();
}

VSQ_END_NAMESPACE
