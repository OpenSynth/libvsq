/**
 * SMFReader.hpp
 * Copyright © 2012,2014 kbinani
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
#pragma once

#include "./Namespace.hpp"
#include "./MidiEvent.hpp"
#include <exception>
#include <vector>

VSQ_BEGIN_NAMESPACE

class InputStream;

/**
 * @brief SMF(Standard MIDI File)の読み込みを行うクラス.
 */
class SMFReader
{
public:
	class ParseException : public std::exception
	{};

	/**
	 * @brief ストリームから, SMF を読み込む.
	 * @param[in] stream 読み込むストリーム.
	 * @param[out] dest 読み込んだ MIDI イベントのリスト. MIDI イベントは, トラックごとに格納される.
	 * @param[out] format SMF のフォーマット.
	 * @param[out] timeFormat 時間分解能.
	 */
	void read(InputStream* stream, std::vector<std::vector<MidiEvent> >& dest, int& format, int& timeFormat);
};

VSQ_END_NAMESPACE
